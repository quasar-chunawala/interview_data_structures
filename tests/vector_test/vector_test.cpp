#include <gtest/gtest.h>
#include "vector.h"

TEST(VectorTest, ConstructionTest)
{
    dev::vector<int> v{1,2,3,4,5};
    
    EXPECT_EQ(!v.empty(), true);
    EXPECT_EQ(v.size() == 5,true);
    EXPECT_EQ(v.capacity() > 0, true);
}