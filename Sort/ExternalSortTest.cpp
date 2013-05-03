
///////////////////////////////////////////////////////////////////////////////
// ExternalSortTest.cpp
///////////////////////////////////////////////////////////////////////////////


#include <gtest/gtest.h>
#include "ExternalSort.h"


// Test a 4MB input file with a 1MB buffer
TEST(ExternalSortTest, externalSort1MBBuffer)
{
	// Create a ~4MB file with uint64_t values in descending order
	// (data written in binary format)
	FILE* testFile;
	FILE* testOutputFile;
 	testFile = fopen ("testFile", "wb");
 	testOutputFile = fopen ("testOutputFile", "wb");
 	
 	// 4 MB
 	int n = 500000;
 	// 5 GB
 	//int n = 640000000;
 	
 	for (unsigned i=n; i>0; i--) {
		uint64_t x = i;
		if (write(fileno(testFile), &x, sizeof(uint64_t)) < 0) {
			std::cout << "error writing to testFile" << endl;
		}
	}
	
	fclose(testFile);
	testFile = fopen("testFile", "rb");


	// Call sorting function	
	int bufferSize = 1 * 1024 * 1024;
  	int numElements = bufferSize / sizeof(uint64_t);
	
	ExternalSort sort;
	sort.externalSort(fileno(testFile), -1, fileno(testOutputFile), bufferSize);
	
    testOutputFile = fopen ("testOutputFile", "rb");
	
 	// Read output file and verify order (blockwise)
  	int readState = 0;
  	do
  	{  		
  		// Allocate buffer (make as large as possible to minimize
  		// the number of reads required)
		uint64_t* buffer = new uint64_t[numElements];
	
		readState =  read(fileno(testOutputFile), buffer, bufferSize);  		
  		

  		if (readState < 0) cerr << "Error reading file into buffer " << readState << endl;
  		if (readState == 0) 
  		{
  			delete[] buffer;
  			break;
  		}
  		
  		// If EOF encountered, make sure only relevant elements are taken
  		// into account for sorting
  		int limit = numElements;
  		if (readState < bufferSize)
  			limit = readState / sizeof(uint64_t);
  		
  		// Check ordering
  		for (int i = 0; i < limit-1; i++)
  			ASSERT_TRUE(buffer[i] < buffer[i+1]);
  			
		// Deallocate memory
  		delete[] buffer;
  	
  	} while (readState != 0);
  	
  	// Cleanup
  	fclose(testFile);
  	fclose(testOutputFile);
  	if (system("rm testFile") < 0) 
  		cout << "Error removing testFile" << endl;
	if (system("rm testOutputFile") < 0) 
		cout << "Error removing testOutputFile" << endl;
		
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
