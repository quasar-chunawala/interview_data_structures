#include <gtest/gtest.h>
#include "forward_list.h"

TEST(ForwardListTest, InitializerListTest)
{
    /* Contructor that takes T* */
    dev::forward_list<int> lst{1,2,3,4,5};
}