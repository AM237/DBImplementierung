
///////////////////////////////////////////////////////////////////////////////
// TwoQueueReplacer.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BufferFrame.h"
#include "BufferHasher.h"
#include "FrameReplacer.h"
#include "BufferManager.h"

#include <sys/mman.h>

using namespace std;
using namespace constants;

//_____________________________________________________________________________
void TwoQueueReplacer::pageFixedFirstTime(BufferFrame* frame)
{
	fifo.push_front(frame);
}


//_____________________________________________________________________________
void TwoQueueReplacer::pageFixedAgain(BufferFrame* frame)
{
	// frame is in FIFO queue, move to LRU queue
	for(std::list<BufferFrame*>::iterator it=fifo.begin(); it!=fifo.end(); it++)
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
	for(std::list<BufferFrame*>::iterator it=lru.begin(); it!=lru.end(); it++)
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
BufferFrame* TwoQueueReplacer::replaceFrame()
{	
	for(std::list<BufferFrame*>::iterator it=fifo.end(); it != fifo.begin(); )
	{   
		it--; 
		BufferFrame* bf = *it;
		
		if(bf == NULL) continue;
		
		if (!bf->pageFixed)
		{
            fifo.erase(it);
            
            // free data in frame, update frame lookup mechanism
            // note: since page in frame is unfixed, it is also clean, since
            // dirty pages are written back to disk when they are unfixed.
            if (munmap(bf->data, constants::pageSize) < 0)
            {
            	cout << "Failed unmapping main memory: " << errno << endl;
				exit(1);            
            }
            bf->data = NULL;
            
            FrameReplacer::hasher->remove(bf->pageId);
			return bf;
		}
	}

	for(std::list<BufferFrame*>::iterator it=lru.end(); it != lru.begin(); )
	{   
		it--;
		BufferFrame* bf = *it;

		if(bf == NULL) continue;
			
		if (!bf->pageFixed)
		{
			lru.erase(it);
            
            // reset frame
            if (munmap(bf->data, constants::pageSize) < 0)
            {
            	cout << "Failed unmapping main memory: " << errno << endl;
				exit(1);            
            }
            bf->data = NULL;
            FrameReplacer::hasher->remove(bf->pageId);
			return bf;
		}
	}

    return NULL;
}



