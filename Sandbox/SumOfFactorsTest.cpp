
#include <gtest/gtest.h>
#include "SumOfFactors.h"

TEST(SumOfFactorsTest, sumOfFactors)
{
	Factorizer fac;
	ASSERT_EQ(7, fac.sumOfFactors(8));
  	ASSERT_EQ(22, fac.sumOfFactors(20));
	ASSERT_EQ(16, fac.sumOfFactors(12));
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
