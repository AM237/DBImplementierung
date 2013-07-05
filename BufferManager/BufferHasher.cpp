
///////////////////////////////////////////////////////////////////////////////
// BufferHasher.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BufferHasher.h"

using namespace std;

//______________________________________________________________________________
BufferHasher::BufferHasher(uint64_t tableSize)
{ 
	if (tableSize == 0)
	{
		cout << "Cannot create Hasher with 0 frames" << endl;
		exit(1);
	}
	size = tableSize;
	currentFrameIndex = 0;
	locks = new std::vector<std::mutex>(size);
	for(unsigned int i = 0; i < size; i++)
	{
		framePool.push_back(new BufferFrame());
		std::vector<BufferFrame*> initial;
		hashTable.push_back(initial);
	}
}

// _____________________________________________________________________________
BufferHasher::~BufferHasher()
{ 
	delete locks;
	for (uint64_t i = 0; i < size; i++) delete nextFrame();	 
}


// _____________________________________________________________________________
uint64_t BufferHasher::hash(uint64_t pageId)
{ 
	return pageId % size; 
}


// _____________________________________________________________________________
void BufferHasher::insert(uint64_t pageId, BufferFrame* bf)
{
	lockBucket(pageId);
	hashTable[hash(pageId)].push_back(bf);
	unlockBucket(pageId);
}

// _____________________________________________________________________________
std::vector<BufferFrame*>* BufferHasher::lookup(uint64_t pageId)
{
	return &hashTable[hash(pageId)]; 
}


// _____________________________________________________________________________
BufferFrame* BufferHasher::nextFrame()
{
	auto temp = framePool[currentFrameIndex];
	currentFrameIndex = (currentFrameIndex + 1) % size;
	return temp;
}


// _____________________________________________________________________________
void BufferHasher::remove(uint64_t pageId)
{
	lockBucket(pageId);
	std::vector<BufferFrame*>* frames = lookup(pageId);
	for (size_t i = 0; i < frames->size(); i++)
	{
		BufferFrame* frame = frames->at(i);
		if(frame->pageId == pageId)
		{
			frames->erase(frames->begin()+i);
			break;
		}
	}
	unlockBucket(pageId);
}

// _____________________________________________________________________________
void BufferHasher::lockBucket(uint64_t bucket)
{
	if (bucket >= size) return;
	locks->at(hash(bucket)).lock();
}

// _____________________________________________________________________________
void BufferHasher::unlockBucket(uint64_t bucket)
{
	if (bucket >= size) return;
	locks->at(hash(bucket)).unlock();
}

// _____________________________________________________________________________
uint64_t BufferHasher::getSize()
{
	return this->size;
}
