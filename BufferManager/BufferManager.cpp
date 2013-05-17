
///////////////////////////////////////////////////////////////////////////////
// BufferManager.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BufferManager.h"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <queue>
#include <sys/mman.h>

using namespace std;




//_____________________________________________________________________________
void TwoQueues::pageFixedFirstTime(BufferFrame* frame)
{

  fifo.push_front(frame);
 
}


//_____________________________________________________________________________
void TwoQueues::pageFixedAgain(BufferFrame* frame)
{
	std::list<BufferFrame*>::iterator it;

	// frame is in FIFO queue, move to LRU queue
	for(it=fifo.begin(); it != fifo.end(); ++it)
	{    
		BufferFrame* bf = *it;
		if (bf == frame)
		{
			fifo.erase(it);
			lru.push_front(frame);
			return;
		}
	}

    // frame is in LRU queue, move to front of LRU queue
	for(it=lru.begin(); it != lru.end(); ++it)
	{    
		BufferFrame* bf = *it;
		if (bf == frame)
		{
			lru.erase(it);
			lru.push_front(frame);
			return;
		}
	}
}


//_____________________________________________________________________________
BufferFrame* TwoQueues::replaceFrame()
{

	std::list<BufferFrame*>::iterator it;
	for(it=fifo.end(); it != fifo.begin(); --it)
	{    
		BufferFrame* bf = *it;
		if (bf->pageFixed == false)
		{
                        fifo.erase(it);
                        // TODO free data in frame
			return bf;
		}
	}

	for(it=lru.end(); it != lru.begin(); --it)
	{    
		BufferFrame* bf = *it;
		if (bf->pageFixed == false)
		{
			lru.erase(it);
                        // TODO free data in frame
			return bf;
		}
	}

    return NULL;
}





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
	
	// Initialize TwoQueues
	twoQueues = new TwoQueues();

	// Initialize buffer frame pool
	for (unsigned int i = 0; i < size; i++)
		framePool.push_back(new BufferFrame());
}


//_____________________________________________________________________________
void BufferManager::readPageInFrame(uint64_t pageId, BufferFrame* frame )
{

	// Read page from file into main memory. 
	// Page begins at pageId * pageSize bytes
	// TODO: pointer goes out of scope?		
	char* memLoc = static_cast<char*>(mmap(NULL, constants::pageSize, 
					PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 
					pageId * constants::pageSize));				
			
	if (memLoc == MAP_FAILED)
	{
		cout << "Failed to read page into main memory" << endl;
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
		cout << "Error writing page back to disk (unfix)" << endl;
		exit(1);
	}
}


//_____________________________________________________________________________
BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive)
{
	// Case: page with pageId is buffered -> return page directly
	vector<BufferFrame*>* frames = hasher->lookup(pageId);
	for (size_t i = 0; i < frames->size(); i++)
	{
		BufferFrame* bf = frames->at(i);
		if (bf->pageId == pageId)
        {
            twoQueues->pageFixedAgain(bf);
			return *bf;
        }
	}
	
	// Case: page with pageId not buffered and space available in buffer
	// -> read from file into a free buffer frame, and add an association
	// between the pageId just read and the BufferFrame the data was read into
	// in the hash table
	bool spaceFound = false;
	for (size_t i = 0; i < framePool.size(); i++)
	{
		BufferFrame* frame = framePool.at(i);
		if (frame->getData() == NULL)
		{
            twoQueues->pageFixedFirstTime(frame);
			spaceFound = true;
			readPageInFrame( pageId, frame );
			break;		
		}
	}
	

	// Case: page with pageId not buffered and buffer full
	// -> use replacement strategy to replace an unfixed page in buffer
	// and update the frame pool proxy (hash table) accordingly.
	// If no pages can be replaced, method is allowed to fail (via exception,
	// block, etc.)
    if(spaceFound==false)
    {    
       	BufferFrame* frame = twoQueues->replaceFrame();
       	if (frame->getData() == NULL)//TODO no frame available
  		{
       		twoQueues->pageFixedFirstTime(frame);
			readPageInFrame( pageId, frame );
		}
	}	
}

//_____________________________________________________________________________
void BufferManager::unfixPage(BufferFrame& frame, bool isDirty)
{
	// Set the page as a candidate for replacement, consider
	// whether to implement force or no force, and whether page is dirty or not
	frame.isDirty = isDirty;
	frame.pageFixed = false;
	
	
	// Write page back to disk if dirty, update dirty bit
	if (isDirty)
	{
		flushFrameToFile(frame);
	
		// Data on disk now corresponds to data in buffer, so frame is
		// no longer dirty	
		frame.isDirty = false;
		
	} else {
	
		// ?
	}
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
		delete[] (char*)frame->data;
		delete frame;
	}
	
	// Close file with pages
	close(fileDescriptor);
	
	delete hasher;
	delete twoQueues;	
}
