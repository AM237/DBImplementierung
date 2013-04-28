
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

using namespace std;

// Reads file with descriptor fdInput and performs an external sort of the 
// uing64_t integers contained therein, writing the output to file with 
// descriptor fdOutput. Buffer size is given by memSize. 
void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize);


// Wrapper for externalSort, readableRuns dictates whether the individual runs
// are written to file in binary or as a regular text file (default: binary)
// Verbose controls amount of feedback
void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize, 
                  bool readableRuns, bool verbose);
                  
// Helper function for externalSort, takes an input file descriptor and 
// produces sorted partitions (runs) of the uint64_t data held in the file.
// The buffer size is dictated by memSize, verbose controls amount of feedback                
void makeSortedRuns(int fdInput, uint64_t size, uint64_t memSize,
                    bool readableRuns, bool verbose);             


#endif  // EXTERNALSORT_H

