
///////////////////////////////////////////////////////////////////////////////
// BMConst.h
//////////////////////////////////////////////////////////////////////////////

#ifndef BMCONST_H
#define BMCONST_H

namespace BM_EXC
{

	struct ReplaceFailAllFramesFixed: public std::exception
	{
  		virtual const char* what() const throw()
  		{ return "BufferManager replace fail - all frames are fixed."; }
	};


	struct ReplaceFailFrameUnclean: public std::exception
	{
  		virtual const char* what() const throw()
  		{ return "Suggested frame for replacement is not clean."; }
	};

	struct ReplaceFailNoFrameSuggested: public std::exception
	{
  		virtual const char* what() const throw()
  		{ return "No frame was suggested for replacement"; }
	};

	struct IllegalPathException: public std::exception
	{
  		virtual const char* what() const throw()
  		{ return "Something went wrong - should not have reached this code"; }
	};
}

namespace BM_CONS
{
	// Page size should be a multiple of the size of a page in virtual memory
	// const int pageSize = sysconf(_SC_PAGE_SIZE);
	const int pageSize = 4096;
	
	// The default number of pages to write to file when initializing
	// the database
	const int defaultNumPages = 50;
}

#endif  // BMCONST_H
