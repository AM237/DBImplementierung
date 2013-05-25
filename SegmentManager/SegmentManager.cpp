
///////////////////////////////////////////////////////////////////////////////
// SegmentManager.cpp
///////////////////////////////////////////////////////////////////////////////

#include "SegmentManager.h"

#include <fcntl.h>
#include <iostream>
#include <vector>

using namespace std;

// _____________________________________________________________________________
SegmentManager::SegmentManager(char* filename)
{
	initializeDatabase(filename);
	
	// segment inventory always has id = 0, space inventory always has id = 1
	segInv = new SegmentInventory(fileno(dbFile), false, 0);
	spaceInv = new FreeSpaceInventory(fileno(dbFile), false, 1);
}

// _____________________________________________________________________________
uint64_t SegmentManager::createSegment()
{
	return 0;
}

// _____________________________________________________________________________
void SegmentManager::dropSegment(uint64_t segId)
{

}

// _____________________________________________________________________________
uint64_t SegmentManager::growSegment(uint64_t segId)
{
	return 0;
}

// _____________________________________________________________________________
Segment& SegmentManager::retrieveSegmentById(uint64_t segId)
{
	
}

// _____________________________________________________________________________
void SegmentManager::initializeDatabase(char* filename)
{
	dbFile = fopen(filename, "r");
 	
 	// If file not existent, create standard file with 2 frames, one for each
 	// inventory. Otherwise, file exists, so inventories can be constructed.
 	if (dbFile == NULL)
  	{
  		if (errno == ENOENT)
  		{
  			dbFile = fopen (filename,"w+b");
  			if (dbFile == NULL)
  			{
  				cout << "Error creating database file (opening)" << endl;
  				exit(1);
  			}
  		
  			vector<uint64_t> twoFrames;
  			twoFrames.resize(2*constants::pageSize/sizeof(uint64_t), 0);
  		
  			// write initial inventory pages
			if (write(fileno(dbFile), twoFrames.data(), 
			    2*constants::pageSize) < 0)
			{
				cout << "Error creating database file (writing):"<<errno <<endl;
				exit(1);
			}
		
		} else {
		
  			cout << "Error opening the database file" << endl;
  			exit(1);
		}
		
  	} else {
  	
  		fclose(dbFile);
  		dbFile = fopen(filename, "w+b");
  	
		// Check file consistency: should be at least two pages worth of bytes,
		// and total number of bytes should be a multiple of the page size.
		if (lseek(fileno(dbFile), 0, SEEK_END) < 0)
		{
			cout << "Error seeking for file end" << endl;
			exit(1);
		}
		
		int fileBytes = ftell(dbFile);

		if (fileBytes % constants::pageSize != 0 ||
		    fileBytes < 2 * constants::pageSize)
		{
			cout << "Database file is not formatted correctly" << endl;
			exit(1);
		
		} else {
		
			if (lseek(fileno(dbFile), 0, SEEK_SET) < 0)
		    {
				cout << "Error seeking for start of file" << endl;
				exit(1);
			}
		}
  	}
}

// _____________________________________________________________________________
SegmentManager::~SegmentManager()
{
	fclose(dbFile);
	delete segInv;
	delete spaceInv;
}
