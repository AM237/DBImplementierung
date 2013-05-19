
///////////////////////////////////////////////////////////////////////////////
// TwoQueueReplacer.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BufferFrame.h"
#include "BufferHasher.h"
#include "FrameReplacer.h"

using namespace std;

//_____________________________________________________________________________
void TwoQueueReplacer::pageFixedFirstTime(BufferFrame* frame)
{
	fifo.push_front(frame);
}


//_____________________________________________________________________________
void TwoQueueReplacer::pageFixedAgain(BufferFrame* frame)
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
BufferFrame* TwoQueueReplacer::replaceFrame()
{	
	std::list<BufferFrame*>::iterator it;
	for(it=fifo.end(); it != fifo.begin(); --it)
	{    
		BufferFrame* bf = *it;
		if (!bf->pageFixed)
		{
            fifo.erase(it);
            
            // free data in frame, update frame lookup mechanism
            // note: since page in frame is unfixed, it is also clean, since
            // dirty pages are written back to disk when they are unfixed.
            //delete[] (char*)bf->data;
            bf->data = NULL;
            FrameReplacer::hasher->remove(bf->pageId);
			return bf;
		}
	}

	for(it=lru.end(); it != lru.begin(); --it)
	{    
		BufferFrame* bf = *it;
		if (!bf->pageFixed)
		{
			lru.erase(it);
            
            // reset frame
            //delete[] (char*)bf->data;
            bf->data = NULL;
            FrameReplacer::hasher->remove(bf->pageId);
			return bf;
		}
	}

    return NULL;
}



