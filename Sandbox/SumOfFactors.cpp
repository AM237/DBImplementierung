
/////////////////////////////////////////////////////////////////////////
// SumOfFactors.cpp
/////////////////////////////////////////////////////////////////////////


#include "SumOfFactors.h"

// ______________________________________________________________________
int Factorizer::sumOfFactors (int n)
{	
	int counter = 1;
	int sumOfFactors = 0;
	
	// Check for every number smaller than n if it is a factor of n,
	// and if so, add it to the sum
	while (counter < n)
	{
		int remainder = n / counter;	
		if (remainder * counter == n)
		{
			sumOfFactors += counter;
		}
		counter += 1;
	}
  
	return sumOfFactors;
}	


