///////////////////////////////////////////////////////////////////////////////
// BufferManager.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BufferManager.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <thread>

using namespace std;

//______________________________________________________________________________
BufferManager::BufferManager(const string& filename, uint64_t size, 
							 const int pages)
{
	// Initialize class fields
	numFrames = size;
	numPages = pages;

	// Open file with pages
	fileDescriptor = initializeDatabase(filename.c_str());
 	
	// Initialize hasher. In terms of the size of the underlying hash table,
	// the worst case is given when each page is mapped to its own unique
	// bucket, so the max. number of required buckets is that of the 
	// manager's frame capacity
	hasher = new BufferHasher(numFrames);

	// Initialize frame replacer
	replacer = new TwoQueueReplacer(hasher);
}


// _____________________________________________________________________________
int BufferManager::initializeDatabase(const char* filename)
{	
	FILE* dbFile = fopen(filename, "r");
 	
 	// If file not existent, create standard file with n pages
 	if (dbFile == nullptr)
  	{
  		if (errno == ENOENT)
  		{
  			dbFile = fopen (filename,"w+b");
  			if (dbFile == nullptr)
  			{
  				cout << "Error creating database file (opening)" << endl;
  				exit(1);
  			}
  		
  			vector<uint64_t> pages;
  			pages.resize(numPages*BM_CONS::pageSize/sizeof(uint64_t), 0);

			if (write(fileno(dbFile), pages.data(), 
			    numPages*BM_CONS::pageSize) < 0)
			{
				cout <<"Error creating database file (writing): "<<errno <<endl;
				exit(1);
			}

		} else {

  			cout << "Error opening the database file" << endl;
  			exit(1);
		}

	// else file exists -> check file consistency: should be at least n pages 
	// worth of bytes and total number of bytes should be a multiple of the 
	// page size.
  	} else {
  	
  		fclose(dbFile);
  		dbFile = fopen(filename, "rb");

		if (lseek(fileno(dbFile), 0, SEEK_END) < 0)
		{
			cout << "Error seeking to end of file: " << errno << endl;
			exit(1);
		}

		uint64_t fileBytes = ftell(dbFile);

		if (fileBytes % BM_CONS::pageSize != 0 ||
		    fileBytes < numPages * BM_CONS::pageSize)
		{			
			cout << "Database file is not formatted correctly" << endl;
			exit(1);

		} else {

			if (lseek(fileno(dbFile), 0, SEEK_SET) < 0)
		    {
				cout << "Error seeking to start of file: " << errno << endl;
				exit(1);
			}
		}
  	}
  	
  	// finally, get file descriptor from newly created or verified database
  	// Note: memory mapping requires that file be opened with O_RDWR flag.
  	fclose(dbFile);
  	int fd = open(filename, O_RDWR);
  	if (fd < 0)
  	{
  		cout << "Error opening file on disk: " << errno << endl;
  		exit(1);
  	}
  	
  	return fd;
}


//______________________________________________________________________________
std::pair<uint64_t, uint64_t> BufferManager::growDB(uint64_t pages)
{
	uint64_t sizeBefore = numPages;

	// seek to end of file
	if (lseek(fileDescriptor, 0, SEEK_END) < 0)
	{
		cout << "Error seeking to end of file: " << errno << endl;
		exit(1);
	}

	// write page
	vector<uint64_t> pageData;
	uint64_t totalBytes = pages*BM_CONS::pageSize;
	pageData.resize(totalBytes/sizeof(uint64_t), 0);
	if (write(fileDescriptor, pageData.data(), totalBytes) < 0)
	{
		cout << "Error appending pages to file: " << errno << endl;
		exit(1);
	}
	numPages += pages;

	uint64_t sizeAfter = numPages;
	return pair<uint64_t, uint64_t>(sizeBefore, sizeAfter);
}


//______________________________________________________________________________
void BufferManager::readPageIntoFrame(uint64_t pageId, BufferFrame* frame)
{
	// Read page from file into main memory. 
	// Page begins at pageId * pageSize bytes	
	char* memLoc = static_cast<char*>(mmap(nullptr, BM_CONS::pageSize, 
					PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 
					pageId * BM_CONS::pageSize));				

	if (memLoc == MAP_FAILED)
	{
		cout << "Failed to read page into main memory: " << errno << endl;
		exit(1);
	}

	// Update frame info
	frame->data = memLoc;
	frame->isDirty = false;
	frame->pageId = pageId;
	frame->pageFixed = true;

	// Update frame pool proxy
	hasher->insert(pageId, frame);
}


