
///////////////////////////////////////////////////////////////////////////////
// BufferManagerTest.cpp
///////////////////////////////////////////////////////////////////////////////


#include <gtest/gtest.h>
#include <iostream>
#include "BufferManager.h"

using namespace std;

// Test the construction process of the buffer manager
TEST(BufferManagerTest, constructorDestructor)
{

	// Write a test file with 51 pages, pages 0, 3, 6 , ... is filled with 'a'
	// pages 1, 4, 7, ... filled with 'b', and pages 2, 5, 8, .. filled with 'c'
	// Note: file must be opened with O_RDWR flag, otherwise a bus error is
	// possible	
	FILE* testFile;
 	testFile = fopen ("testFile", "wb");
 	for (unsigned i=0; i<51; i=i+3)
    {
		for(int j = 0; j < 4096; j++)
		{
			char x = 'a';
			if (write(fileno(testFile), &x, sizeof(char)) < 0)
			std::cout << "error writing to testFile" << endl;
		}
		
		for(int j = 0; j < 4096; j++)
		{
			char x = 'b';
			if (write(fileno(testFile), &x, sizeof(char)) < 0) 
			std::cout << "error writing to testFile" << endl;
		}
		
		for(int j = 0; j < 4096; j++)
		{
			char x = 'c';
			if (write(fileno(testFile), &x, sizeof(char)) < 0) 
			std::cout << "error writing to testFile" << endl;
		}	
	}	
	fclose(testFile);
	
	// Construct BufferManager object with 10 BufferFrames
	BufferManager* bm = new BufferManager("testFile", 10);
	
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
	
	// TwoQueues
	TwoQueues* tq = bm->twoQueues;
	ASSERT_EQ(tq->fifo.size(), 0);
	ASSERT_EQ(tq->lru.size(), 0);
	
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


int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
