#include <gtest/gtest.h>
#include "shared_ptr.h"

TEST(SharedPtrTest, ParametrizedCTorTest)
{
    /* Contructor that takes T* */
    int* raw_ptr = new int(42);
    dev::shared_ptr<int> p1(raw_ptr);
    
    EXPECT_EQ(*p1 == 42,true);
    EXPECT_EQ(p1.get(), raw_ptr);

    dev::shared_ptr<int> p2 = new int(17);
    EXPECT_EQ(*p2 == 17,true);
    EXPECT_EQ(p2.get() != nullptr, true);
}

TEST(SharedPtrTest, CopyConstructorTest){
    /* Copy constructor */
    int* raw_ptr = new int(42);
    dev::shared_ptr<int> p1(raw_ptr);

    dev::shared_ptr<int> p2 = p1;
    EXPECT_EQ(*p2 == 42, true);
    EXPECT_EQ(p2.get(), raw_ptr);
}

TEST(SharedPtrTest, MoveConstructorTest){
    /* Move constructor*/
    dev::shared_ptr<int> p1(new int(28));
    dev::shared_ptr<int> p2 = std::move(p1);
    dev::shared_ptr<int> p3 = std::move(p2);
    EXPECT_EQ(*p3 == 28, true);
}

TEST(SharedPtrTest, CopyAssignmentTest){
    /* Copy Assignment */
    dev::shared_ptr<double> p1(new double(2.71828));
    dev::shared_ptr<double> p2(new double(3.14159));

    EXPECT_EQ(*p2 == 3.14159, true);
    p2 = p1;
    EXPECT_EQ(p2.get() == p1.get(), true );
    EXPECT_EQ(*p2 == *p1, true);
}

TEST(SharedPtrTest, MoveAssignmentTest){
    /* Move Assignment */
    
}