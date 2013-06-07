
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
		
	fclose(db);
		
	// Check contents: SI
	ASSERT_EQ(pages[0], 2);
	ASSERT_EQ(pages[1], 0);
	ASSERT_EQ(pages[2], 0);
	ASSERT_EQ(pages[3], 1);
	
	ASSERT_EQ(pages[4], 1);
	ASSERT_EQ(pages[5], 1);
	ASSERT_EQ(pages[6], 2);
	
	for (int i = 7; i < intsPerPage; i++) ASSERT_EQ(pages[i], 0);
		
	// Check contents: FSI
	ASSERT_EQ(pages[intsPerPage], 1);
	ASSERT_EQ(pages[intsPerPage+1], 2);
	ASSERT_EQ(pages[intsPerPage+2], 3);
	
	for (int i = intsPerPage+3; i < 3*intsPerPage; i++) ASSERT_EQ(pages[i], 0);
	
	// Cleanup
	if (system("rm database") < 0) 
  		cout << "Error removing database" << endl;
}



// _____________________________________________________________________________
TEST(SegmentManagerTest, initializeWithFile)
{
	// Create a data file that represents a snapshot of the db as created
	// by a previous instance of a segment manager. The pages layout looks
	// as follows:
	//
	// | 0  | 1   | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10| 11   | 12   | 13| 14   |
	// | SI | FSI | 4 | 2 | 3 | 2 | 3 | 3 | 2 | 4 | 4 | Free | Free | 2 | Free |

	uint64_t size = constants::pageSize/sizeof(uint64_t);
	vector<uint64_t> pages;
	pages.resize(15 * size, 0);
	
	// Set page contents
	for (unsigned int i = 2*size; i < 3*size; i++) pages[i] = 4;
	for (unsigned int i = 3*size; i < 4*size; i++) pages[i] = 2;
	for (unsigned int i = 4*size; i < 5*size; i++) pages[i] = 3;
	for (unsigned int i = 5*size; i < 6*size; i++) pages[i] = 2;
	for (unsigned int i = 6*size; i < 8*size; i++) pages[i] = 3;
	for (unsigned int i = 8*size; i < 9*size; i++) pages[i] = 2;
	for (unsigned int i = 9*size; i < 11*size; i++) pages[i] = 4;
	for (unsigned int i = 13*size; i < 14*size; i++) pages[i] = 2;
	
	// Encode SI
	vector<uint64_t> sivec = { 10, 0, 0, 1,
	                               1, 1, 2, 
	                               4, 2, 3,
	                               2, 3, 4,
	                               3, 4, 5,
	                               2, 5, 6,
	                               3, 6, 8,
	                               2, 8, 9,
	                               4, 9, 11,
	                               2, 13, 14 };
	// Encode FSI
	vector<uint64_t> fsivec = { 2, 11, 13,
	                               14, 15 }; 
	                                                    
	// Build data vector
	sivec.resize(size, 0);
	fsivec.resize(size, 0);
	for (size_t i = 0; i < sivec.size(); i++) 
	{
		pages[i] = sivec[i];
		pages[size+i] = fsivec[i];
	}
	
	// Create file
	FILE* testFile;
 	testFile = fopen ("database", "wb");
	if (write(fileno(testFile), pages.data(), 15 * constants::pageSize) < 0)
		std::cout << "initializeWithFile: error writing to testFile" << endl;
	fclose(testFile);
	
	
	// Check components initialization ----------------------------------------
	SegmentManager* sm = new SegmentManager("database");
	ASSERT_TRUE(sm->bm != nullptr);
	ASSERT_TRUE(sm->segInv != nullptr);
	ASSERT_TRUE(sm->spaceInv != nullptr);
	
	
	// Check SI ----------------------------------------------------------------
	SegmentInventory* si = sm->segInv;
	ASSERT_EQ(si->bm, sm->bm);
	ASSERT_EQ(si->id, 0);
	ASSERT_EQ(si->nextId, 5);
	ASSERT_EQ(si->numEntries, 10);
	ASSERT_EQ(si->nextPageCounter, 0);
	ASSERT_FALSE(si->visible);
	ASSERT_TRUE(si->permanent);
	ASSERT_EQ(si->extents.size(), 1);
	ASSERT_EQ(si->extents[0].start, 0);
	ASSERT_EQ(si->extents[0].end, 1);
	
	// Check segment mapping
	ASSERT_EQ(si->segments.size(), 5);
	ASSERT_NE(si->segments.find(0), si->segments.end());
	ASSERT_NE(si->segments.find(1), si->segments.end());
	ASSERT_NE(si->segments.find(2), si->segments.end());
	ASSERT_NE(si->segments.find(3), si->segments.end());
	ASSERT_NE(si->segments.find(4), si->segments.end());
	
	// Check FSI ---------------------------------------------------------------
	FreeSpaceInventory* fsi = sm->spaceInv;
	ASSERT_EQ(fsi->bm, sm->bm);
	ASSERT_EQ(fsi->id, 1);
	ASSERT_EQ(fsi->numEntries, 2);
	ASSERT_EQ(fsi->nextPageCounter, 0);
	ASSERT_FALSE(fsi->visible);
	ASSERT_TRUE(fsi->permanent);
	
	// Check entries
	ASSERT_EQ(fsi->forwardMap.size(), 2);
	ASSERT_EQ(fsi->reverseMap.size(), 2);
	ASSERT_NE(fsi->forwardMap.find(11), fsi->forwardMap.end());
	ASSERT_NE(fsi->forwardMap.find(14), fsi->forwardMap.end());
	ASSERT_NE(fsi->reverseMap.find(13), fsi->reverseMap.end());
	ASSERT_NE(fsi->reverseMap.find(15), fsi->reverseMap.end());
	ASSERT_EQ(fsi->forwardMap.find(11)->second, 13);
	ASSERT_EQ(fsi->forwardMap.find(14)->second, 15);
	ASSERT_EQ(fsi->reverseMap.find(13)->second, 11);
	ASSERT_EQ(fsi->reverseMap.find(15)->second, 14);
	
	ASSERT_EQ(fsi->extents.size(), 1);
	ASSERT_EQ(fsi->extents[0].start, 1);
	ASSERT_EQ(fsi->extents[0].end, 2);
	
	// Check segments extents --------------------------------------------------
	ASSERT_EQ(si->segments.find(0)->second->getSize(), 1);
	ASSERT_EQ(si->segments.find(1)->second->getSize(), 1);
	ASSERT_EQ(si->segments.find(2)->second->getSize(), 4);
	ASSERT_EQ(si->segments.find(3)->second->getSize(), 3);
	ASSERT_EQ(si->segments.find(4)->second->getSize(), 3);
	
	ASSERT_EQ(si->segments.find(2)->second->extents.size(), 4);
	ASSERT_EQ(si->segments.find(2)->second->extents[0].start, 3);
	ASSERT_EQ(si->segments.find(2)->second->extents[0].end, 4);
	ASSERT_EQ(si->segments.find(2)->second->extents[1].start, 5);
	ASSERT_EQ(si->segments.find(2)->second->extents[1].end, 6);
	ASSERT_EQ(si->segments.find(2)->second->extents[2].start, 8);
	ASSERT_EQ(si->segments.find(2)->second->extents[2].end, 9);
	ASSERT_EQ(si->segments.find(2)->second->extents[3].start, 13);
	ASSERT_EQ(si->segments.find(2)->second->extents[3].end, 14);
	
	ASSERT_EQ(si->segments.find(3)->second->extents.size(), 2);
	ASSERT_EQ(si->segments.find(3)->second->extents[0].start, 4);
	ASSERT_EQ(si->segments.find(3)->second->extents[0].end, 5);
	ASSERT_EQ(si->segments.find(3)->second->extents[1].start, 6);
	ASSERT_EQ(si->segments.find(3)->second->extents[1].end, 8);
	
	ASSERT_EQ(si->segments.find(4)->second->extents.size(), 2);
	ASSERT_EQ(si->segments.find(4)->second->extents[0].start, 2);
	ASSERT_EQ(si->segments.find(4)->second->extents[0].end, 3);
	ASSERT_EQ(si->segments.find(4)->second->extents[1].start, 9);
	ASSERT_EQ(si->segments.find(4)->second->extents[1].end, 11);
	
	// Check segment contents --------------------------------------------------
	Segment* seg;
	vector<uint64_t> segpages;
	
	seg = si->segments.find(2)->second;
	ASSERT_EQ(3, seg->nextPage());
	ASSERT_EQ(5, seg->nextPage());
	ASSERT_EQ(8, seg->nextPage());
	ASSERT_EQ(13, seg->nextPage());
	ASSERT_EQ(13, seg->nextPage());
	ASSERT_EQ(13, seg->nextPage());
	
	segpages = { 3, 5, 8, 13 };
	for (size_t i = 0; i < segpages.size(); i++)
	{
		BufferFrame& bf = si->bm->fixPage(segpages[i], false);
		uint64_t* data = (uint64_t*)bf.getData();
		for (uint64_t i = 0; i < size; i++) ASSERT_EQ(data[i], 2);
		si->bm->unfixPage(bf, false);
	}
	
	seg = si->segments.find(3)->second;
	ASSERT_EQ(4, seg->nextPage());
	ASSERT_EQ(6, seg->nextPage());
	ASSERT_EQ(7, seg->nextPage());
	ASSERT_EQ(7, seg->nextPage());
	ASSERT_EQ(7, seg->nextPage());
	
	segpages.clear();
	segpages = { 4, 6, 7 };
	for (size_t i = 0; i < segpages.size(); i++)
	{
		BufferFrame& bf = si->bm->fixPage(segpages[i], false);
		uint64_t* data = (uint64_t*)bf.getData();
		for (uint64_t i = 0; i < size; i++) ASSERT_EQ(data[i], 3);
		si->bm->unfixPage(bf, false);
	}
	
	
	seg = si->segments.find(4)->second;
	ASSERT_EQ(2, seg->nextPage());
	ASSERT_EQ(9, seg->nextPage());
	ASSERT_EQ(10, seg->nextPage());
	ASSERT_EQ(10, seg->nextPage());
	
	segpages.clear();
	segpages = { 2, 9, 10 };
	for (size_t i = 0; i < segpages.size(); i++)
	{
		BufferFrame& bf = si->bm->fixPage(segpages[i], false);
		uint64_t* data = (uint64_t*)bf.getData();
		for (uint64_t i = 0; i < size; i++) ASSERT_EQ(data[i], 4);
		si->bm->unfixPage(bf, false);
	}
	
	// Check free space --------------------------------------------------------
	vector<uint64_t> emptypages = { 11, 12, 14 };
	for (size_t i = 0; i < emptypages.size(); i++)
	{
		BufferFrame& bf = si->bm->fixPage(emptypages[i], false);
		uint64_t* data = (uint64_t*)bf.getData();
		for (uint64_t i = 0; i < size; i++) ASSERT_EQ(data[i], 0);
		si->bm->unfixPage(bf, false);
	}
	
		
	// Cleanup
	delete sm;
	if (system("rm database") < 0) 
  		cout << "Error removing database" << endl;
}



////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