//______________________________________________________________________________
void BufferManager::flushFrameToFile(BufferFrame& frame)
{
	uint64_t pageId = frame.pageId;

	// seek to correct position in file
	if (lseek(fileDescriptor, pageId*BM_CONS::pageSize, SEEK_SET) < 0)
	{
		cout << "Error seeking for page on disk" << endl;
		exit(1);
	}

	// write page
	if (write(fileDescriptor, frame.getData(), BM_CONS::pageSize) < 0)
	{
		cout << "Error writing page back to disk:" << errno << endl;
		exit(1);
	}
}


//______________________________________________________________________________
BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive)
{
	// Label implements tail recursion to avoid stack overflow 	
	top:

	// Case: page with pageId is buffered -> return page directly
	bmlock.lock();
	vector<BufferFrame*>* frames = hasher->lookup(pageId);
	for (size_t i = 0; i < frames->size(); i++)
	{
		BufferFrame* bf = frames->at(i);
		if (bf->isClient()) continue;
		if(!bf->tryLockFrame(exclusive))
        {
           	bmlock.unlock();
            goto top;
        }
        
		if (bf->pageId == pageId)
        {
            replacer->pageFixedAgain(bf);
            bmlock.unlock();
			return *bf;
        }
        bf->unlockFrame();
	}
	bmlock.unlock();


	// Case: page with pageId not buffered and space available in buffer
	// -> read from file into a free buffer frame, and add an association
	// between the pageId just read and the BufferFrame the data was read into
	// in the hash table
	bmlock.lock();
	bool spaceFound = false;
	bool allPagesFixed = true;
	for (uint64_t i = 0; i < numFrames; i++)
	{
		BufferFrame* frame = hasher->nextFrame();
		if (frame->isClient()) continue;
		if(!frame->tryLockFrame(true))
		{
			bmlock.unlock();
            goto top;
        }
       
		if (frame->getData() == nullptr)
		{
        	spaceFound = true;
            readPageIntoFrame(pageId, frame);
            
            frame->unlockFrame();
            frame->lockFrame(exclusive);

            replacer->pageFixedFirstTime(frame);     
            bmlock.unlock();
			return *frame;
		}
		frame->unlockFrame();
		if (!frame->pageFixed) allPagesFixed = false;
	}

	// Case: page with pageId not buffered and buffer full
	// -> use replacement strategy to replace an unfixed page in buffer
	// and update the frame lookup mechanism (hash table) accordingly.
	// If no pages can be replaced, method is allowed to fail (via exception,
	// block, etc
    if(!spaceFound)
    {       	
    	// no pages can be replaced
    	if (allPagesFixed) 
    	{ 
    		bmlock.unlock();
    		BM_EXC::ReplaceFailAllFramesFixed e; throw e; 
    	}

       	BufferFrame* frame = replacer->replaceFrame();
       	if(!frame->tryLockFrame(true))
        {
           	bmlock.unlock();
            goto top;
        }

       	// should never be the case
       	if (frame == nullptr) 
       	{
       		bmlock.unlock(); 
       		BM_EXC::ReplaceFailNoFrameSuggested e; throw e;
       	}
       	
       	// should always be the case
       	if (frame->getData() == nullptr)
  		{ 
			readPageIntoFrame(pageId, frame);
  			            
            frame->unlockFrame();
            frame->lockFrame(exclusive);

            replacer->pageFixedFirstTime(frame);
  			bmlock.unlock();
			return *frame;
		} 

		else 
		{
			bmlock.unlock(); 
			BM_EXC::ReplaceFailFrameUnclean  e; throw e; 
		}
	}

	// Is never returned, because exactly one case above is true
	BM_EXC::IllegalPathException e; throw e;
	return *(new BufferFrame());
}

//______________________________________________________________________________
void BufferManager::unfixPage(BufferFrame& frame, bool isDirty)
{	
	// Note: frame is a reference to an existing buffer frame in the pool.
	// Therefore, it suffices to directly set the page as candidate for
	// replacement.
	bmlock.lock();
	frame.isDirty = isDirty;
	frame.pageFixed = false;

	// Write page back to disk if dirty, update dirty bit
	if (isDirty)
	{	
		flushFrameToFile(frame);

		// Data on disk now corresponds to data in buffer, so frame is
		// no longer dirty	
		frame.isDirty = false;
	}

	frame.unlockFrame();
	bmlock.unlock();
}

//______________________________________________________________________________
BufferManager::~BufferManager()
{	
	// Write all dirty frames to disk + clean main memory
	// Note: order in which deletes occur is important	

	// Close file with pages
	close(fileDescriptor);

	delete hasher;
	delete replacer;	
}