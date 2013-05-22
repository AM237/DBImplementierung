DBImplementierung
=================

Code Repository for the "Databases Implementation on Modern CPU Architectures" Lecture, SS 2013, TUM

Project BufferManager

1. BufferManagerMain contains the external test provided in the lecture site.

2. 'make release' compiles source code and places executables (main and own unit tests) in Sort/bin.

3. A sample (binary) data file has been provided in /bin. The BufferManager assumes the
existence and correctness of this or any similar file.



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
