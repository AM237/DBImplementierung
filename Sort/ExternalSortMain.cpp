
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
	
	cout << endl << "./ExternalSortMain <inputFile> <outputFile> "
	     << "<memoryBufferInMB> ";
	cout << endl;
	cout << "Run with --readableRuns to write each run file in readable "
	     << "(txt) format" << endl;

	cout << endl << " ***************************************** " << endl;
	exit(1);
}

// command line options
struct option options[] =
{
	{"readableRuns",	0, NULL, 'r'},
	{"verbose",			0, NULL, 'v'},
	{NULL, 				0, NULL,  0 }
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
	
	// Parse options
	optind = 1;
	while(true)
	{
		int c = getopt_long(argc, argv, "rv", options, NULL);
		if (c == -1) break;
		switch (c)
		{
			case 'r':
				readableRuns = true; break;
			case 'v':
				verbose = true; break;
			default: printUsage();
		}
	}
	
	// Parse input
	if (argc != optind+3) printUsage();
	inputFile = argv[optind];
	outputFile = argv[optind+1];
	memBuffer = argv[optind+2];
	
	// Get handle to input and output files
	FILE* pFile;
	FILE* oFile;
 	pFile = fopen (inputFile,"rb");
 	oFile = fopen (outputFile, "wb");
 	
  	if (pFile==NULL) cerr << "Unable to open the input file" << endl;
  	if (oFile==NULL) cerr << "Unable to open the output file" << endl;
  	
  	// Call sorting algorithm
  	externalSort(fileno(pFile), 64, fileno(oFile), atoi(memBuffer), 
  	             readableRuns, verbose);
  	
  	fclose(pFile);
  	fclose(oFile);
  	
  	return 0;
}
