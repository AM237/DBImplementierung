
///////////////////////////////////////////////////////////////////////////////
// ExternalSortMain.cpp
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>

#include "BufferManager.h"

using namespace std;

// Prints usage (parameters + options) of the program, and then exits
void printUsage()
{
	cout << endl << " **************** Usage: *****************" << endl;
	


	cout << endl << " ***************************************** " << endl;
	exit(1);
}

// command line options
struct option options[] =
{
	{NULL, 				0, NULL,  0 }
};


// main routine
int main(int argc, char** argv)
{
	
	// Parse options
	optind = 1;
	while(true)
	{
		int c = getopt_long(argc, argv, "", options, NULL);
		if (c == -1) break;
		switch (c)
		{
			default: printUsage();
		}
	}

    
  	return 0;
  	
}
