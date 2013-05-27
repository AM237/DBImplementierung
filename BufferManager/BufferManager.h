
///////////////////////////////////////////////////////////////////////////////
// BufferManager.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include "BufferFrame.h"
#include "BufferHasher.h"
#include "FrameReplacer.h"
#include <mutex>


// ***************************************************************************
// Constants
// ***************************************************************************

namespace constants
{
	// Page size should be a multiple of the size of a page in virtual memory
	// const int pageSize = sysconf(_SC_PAGE_SIZE);
	const int pageSize = 4096;
	
	// The default number of pages to write to file when initializing
	// the database
	const int defaultNumPages = 50;
}

// ***************************************************************************
// Structs, templates, types, exceptions, etc.
// ***************************************************************************


class ReplaceFailAllFramesFixed: public std::exception
{
  virtual const char* what() const throw()
  {
    return "BufferManager could not replace frame - all frames are fixed.";
  }
};


class ReplaceFailFrameUnclean: public std::exception
{
  virtual const char* what() const throw()
  {
    return "Suggested frame for replacement is not clean.";
  }
};


class ReplaceFailNoFrameSuggested: public std::exception
{
  virtual const char* what() const throw()
  {
    return "No frame was suggested for replacement";
  }
};


// Implements RAII on a given lock
class ScopedLock
{
public:

	ScopedLock(std::mutex& lock) : m(lock) { }
	~ScopedLock() {  m.unlock();  } 
	
private:
	
	std::mutex& m;
};


// ***************************************************************************
// Core Classes
// ***************************************************************************


// Manages page IO to/in main memory
class BufferManager
{

public:

	// Creates a new instance that manages #size frames and operates on the
	// file 'filename'
	FRIEND_TEST(BufferManagerTest, constructor);
	BufferManager(const std::string& filename, uint64_t size, 
				  int numPages=constants::defaultNumPages);
	
	// A method to retrieve frames given a page ID and indicating whether the
	// page will be held exclusively by this thread or not. The method can fail
	// if no free frame is available and no used frame can be freed.
	FRIEND_TEST(BufferManagerTest, fixPageNoReplaceAndDestructor);
	FRIEND_TEST(BufferManagerTest, fixUnfixPageWithReplace);
	BufferFrame& fixPage(uint64_t pageId, bool exclusive);
	
	// Return a frame to the buffer manager indicating whether it is dirty or
	// not. If dirty, the page manager must write it back to disk. It does not
	// have to write it back immediately, but must not write it back before
	// unfixPage is called.
	void unfixPage(BufferFrame& frame, bool isDirty);

	
	// Destructor. Write all dirty frames to disk and free all resources.
	~BufferManager();
                  
private:
	
	// Reads page with pageID into frame, updates hash table. The page becomes
	// fixed in the frame, and is not dirty.
	FRIEND_TEST(BufferManagerTest, readPageIntoFrame);
    void readPageIntoFrame(uint64_t pageId, BufferFrame* frame );
    
    // Writes the page #frame back to disk. Assumes frame.pageId * pageSize
    // is a valid offset inside the file (i.e. is an offset that is followed
    // by at least one page)
    FRIEND_TEST(BufferManagerTest, flushFrameToFile);
    void flushFrameToFile(BufferFrame& frame);
    
   	// If no file with name = filename exists, create a file
	// with #numPages initial pages. Returns file descriptor to database.
	// No check forgoes all consistency checks, assumes file exists and uses
	// it as is. This is useful for instance if data other than 8 byte integers
	// are stored on file (see test cases)
	int initializeDatabase(const char* filename, const int numPages);
    
    // Manage page replacements
	FrameReplacer* replacer;
	
	// Hash proxy, supporting queries for pages given their id
	BufferHasher* hasher;
    
    // The pool of buffer frames, is instantiated and filled
	// on construction of the BufferManager object
	std::vector<BufferFrame*> framePool;

	// The number of frames to be managed
	uint64_t numFrames;
	
	// Handler to file with pages on disk. At all times, the file is assumed 
	// to contain a multiple of constants::pageSize bytes. The pages
	// are assumed to be numered 0 ... n-1.
	int fileDescriptor;

	// TODO	
	//pthread_rwlock_t lock;
	 std::mutex lock;
	// std::mutex unfixlock;
		
};

#endif  // BUFFERMANAGER_H

