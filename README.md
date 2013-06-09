DBImplementierung
=================

Code Repository for the "Databases Implementation on Modern CPU Architectures" Lecture, SS 2013, TUM


Project BTree

1. BTreeMain contains the external test provided in the lecture site.

2. 'make release' compiles source code and places executables (main and own unit tests) in BTree/bin.



Project SegmentManager

1. Current status: SI, FSI, and regular operations on the segment manager and segments (drop, create, grow, etc.) are implemented and tested. The segment manager support a multiple page span for the SI and FSI, this still requires testing however. Slotted pages are not yet implemented.

2. Design: the segment manager encompasses the buffer manager, that is, the buffer manager is instantiated internally within the segment manager.

3. Currently only three types of segments are implemented, the SI, the FSI, and regular segments.

4. 'make release' compiles source code and places executables (main and own unit tests) in SegmentManager/bin.



Project BufferManager

1. BufferManagerMain contains the external test provided in the lecture site.

2. 'make release' compiles source code and places executables (main and own unit tests) in BufferManager/bin.

3. A sample (binary) data file has been provided in /bin. The BufferManager assumes the
existence and correctness of this or any similar file, if such a file previously exists. Otherwise, the buffer manager can create an empty database.



Project: Sort

1. 'make release' compiles source code and places executables (main and test) in Sort/bin.

2. test case ('make test') creates sample 4MB input file and corresponding output, both of which are deleted once the test is finished. Similarly, we tested locally with a 5 GB input file and obtained similar results.

3. execute with no arguments for an overview of the available command line options (used for debugging, etc.).



Project tools and dependencies:

1. Testing framework: Google Test 1.6.0 - Assumes header and library files have been installed to /usr/include and /usr/lib, respectively.

2. Build framework - CMake (see Makefile for available make targets)

3. Compiler - g++ with added support for C++11.



Sandbox project links:

1. Installing git: http://www.thegeekstuff.com/2011/08/git-install-configure/

2. Installing Google Test: http://stackoverflow.com/questions/13513905/how-to-easily-setup-googletest-on-linux

3. Installing CMake: https://secure.mash-project.eu/wiki/index.php/CMake:_Quick_Start_Guide
