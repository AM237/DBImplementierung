
///////////////////////////////////////////////////////////////////////////////
// SumOfFactors.h
//////////////////////////////////////////////////////////////////////////////


#ifndef EXTERNALSORT_H
#define EXTERNALSORT_H

#include <gtest/gtest.h>
#include <string>
#include <queue>
#include <iostream>

// ***************************************************************************
// Constants
// ***************************************************************************

namespace
{
    // Path to where directory containing the runs should be stored
	 const std::string runDirPath = "./";
	
	// Name of runs directory
	 const std::string runDirName = "runs";
	
	// Runs naming scheme
	 const std::string runName = "runNr";
}

// ***************************************************************************
// Structs, templates, types
// ***************************************************************************

// Priority queue predicate, priority queue delivers smallest? element
struct OrderBySize
{
    bool operator() ( uint64_t const a, uint64_t const b) { return a > b; }
};

// Priority queue, delivers smallest element, vector container
typedef std::priority_queue<uint64_t, std::vector<uint64_t>, OrderBySize> 
        prioritysize_queue;
 
        
// ***************************************************************************
// Main class
// ***************************************************************************

// Class with procedures for external sorting        
class ExternalSort
{

public:
	// Calls private overload with standard options.
	// Refer to private overload for parameters.
	void externalSort(int fdInput, uint64_t size, int fdOutput, 
	                  uint64_t memSize);

	                  
 	// Main point of entry, IO proxy for the underlying externalSort method.
 	// Refer to the private overload for parameters
	void externalSort(const char* inputFile, uint64_t size, 
	                  const char* outputFile, uint64_t memSize, 
	                  bool readableRuns, bool verbose, bool nocleanup,
	                  bool replSelect);
                  
private:


	// Wrapper for externalSort.
	// Reads file with descriptor fdInput and performs an external sort of the 
	// uing64_t integers contained therein, writing the output to file with 
	// descriptor fdOutput. Buffer size (b) is given by memSize. must be 
	// divisible by sizeof(uint64_t)
	// size gives the number of integer values to consider. size = -1 sorts 
	// all values in the input file.
	// See program usage for bool options / parameters
	FRIEND_TEST(ExternalSortTest, externalSort1MBBufferNoReplSelect);
	FRIEND_TEST(ExternalSortTest, externalSort1MBBufferReplSelect);
	void externalSort(int fdInput, uint64_t size, int fdOutput, 
					  uint64_t memSize, bool readableRuns, bool verbose, 
					  bool nocleanup, bool replSelect);
                  
	// Helper function for externalSort, takes an input file descriptor and 
	// produces sorted partitions (runs) of the uint64_t data held in the file.
	// size > 0 gives the number of integer values to consider
	// The buffer size (MB) is dictated by memSize, verbose controls amount 
	// of feedback. Returns number of created runs.  
	int makeSortedRuns(int fdInput, uint64_t size, uint64_t memSize,
                    bool readableRuns, bool verbose);
                    
    // As above, but uses replacement selection
    int makeSortedRunsReplSel(int fdInput, uint64_t size, uint64_t memSize,
                    bool readableRuns, bool verbose);    

    // Helper function to merge the runs, parameters are the memSize to use,
	// the output file descriptor and the number of runs
	void mergeSortedRuns(uint64_t memSize, int fdOutput, int runs);     

};

#endif  // EXTERNALSORT_H

