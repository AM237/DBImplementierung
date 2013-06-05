
///////////////////////////////////////////////////////////////////////////////
// SegmentManagerTest.cpp
///////////////////////////////////////////////////////////////////////////////

#include "SegmentManager.h"

using namespace std;


// _____________________________________________________________________________
TEST(SegmentManagerTest, initializeNoFile)
{
	// Check components initialization ----------------------------------------
	SegmentManager* sm = new SegmentManager("database");
	ASSERT_TRUE(sm->bm != nullptr);
	ASSERT_TRUE(sm->segInv != nullptr);
	ASSERT_TRUE(sm->spaceInv != nullptr);
	
	// Check SI ---------------------------------------------------------------
	SegmentInventory* si = sm->segInv;
	ASSERT_EQ(si->bm, sm->bm);
	ASSERT_EQ(si->id, 0);
	ASSERT_EQ(si->nextId, 2);
	ASSERT_EQ(si->numEntries, 2);
	ASSERT_EQ(si->nextPageCounter, 0);
	ASSERT_FALSE(si->visible);
	ASSERT_TRUE(si->permanent);
	
	// The segments mapping should contain only one extra entry, that of the FSI
	ASSERT_EQ(si->segments.size(), 2);
	ASSERT_NE(si->segments.find(1), si->segments.end());
	ASSERT_NE(si->segments.find(0), si->segments.end());
	ASSERT_EQ(si->segments.find(1)->second->id, 1);
	ASSERT_EQ(si->segments.find(0)->second->id, 0);
	
	ASSERT_EQ(si->extents.size(), 1);
	ASSERT_EQ(si->extents[0].start, 0);
	ASSERT_EQ(si->extents[0].end, 1);
	
	// Check FSI ---------------------------------------------------------------
	FreeSpaceInventory* fsi = sm->spaceInv;
	ASSERT_EQ(fsi->bm, sm->bm);
	ASSERT_EQ(fsi->id, 1);
	ASSERT_EQ(fsi->numEntries, 1);
	ASSERT_EQ(fsi->nextPageCounter, 0);
	ASSERT_FALSE(fsi->visible);
	ASSERT_TRUE(fsi->permanent);
	
	// The free space mapping should span one page, namely page #2
	ASSERT_EQ(fsi->forwardMap.size(), 1);
	ASSERT_EQ(fsi->reverseMap.size(), 1);
	ASSERT_NE(fsi->forwardMap.find(2), fsi->forwardMap.end());
	ASSERT_NE(fsi->reverseMap.find(3), fsi->reverseMap.end());
	ASSERT_EQ(fsi->forwardMap.find(2)->second, 3);
	ASSERT_EQ(fsi->reverseMap.find(3)->second, 2);
	
	ASSERT_EQ(fsi->extents.size(), 1);
	ASSERT_EQ(fsi->extents[0].start, 1);
	ASSERT_EQ(fsi->extents[0].end, 2);
	
	delete sm;

	// Check actual file created -----------------------------------------------
	FILE* db = fopen("database", "rb");
	if (lseek(fileno(db), 0, SEEK_END) < 0)
	{
		cout << "Error seeking to end of file: " << errno << endl;
		exit(1);
	}
	uint64_t fileBytes = ftell(db);
	
	// Check size of db
	ASSERT_EQ(fileBytes, 3*constants::pageSize);
	
	// Read in the individual pages
	vector<uint64_t> pages;
	int intsPerPage = constants::pageSize/sizeof(uint64_t);
	pages.resize(3*intsPerPage);

	if (lseek(fileno(db), 0, SEEK_SET) < 0)
	{
		cout << "Error seeking to start of file: " << errno << endl;
		exit(1);
	}
	if (read(fileno(db), pages.data(), 3*constants::pageSize) < 0)
		std::cout << "Error reading test db " << endl;
	
		
	// Check contents: SI
	ASSERT_EQ(pages[0], 2);
	ASSERT_EQ(pages[1], 0);
	ASSERT_EQ(pages[2], 0);
	ASSERT_EQ(pages[3], 1);
	
	ASSERT_EQ(pages[4], 1);
	ASSERT_EQ(pages[5], 1);
	ASSERT_EQ(pages[6], 2);
	
	for (int i = 7; i < intsPerPage; i++)
		ASSERT_EQ(pages[i], 0);
		
	// Check contents: FSI
	ASSERT_EQ(pages[intsPerPage], 1);
	ASSERT_EQ(pages[intsPerPage+1], 2);
	ASSERT_EQ(pages[intsPerPage+2], 3);
	
	for (int i = intsPerPage+3; i < 3*intsPerPage; i++)
		ASSERT_EQ(pages[i], 0);
	
	// Cleanup
	if (system("rm database") < 0) 
  		cout << "Error removing database" << endl;
}


////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
