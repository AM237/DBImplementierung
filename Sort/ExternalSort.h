
///////////////////////////////////////////////////////////////////////////////
// SumOfFactors.h
//////////////////////////////////////////////////////////////////////////////


#ifndef EXTERNALSORT_H
#define EXTERNALSORT_H

#include <stdint.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <queue>


// Constants
namespace
{
    // Path to where directory containing the runs should be stored
	 const std::string runDirPath = "./";
	
	// Name of runs directory
	 const std::string runDirName = "runs";
	
	// Runs naming scheme
	 const std::string runName = "runNr";
}



using namespace std;

class ExternalSort
{

public: 
	// Reads file with descriptor fdInput and performs an external sort of the 
	// uing64_t integers contained therein, writing the output to file with 
	// descriptor fdOutput. Buffer size (b) is given by memSize. size gives the
	// number of integer values to consider. size = -1 sorts all values in the
	// input file.
	void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize);


	// Wrapper for externalSort, readableRuns dictates whether the individual runs
	// are written to file in binary or as a regular text file (default: binary)
	// Verbose controls amount of feedback
	void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize, 
                  bool readableRuns, bool verbose, bool nocleanup);
                  
private:
                  
	// Helper function for externalSort, takes an input file descriptor and 
	// produces sorted partitions (runs) of the uint64_t data held in the file.
	// size > 0 gives the number of integer values to consider
	// The buffer size (MB) is dictated by memSize, verbose controls amount 
	// of feedback. Returns number of created runs.  
	int makeSortedRuns(int fdInput, uint64_t size, uint64_t memSize,
                    bool readableRuns, bool verbose);        

        // Helper function to merge the runs, parameters are the memSize to use, the 
	// output file descriptor and the number of runs
	void mergeSortedRuns(uint64_t memSize, int fdOutput, int runs);     

};

#endif  // EXTERNALSORT_H

