#include <gtest/gtest.h>
#include "vector.h"

TEST(VectorTest, DefaultConstructorTest)
{
    dev::vector<int> v;

    EXPECT_EQ(v.empty(), true );
}

TEST(VectorTest, InitializerListTest)
{
    dev::vector<int> v{1,2,3,4,5};
    
    EXPECT_EQ(!v.empty(), true);
    EXPECT_EQ(v.size() == 5,true);
    EXPECT_EQ(v.capacity() > 0, true);
    EXPECT_EQ(v[2] == 3, true);
}

TEST(VectorTest, ParameterizedConstructorTest)
{
    dev::vector v(10, 5.5);

    EXPECT_EQ(v.size() == 10, true);
    EXPECT_EQ(v[0] == 5.5, true);
}


