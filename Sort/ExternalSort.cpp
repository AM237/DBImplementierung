
///////////////////////////////////////////////////////////////////////////////
// ExternalSort.cpp
///////////////////////////////////////////////////////////////////////////////


#include "ExternalSort.h"


struct OrderBySize
{
    bool operator() ( uint64_t const a, uint64_t const b) { return a > b; }
}; 
typedef std::priority_queue<uint64_t, std::vector<uint64_t>, OrderBySize> prioritysize_queue;



// ____________________________________________________________________________
void ExternalSort::externalSort(int fdInput, uint64_t size, int fdOutput, 
				  uint64_t memSize)
{
	externalSort(fdInput, size, fdOutput, memSize, false, false, false);
}



// ____________________________________________________________________________
void ExternalSort::externalSort(int fdInput, uint64_t size, int fdOutput, 
                  uint64_t memSize, bool readableRuns, bool verbose,
                  bool nocleanup)
{	
	// Partition file into sorted runs, store them to disk.
	time_t start, end;
	cout << endl << "Partitioning into sorted runs ... " << flush;
    time(&start);
	int runs = makeSortedRuns(fdInput, size, memSize/(1024*1024), 
				              readableRuns, verbose);
	time(&end);
	cout << "Finished partitioning (" << runs << " runs)."
	     << " Time required: " 
	     << difftime(end, start) << " sec" << endl << flush;
	if (verbose) cout << endl;

	// Merge the individual partitions, write result to
	// file with descriptor fdOutput (in binary format)
	cout << "Merging sorted runs ... " << flush;
	time(&start);
	mergeSortedRuns(memSize, fdOutput, runs);
	time(&end);
	cout << "Finished merging. Time required: " 
	     << difftime(end, start) << " sec" << endl << flush;
	     
	// Clean up
	if (!nocleanup)
	{
		cout << "Cleaning up ... ";
		if (system(("rm -r " + runDirPath + runDirName).c_str()) < 0) 
			cout << "Error deleting runs folder" << endl;
		cout << "done. " << endl << endl;
	}
	
}

// ____________________________________________________________________________
int ExternalSort::makeSortedRuns(int fdInput, uint64_t size, uint64_t memSize,
                    bool readableRuns, bool verbose)
{
	// For simplicity, buffer size is limited to 4 GB.
  	int requestedSize = memSize;
  	if (requestedSize < 0 || requestedSize > (4*1024))
  	{
  		cerr << "Invalid requested memory buffer size" << endl;
  		exit(1);
  	}
  		
  	// Compute memory requirements, as well as the max number of
  	// containable elements in the buffer
  	int bufferSize = requestedSize * 1024 * 1024;
  	if (bufferSize % sizeof(uint64_t) != 0)
  		{
  			cerr << "Cannot evenly fit data into buffer " << endl;	
  			exit(1);
  		}
  	int numElements = bufferSize / sizeof(uint64_t);
  
  	// Make directory to store sorted runs
  	if (system(("mkdir " + runDirPath + runDirName).c_str()) < 0) 
  		cout << "Error creating runs folder" << endl;
  		
  	// Read the input file blockwise into main memory
  	int readState = 0;
  	int runIndex = 1;
  	uint64_t processedElements = 0;
  	bool sizeReached = false;
  	
  	if (verbose) cout << endl;
  	do
  	{
  		if (verbose)
  		cout << endl << "Processing run number " << runIndex << endl;
  	  		  	
  	  	// Allocate buffer (make as large as possible to minimize
  		// the number of reads required)
		uint64_t* buffer = new uint64_t[numElements];

		readState =  read(fdInput, buffer, bufferSize);  		
  		
  		if (readState < 0) cerr << "Error reading file into buffer" << endl;
  		if (readState > 0) 
  			if (verbose) 
  				cout << "Read " << readState 
  		             << " bytes successfully" << endl;
  		             
  		if (readState == 0) 
  		{
  			if (verbose)
  				cout << "Reached end of file." << endl;
  			delete[] buffer;
  			break;
  		}
  		
  		// If EOF encountered, make sure only relevant elements are taken
  		// into account for sorting
  		int limit = numElements;
  		if (readState < bufferSize)
  			limit = readState / sizeof(uint64_t);
  		
  		// Sort the in memory block (run)
  		vector<uint64_t> buf;
  		for (int i = 0; i < limit; i++)
  		{
  			buf.push_back(buffer[i]);
  			
  			if (size > 0)
  			{
  				processedElements++;
  			
  				// Quit if enough elements have been processed
  				if (processedElements >= size)
  				{
  					sizeReached = true;
  					break;
  				}
  			}
  		}

  		delete[] buffer;  			
  		sort(buf.begin(), buf.end());
  			
  		// Write run to disk
  		const char* filename = 
  			(runDirPath+runDirName+"/"+runName+to_string(runIndex)).c_str();
  		
  		if (readableRuns)
  		{
  			ofstream runFile;
  			cout << "Opening: " << filename << endl;
	    	runFile.open (filename);
        
	    	for (unsigned i=0; i < buf.size(); i++)
	        	runFile << buf[i] <<"\n";
	      
        	runFile.close();
        	
        } else {
        
        	FILE* runFile;
        	runFile = fopen(filename, "wb");
        	if (runFile==NULL) cerr << "Unable to open run file number " 
        	                        << runIndex <<endl;
        
        	for (unsigned i=0; i<buf.size(); i++) {
				if (write(fileno(runFile), &(buf[i]), sizeof(uint64_t)) < 0)
				{
				  	if (verbose)
						cout << "Error writing to run file nr. " 
						     << runIndex<<endl;
					fclose(runFile);
					exit(1);
				}
			}

			fclose(runFile);
		}
        
   		if (verbose)
			cout << "Finished processing run number " << runIndex << endl;  		
		runIndex++;
  	
  	} while (readState != 0 && !sizeReached);
  	
  	if (verbose) cout << endl;
  	return runIndex-1;
} 



