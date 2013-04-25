
///////////////////////////////////////////////////////////////////////////////
// SumOfFactorsMain.cpp
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include "SumOfFactors.h"


// Main function that computes the sum of the factors of the given natural number.
int main(int argc, char** argv)
{
	// If not called with exactly one argument, show usage info. 
	if (argc != 2)
	{
		printf("Usage: ./SumOfFactorsMain <integer>\n");
        exit(1);
	}

	int n = atoi(argv[1]);
		
	// Every natural number is a factor of 0
	if (n == 0)
		printf("The sum of the factors of 0 is infinity");

	// Number invalid
	if (n < 1)
	{
		printf("Number is invalid");
		exit(1);
	}  

	// Compute the sum of the factors.
	Factorizer fac;	 
	int result = fac.sumOfFactors(n);

	// Output the result.  
	printf("The sum of the factors of %d is %d\n", n, result);
}
