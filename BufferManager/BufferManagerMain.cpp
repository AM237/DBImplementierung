
///////////////////////////////////////////////////////////////////////////////
// ExternalSortMain.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BufferManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>

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
    
    
    
    BufferManager bm("testFile", 10);
    bm.fixPage(0, false);
    
    	
  	
  	
  	
  	/*
  	FILE* testFile;
 	testFile = fopen ("testFile", "wb");

 	
 	for (unsigned i=50; i>0; i=i-2)
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
	}
		
	
	
	fclose(testFile);
  	
  	cout << "finished writing to file" << endl;
  	exit(1);*/
  	
  	return 0;
  	
}