// ____________________________________________________________________________
void ExternalSort:: mergeSortedRuns(uint64_t memSize, int fdOutput, int runs)
{	

       
        // File descriptorss
	FILE* pFileRun[runs];
	FILE* oFile;

        // Priority queue
        prioritysize_queue pq; 

        // Size of the loaded run and the sorted area sizeof(uint64_t)
        uint64_t runMemSize = (memSize/(runs+1))/sizeof(uint64_t);
        
        // left memory size - -1 if run is empty
        uint64_t leftRunMemSize[runs]; 
	for (int i =0; i<runs; i++) leftRunMemSize[i]  = 0;

        // initial memory size
        uint64_t initialRunMemSize[runs]; 
	for (int i=0; i<runs; i++) initialRunMemSize[i]  = 0;

        // length of sorted list
	int sortedLength = 0;

        // Number of finished runs
        int countFinished = 0;

        // Read the input file blockwise into main memory
  	int readState = 0;

        // Not enough Memory to do a reasonable Merge
        if (runMemSize < 1) cerr << "Not enough Memory" << endl;

        //Buffer anlegen
        uint64_t* buffer = new uint64_t[memSize/sizeof(uint64_t)];
        uint64_t* actBuffer = buffer;
                                        
        // Open files
	for( int runIndex = 0; runIndex < runs; runIndex++){
                const char* filename = 
  			(runDirPath+runDirName+"/"+runName+to_string(runIndex+1)).c_str();
		pFileRun[runIndex] = fopen (filename,"rb");
                if (pFileRun[runIndex] ==NULL) cerr << "Unable to open temporary file" << endl;

	}

        // main loop to merge runs
        do{
                // Read blocks of data of emtpy runs
		for( int runIndex = 0; runIndex < runs; runIndex++){
            			if(leftRunMemSize[runIndex]==0 && leftRunMemSize[runIndex]!=-1){// Block empty and data to load in file
               				readState =  read(fileno(pFileRun[runIndex]), &(buffer[(runIndex+1)*runMemSize]),runMemSize*sizeof(uint64_t)); 
               				if (readState > 0) {

  						leftRunMemSize[runIndex]=readState/sizeof(uint64_t);
						initialRunMemSize[runIndex] = readState/sizeof(uint64_t);
                                                // push first element of run on priority queue
                			        pq.push(buffer[(runIndex+1)*runMemSize]);
                                              

  					} else if (readState == 0){ 
						leftRunMemSize[runIndex]=-1;
						countFinished++;

					}                                    
             			}      
        	}


               // all runs are processed
               if(countFinished == runs){
                  // write remaining sorted elements
                  write(fdOutput, &(buffer[0]), sortedLength*sizeof(uint64_t));
               } else
               {
               		// Get and remove top element
               		uint64_t topElement = pq.top();


                        pq.pop();


                        // write sorted elements in main memory
			buffer[sortedLength] = topElement;
                        sortedLength++;

                        // write block of sorted elements on disk
			if(sortedLength==runMemSize){
				write(fdOutput, &(buffer[0]), runMemSize*sizeof(uint64_t));
				sortedLength = 0;
			}


               		// Remove top element from buffer of run 
	    		for( int runIndex = 0; runIndex < runs; runIndex++){
            			if(leftRunMemSize[runIndex]!=-1){
					//top element is in this run, move read pointer
					if(topElement==buffer[(runIndex+1)*runMemSize+(initialRunMemSize[runIndex]-leftRunMemSize[runIndex])]){
               					leftRunMemSize[runIndex]=leftRunMemSize[runIndex]-1;

						if(leftRunMemSize[runIndex] > 0)
  							pq.push(buffer[(runIndex+1)*runMemSize+(initialRunMemSize[runIndex]-leftRunMemSize[runIndex])]);
						break;
					}
				}
        		}
		}


	}while( countFinished < runs);

       delete[] buffer;

}









                

