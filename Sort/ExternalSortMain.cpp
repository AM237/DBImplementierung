
///////////////////////////////////////////////////////////////////////////////
// ExternalSortMain.cpp
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "ExternalSort.h"

using namespace std;

// Prints usage (parameters + options) of the program, and then exits
void printUsage()
{
	cout << endl << " **************** Usage: *****************" << endl;
	
	cout << endl << "./<executable name> <inputFile> <outputFile> "
	     << "<memoryBufferInMB> ";
	cout << endl;
	cout << "Run with --readableRuns to write each run file in readable "
	     << "(txt) format" << endl;
	cout << "Run with --verbose to get more feedback " << endl;
	cout << "Run with --nocleanup to save the runs on disk " << endl;
	cout << "Run with --replSelect to use replacement selection " << endl;

	cout << endl << " ***************************************** " << endl;
	exit(1);
}

// command line options
struct option options[] =
{
	{"readableRuns",			0, NULL, 'r'},
	{"verbose",					0, NULL, 'v'},
	{"nocleanup",				0, NULL, 'n'},
	{"replSelect", 				0, NULL, 's'},
	{NULL, 						0, NULL,  0 }
};


// main routine
int main(int argc, char** argv)
{
	// Input parameters, constants, debug
	const char* inputFile = NULL;
	const char* outputFile = NULL;
	const char* memBuffer = NULL;
	bool readableRuns = false;
	bool verbose = false;
	bool nocleanup = false;
	bool replSelect = false;
	
	// Parse options
	optind = 1;
	while(true)
	{
		int c = getopt_long(argc, argv, "rvns", options, NULL);
		if (c == -1) break;
		switch (c)
		{
			case 'r':
				readableRuns = true; break;
			case 'v':
				verbose = true; break;
			case 'n':
				nocleanup = true; break;
			case 's':
				replSelect = true; break;
			default: printUsage();
		}
	}
	
	// Parse input
	if (argc != optind+3) printUsage();
	inputFile = argv[optind];
	outputFile = argv[optind+1];
	memBuffer = argv[optind+2];
	
  	// Call sorting algorithm
  	ExternalSort sort;
  	sort.externalSort(inputFile, -1, outputFile,
  					  atoi(memBuffer)*1024*1024, 
  	                  readableRuns, verbose, nocleanup, replSelect);
    	
  	return 0;
}
