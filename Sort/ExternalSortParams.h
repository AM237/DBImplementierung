
///////////////////////////////////////////////////////////////////////////////
// ExternalSortParams.h
///////////////////////////////////////////////////////////////////////////////

#ifndef EXTERNALSORTPARAMS_H
#define EXTERNALSORTPARAMS_H

struct ExternalSortParams
{
	// Path to where directory containing the runs should be stored
	static const char* runDirPath;
	
	// Name of runs directory
	static const char* runDirName;
	
	// Runs naming scheme
	static const char* runName;
		
	ExternalSortParams()
	{
		runDirPath = "./";
		runDirName = "runs";
		runName = "runNr.";
	}	
};


#endif // EXTERNALSORTPARAMS_H
