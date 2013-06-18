
///////////////////////////////////////////////////////////////////////////////
// BufferManagerTest.cpp
///////////////////////////////////////////////////////////////////////////////

// dd if=/dev/zero of=numData bs=1024 count=2048

#include "BufferManager.h"

using namespace std;

/*
// _____________________________________________________________________________
TEST(BufferManagerTest, flushFrameToFile)
{
	// Write a test file with 3 pages, filled with 'a', 'b', and 'c' resp.
	FILE* testFile;
 	testFile = fopen ("testFile", "wb");
 	vector<char> aVec(BM_CONS::pageSize, 'a');
 	vector<char> bVec(BM_CONS::pageSize, 'b');
 	vector<char> cVec(BM_CONS::pageSize, 'c');

	if ((write(fileno(testFile), aVec.data(), BM_CONS::pageSize) < 0) ||
		(write(fileno(testFile), bVec.data(), BM_CONS::pageSize) < 0) ||
		(write(fileno(testFile), cVec.data(), BM_CONS::pageSize) < 0))
			std::cout << "error writing to testFile" << endl;

	fclose(testFile);

	// Dummy BufferManager object	
	BufferManager bm("testFile", 1, 3);
	BufferFrame bf;

	bf.pageId= 0;	
	bf.data = cVec.data();
	bm.flushFrameToFile(bf);
	
	bf.pageId = 1;
	bf.data = aVec.data();
	bm.flushFrameToFile(bf);
	
	bf.pageId = 2;
	bf.data = bVec.data();
	bm.flushFrameToFile(bf);
	
	testFile = fopen ("testFile", "rb");
	for(int j = 0; j < 3; j++)
	{
		vector<char> input;
		input.resize(BM_CONS::pageSize);
		if (read(fileno(testFile), input.data(), BM_CONS::pageSize) < 0)
		std::cout << "error reading from testFile" << endl;
		
		for (size_t i = 0; i < input.size(); i++)
		{
			if (j == 0) ASSERT_EQ(input[i], 'c');
			if (j == 1) ASSERT_EQ(input[i], 'a');
			if (j == 2) ASSERT_EQ(input[i], 'b');
		}
	}
	fclose(testFile);
	
	// Cleanup
	if (system("rm testFile") < 0) 
  		cout << "Error removing testFile" << endl;
}



// _____________________________________________________________________________
TEST(BufferManagerTest, readPageIntoFrame)
{
	// Write a test file with 3 pages, filled with 'a', 'b', and 'c' resp.
	FILE* testFile;
 	testFile = fopen ("testFile", "wb");
 	vector<char> aVec(BM_CONS::pageSize, 'a');
 	vector<char> bVec(BM_CONS::pageSize, 'b');
 	vector<char> cVec(BM_CONS::pageSize, 'c');

	if ((write(fileno(testFile), aVec.data(), BM_CONS::pageSize) < 0) ||
		(write(fileno(testFile), bVec.data(), BM_CONS::pageSize) < 0) ||
		(write(fileno(testFile), cVec.data(), BM_CONS::pageSize) < 0))
		std::cout << "error writing to testFile" << endl;

	fclose(testFile);

	// Dummy BufferManager object	
	BufferManager bm("testFile", 1, 3);

	for (int i = 0; i < 3; i++)
	{
		BufferFrame* bf = new BufferFrame();
		bm.readPageIntoFrame(i, bf);
		
		ASSERT_TRUE(!bf->isDirty);
		ASSERT_TRUE(bf->pageFixed);
		
		char* data = static_cast<char*>(bf->getData());
		for (int j = 0; j < BM_CONS::pageSize; j++)
		{
			if (i == 0) ASSERT_EQ(data[j], 'a');
			if (i == 1) ASSERT_EQ(data[i], 'b');
			if (i == 2) ASSERT_EQ(data[i], 'c');			
		}
		delete bf;
	}
	
	// Cleanup
	if (system("rm testFile") < 0) 
  		cout << "Error removing testFile" << endl;
}


// _____________________________________________________________________________
TEST(BufferManagerTest, fixUnfixPageWithReplace)
{
	// Write a test file with 150 pages
	FILE* testFile;

 	testFile = fopen ("testFile", "wb");
 	vector<char> aVec(BM_CONS::pageSize, 'a');
 	vector<char> bVec(BM_CONS::pageSize, 'b');
 	vector<char> cVec(BM_CONS::pageSize, 'c');
   	

 	for (unsigned i=0; i<50; i++)
		if ((write(fileno(testFile), aVec.data(), BM_CONS::pageSize) < 0) ||
			(write(fileno(testFile), bVec.data(), BM_CONS::pageSize) < 0) ||
			(write(fileno(testFile), cVec.data(), BM_CONS::pageSize) < 0))
				std::cout << "error writing to testFile" << endl;
			

	fclose(testFile);
	
	// Construct BufferManager object with 3 BufferFrames, managing 150
	BufferManager* bm = new BufferManager("testFile", 3, 150);
	
	// page contains 'a's, 'b's, and 'c's respectively
	BufferFrame& aFrame = bm->fixPage(0, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	BufferFrame& bFrame = bm->fixPage(1, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	BufferFrame& cFrame = bm->fixPage(2, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	
	// buffer full and all pages fixed: should throw exception
	ASSERT_THROW(bm->fixPage(3, false), ReplaceFailAllFramesFixed);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
		
	// set candidate for replacement

	bm->unfixPage(bFrame, false);
	
	// set new page to contain all 'a's
	// buffer now contains pages 0, 3, 2
	BufferFrame& secondAFrame = bm->fixPage(3, false);
	//pthread_rwlock_unlock(&(bm->lock));

	//bm->lock.unlock();
	for (int i = 0; i < BM_CONS::pageSize; i++)
		ASSERT_EQ(((char*)secondAFrame.getData())[i], 'a');
		
	// buffer full and all pages fixed:
	// should throw exception if new page requested
	ASSERT_THROW(bm->fixPage(4, false), ReplaceFailAllFramesFixed);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	bm->unfixPage(cFrame, false);
	

	// set new page to contain all 'a's
	// buffer now contains pages 0, 3, 6
	BufferFrame& thirdAFrame = bm->fixPage(6, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	for (int i = 0; i < BM_CONS::pageSize; i++)
		ASSERT_EQ(((char*)thirdAFrame.getData())[i], 'a');
		
	// Unfix pages: update data, then check contents on disk
	for (int i = 0; i < BM_CONS::pageSize; i++)
	{
		((char*)aFrame.getData())[i] = 'd';
		((char*)secondAFrame.getData())[i] = 'd';
		((char*)thirdAFrame.getData())[i] = 'd';	
	}
	
	bm->unfixPage(aFrame, true);
	bm->unfixPage(secondAFrame, true);
	bm->unfixPage(thirdAFrame, true);
	
	testFile = fopen ("testFile", "rb");
	
	// pages to seek: 0, 3, 6

	vector<int> seek = { 0, 3, 6 };
	for (size_t i = 0; i < seek.size(); i++)
	{
		vector<char> inputBuffer;
		inputBuffer.resize(BM_CONS::pageSize);

		if (lseek(fileno(testFile), seek[i]*BM_CONS::pageSize, SEEK_SET) < 0)
		{
			cout << "Error seeking for page on disk" << endl;
			exit(1);
		}
		

		if (read(fileno(testFile), inputBuffer.data(), BM_CONS::pageSize) < 0)
			cout << "Error reading from testFile";
			
		for (int j = 0; j < BM_CONS::pageSize; j++)
			ASSERT_EQ(inputBuffer[j], 'd');
	}
	
	fclose(testFile);
	
	// Cleanup
	delete bm;

	
	if (system("rm testFile") < 0) 
  		cout << "Error removing testFile" << endl;
}



// _____________________________________________________________________________
TEST(BufferManagerTest, fixPageNoReplaceAndDestructor)
{
	// Write a test file with 150 pages
	FILE* testFile;
 	testFile = fopen ("testFile", "wb");
 	vector<char> aVec(BM_CONS::pageSize, 'a');
 	vector<char> bVec(BM_CONS::pageSize, 'b');
 	vector<char> cVec(BM_CONS::pageSize, 'c');
   	
 	for (unsigned i=0; i<50; i++)
		if ((write(fileno(testFile), aVec.data(), BM_CONS::pageSize) < 0) ||
			(write(fileno(testFile), bVec.data(), BM_CONS::pageSize) < 0) ||
			(write(fileno(testFile), cVec.data(), BM_CONS::pageSize) < 0))
				std::cout << "error writing to testFile" << endl;
			
	fclose(testFile);
	
	// Construct BufferManager object with 10 BufferFrames, managing 50 pages
	BufferManager* bm = new BufferManager("testFile", 10, 50);
	
	// page contains 'b's, 'c's, and 'a's respectively
	BufferFrame& bFrame = bm->fixPage(1, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	BufferFrame& cFrame = bm->fixPage(5, false);
	// pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	BufferFrame& aFrame = bm->fixPage(9, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();

	// BufferFrame pool: only 3 pages initialized with data
	int count = 0;
	for (size_t i = 0; i < bm->framePool.size(); i++)
		if (bm->framePool[i]->getData() != NULL)
			count++;
			
	ASSERT_EQ(count, 3);
	
	// BufferHasher before: only buckets for pages 1, 5, 9 have exactly 1 entry
	BufferHasher* hasher = bm->hasher;
	ASSERT_EQ(hasher->hashTable[hasher->hash(1)].size(), 1);
	ASSERT_EQ(hasher->hashTable[hasher->hash(5)].size(), 1);
	ASSERT_EQ(hasher->hashTable[hasher->hash(9)].size(), 1);
	ASSERT_EQ(hasher->hashTable[hasher->hash(0)].size(), 0);

	ASSERT_EQ(hasher->hashTable[hasher->hash(2)].size(), 0);
	ASSERT_EQ(hasher->hashTable[hasher->hash(3)].size(), 0);
	ASSERT_EQ(hasher->hashTable[hasher->hash(4)].size(), 0);
	ASSERT_EQ(hasher->hashTable[hasher->hash(6)].size(), 0);
	ASSERT_EQ(hasher->hashTable[hasher->hash(7)].size(), 0);
	ASSERT_EQ(hasher->hashTable[hasher->hash(8)].size(), 0);
	
	// Replacer before: all 3 pages in fifo queue, lru queue empty
	TwoQueueReplacer* replacer = (TwoQueueReplacer*)(bm->replacer);
	ASSERT_EQ(replacer->fifo.size(), 3);
	ASSERT_EQ(replacer->lru.size(), 0);
	
	// Request additional (buffered) frames
	BufferFrame& cBufferedFrame = bm->fixPage(5, false);
	//pthread_rwlock_unlock(&(bm->lock));

	//bm->lock.unlock();
	BufferFrame& newCFrame = bm->fixPage(11, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	BufferFrame& aBufferedFrame = bm->fixPage(9, false);
	//pthread_rwlock_unlock(&(bm->lock));

	//bm->lock.unlock();
	
	// BufferHasher after: bucket for newCFrame has exactly two entries
	ASSERT_EQ(hasher->hashTable[hasher->hash(1)].size(), 2);
	ASSERT_EQ(hasher->hashTable[hasher->hash(11)].size(), 2);
	ASSERT_EQ(hasher->hashTable[hasher->hash(5)].size(), 1);

	ASSERT_EQ(hasher->hashTable[hasher->hash(9)].size(), 1);
	ASSERT_EQ(hasher->hashTable[hasher->hash(2)].size(), 0);
	ASSERT_EQ(hasher->hashTable[hasher->hash(3)].size(), 0);
	ASSERT_EQ(hasher->hashTable[hasher->hash(0)].size(), 0);
	ASSERT_EQ(hasher->hashTable[hasher->hash(6)].size(), 0);

	ASSERT_EQ(hasher->hashTable[hasher->hash(7)].size(), 0);
	ASSERT_EQ(hasher->hashTable[hasher->hash(8)].size(), 0);
	
	// Replacer after: fifo size increased by one, two buffered frames then
	// moved to lru queue
	ASSERT_EQ(replacer->fifo.size(), 2);

	ASSERT_EQ(replacer->lru.size(), 2);
	
	// Request buffered frame that is in LRU, check that it is moved up.
	BufferFrame& aFrameFromLRU = bm->fixPage(9, false);
	//pthread_rwlock_unlock(&(bm->lock));

	//bm->lock.unlock();
	ASSERT_EQ(replacer->lru.front()->pageId, 9);
	
	// Check frame contents
	for (int i = 0; i < BM_CONS::pageSize; i++)
	{
		ASSERT_EQ(((char*)aFrame.getData())[i], 'a');
		ASSERT_EQ(((char*)bFrame.getData())[i], 'b');
		ASSERT_EQ(((char*)cFrame.getData())[i], 'c');
		ASSERT_EQ(((char*)cBufferedFrame.getData())[i], 'c');
		ASSERT_EQ(((char*)newCFrame.getData())[i], 'c');
		ASSERT_EQ(((char*)aBufferedFrame.getData())[i], 'a');
		ASSERT_EQ(((char*)aFrameFromLRU.getData())[i], 'a');
	}
	
	// Fill frame buffer pool, expect an exception to be thrown
	bm->fixPage(12, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	bm->fixPage(13, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	bm->fixPage(14, false);

	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	bm->fixPage(15, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();

	bm->fixPage(16, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	bm->fixPage(17, false);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	ASSERT_THROW(bm->fixPage(18, false), ReplaceFailAllFramesFixed);
	//pthread_rwlock_unlock(&(bm->lock));
	//bm->lock.unlock();
	
	// Cleanup
	delete bm;

	
	if (system("rm testFile") < 0) 
  		cout << "Error removing testFile" << endl;
}


// _____________________________________________________________________________
TEST(BufferManagerTest, constructor)
{
	// Write a test file with 50 pages
	FILE* testFile;
 	testFile = fopen ("testFile", "wb");
   	vector<char> aVec(BM_CONS::pageSize, 'a');
   	
 	for (unsigned i=0; i<50; i++)
		if ((write(fileno(testFile), aVec.data(), BM_CONS::pageSize) < 0))			
			std::cout << "error writing to testFile" << endl;
			
	fclose(testFile);
	
	// Construct BufferManager object with 10 BufferFrames, managing 50 pages
	BufferManager* bm = new BufferManager("testFile", 10, 50);
	
	// Fields
	ASSERT_EQ(bm->numFrames, 10);
	ASSERT_TRUE(bm->fileDescriptor > 0);
	
	// BufferHasher
	BufferHasher* hasher = bm->hasher;
	ASSERT_TRUE(hasher != NULL);
	ASSERT_EQ(hasher->size, 10);
	ASSERT_EQ(hasher->hashTable.size(), 10);
	
	for (size_t i = 0; i < hasher->hashTable.size(); i++)
	{
		vector<BufferFrame*> frameVec = hasher->hashTable[i];
		ASSERT_EQ(frameVec.size(), 0);
	}
	
	// Test locking
	//pthread_rwlock_wrlock(&(hasher->lockVec->at(5)));
	//pthread_rwlock_unlock(&(hasher->lockVec->at(5)));
	
	// Replacer
	TwoQueueReplacer* replacer = (TwoQueueReplacer*)(bm->replacer);
	ASSERT_EQ(replacer->fifo.size(), 0);
	ASSERT_EQ(replacer->lru.size(), 0);
	
	
	// BufferFrame pool
	ASSERT_EQ(bm->framePool.size(), 10);
	
	for (size_t i = 0; i < bm->framePool.size(); i++)
	{
		BufferFrame* frame = bm->framePool[i];
		ASSERT_TRUE(frame != NULL);
		ASSERT_TRUE(frame->data == NULL);
	}
	
	// Cleanup
	delete bm;
	if (system("rm testFile") < 0) 
  		cout << "Error removing testFile" << endl;
}

*/
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
