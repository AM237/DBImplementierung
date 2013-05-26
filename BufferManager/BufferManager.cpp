///////////////////////////////////////////////////////////////////////////////
// BufferManager.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BufferManager.h"
#include <fcntl.h>
#include <sys/mman.h>

using namespace std;


//_____________________________________________________________________________
BufferManager::BufferManager(const string& filename, uint64_t size)
{
	// Initialize class fields
	numFrames = size;

	// Open file with pages
  	fileDescriptor = open(filename.c_str(), O_RDWR);
  	if (fileDescriptor < 0)
  	{
  		cout << "Error opening file on disk" << endl;
  		exit(1);
  	}
  	
  	// Check that file has a multiple of constants::pageSize bytes
  	// TODO
  	
	// Initialize hasher. In terms of the size of the underlying hash table,
	// the worst case is given when each page is mapped to its own unique
	// bucket, so the max. number of required buckets is that of the 
	// manager's frame capacity
	hasher = new BufferHasher(numFrames);

	// Initialize frame replacer
	replacer = new TwoQueueReplacer(hasher);

	// Initialize buffer frame pool
	for (unsigned int i = 0; i < size; i++)
		framePool.push_back(new BufferFrame());
		
	lock = PTHREAD_RWLOCK_INITIALIZER;
}


//_____________________________________________________________________________
void BufferManager::readPageIntoFrame(uint64_t pageId, BufferFrame* frame )
{

	// Read page from file into main memory. 
	// Page begins at pageId * pageSize bytes	
	char* memLoc = static_cast<char*>(mmap(NULL, constants::pageSize, 
					PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 
					pageId * constants::pageSize));				

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


//_____________________________________________________________________________
void BufferManager::flushFrameToFile(BufferFrame& frame)
{
	uint64_t pageId = frame.pageId;

	// seek to correct position in file
	if (lseek(fileDescriptor, pageId*constants::pageSize, SEEK_SET) < 0)
	{
		cout << "Error seeking for page on disk" << endl;
		exit(1);
	}

	// write page
	if (write(fileDescriptor, frame.getData(), constants::pageSize) < 0)
	{
		cout << "Error writing page back to disk:" << errno << endl;
		exit(1);
	}
}


//_____________________________________________________________________________
BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive)
{

 	if(exclusive) pthread_rwlock_wrlock(&lock);
 	else pthread_rwlock_wrlock(&lock);
 	
	// Case: page with pageId is buffered -> return page directly
	vector<BufferFrame*>* frames = hasher->lookup(pageId);
	for (size_t i = 0; i < frames->size(); i++)
	{
		BufferFrame* bf = frames->at(i);
		if (bf->pageId == pageId)
        {
        	//bf->lock.lock();
        	
            replacer->pageFixedAgain(bf);

			//ScopedLock scoped(lock);
			return *bf;
        }
	}

	// Case: page with pageId not buffered and space available in buffer
	// -> read from file into a free buffer frame, and add an association
	// between the pageId just read and the BufferFrame the data was read into
	// in the hash table
	bool spaceFound = false;
	bool allPagesFixed = true;
	for (size_t i = 0; i < framePool.size(); i++)
	{
		BufferFrame* frame = framePool.at(i);
		if (frame->getData() == NULL)
		{
			//frame->lock.lock();
			
            replacer->pageFixedFirstTime(frame);
			readPageIntoFrame( pageId, frame );
			spaceFound = true;
		
			//ScopedLock scoped(lock);
			return *frame;
		}
		if (!frame->pageFixed) allPagesFixed = false;
	}


	// Case: page with pageId not buffered and buffer full
	// -> use replacement strategy to replace an unfixed page in buffer
	// and update the frame lookup mechanism (hash table) accordingly.
	// If no pages can be replaced, method is allowed to fail (via exception,
	// block, etc.)
    if(!spaceFound)
    {       	
    	// no pages can be replaced
    	if (allPagesFixed)
    	{
    		ReplaceFailAllFramesFixed fail;
       	 	throw fail;
    	}
    	    	
       	BufferFrame* frame = replacer->replaceFrame();
       	
       	// should never be the case?
       	if (frame == NULL)
       	{
			ReplaceFailNoFrameSuggested fail;
			throw fail;
       	}
       	
       	// should always be the case
       	if (frame->getData() == NULL)
  		{
  			//frame->lock.lock();
  			
       		replacer->pageFixedFirstTime(frame);
			readPageIntoFrame( pageId, frame );
		
			//ScopedLock scoped(lock);
			return *frame;

		} else {

			ReplaceFailFrameUnclean fail;
			throw fail;
		}
	}

	// Is never returned, because exactly one case above is true
	BufferFrame* bf = new BufferFrame();
	return *bf;
}

//_____________________________________________________________________________
void BufferManager::unfixPage(BufferFrame& frame, bool isDirty)
{
	//unfixlock.lock();
	
	// Note: frame is a reference to an existing buffer frame in the pool.
	// Therefore, it suffices to directly set the page as candidate for
	// replacement.
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


	//unfixlock.unlock();
	//frame.lock.unlock();
	//lock.unlock();
	pthread_rwlock_unlock(&lock);
}

//_____________________________________________________________________________
BufferManager::~BufferManager()
{	
	// Write all dirty frames to disk + clean main memory
	// Note: order in which deletes occur is important	
	for (size_t i = 0; i < framePool.size(); i++)
	{
		BufferFrame* frame = framePool[i];
		if (frame->getData() != NULL && frame->isDirty)
			flushFrameToFile(*frame);

		// delete data in frame
		if (frame != NULL)
		{
			if (munmap(frame->data, constants::pageSize) < 0)
            {
            	cout << "Destructor: failed unmapping main memory: " 
            	     << errno << endl;
				exit(1);            
            }
			delete frame;
		}
	}

	// Close file with pages
	close(fileDescriptor);

	delete hasher;
	delete replacer;	
}
