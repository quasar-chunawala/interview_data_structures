#include <gtest/gtest.h>
#include "unique_ptr.h"

TEST(UniquePtrTest, CreateAndAccessTest)
{
    int* raw_ptr = new int(42);
    dev::unique_ptr<int> p(raw_ptr);
    
    EXPECT_EQ(*unique_ptr == 42,true);
    EXPECT_EQ(unique_ptr.get(), raw_ptr);
}

/* Move constructor - Transfer of ownership */
TEST(UniquePtrTest, MoveConstructorTest)
{
    dev::unique_ptr p {dev::unique_ptr(new int(17))};

    EXPECT_EQ(*unique_ptr, 17);
    EXPECT_EQ(unique_ptr!=nullptr, true);
}

/* Move assignment */
TEST(UniquePtrTest, MoveAssignmentTest)
{
    dev::unique_ptr<int> p1(new int(42));
    dev::unique_ptr<int> p2(new int(17));
    p1 = p2;

    EXPECT_EQ(p1!=nullptr, true);
    EXPECT_EQ(*p1 == 17, true);
    EXPECT_EQ(sizeof(p2) == 0, true);
}

// Modifiers
/* release() : Returns the pointer to resource and releases ownership*/
TEST(UniquePtrTest, ReleaseTest){
    dev::unique_ptr<double> ptr(new double(3.14));
    double* rawPtr = ptr.release();

    EXPECT_EQ(ptr == nullptr, true);
    EXPECT_EQ(rawPtr != nullptr, true);
    EXPECT_EQ(*rawPtr == 3.14, true);
}

/* reset() :  replaces the managed object */
TEST(UniquePtrTest, ResetUniquePtr) {
    dev::unique_ptr<int> ptr(new int(10));
    ptr.reset(new int(20));
    EXPECT_EQ(ptr != nullptr, true);
    EXPECT_EQ(*ptr == 20, true);

    // Self-reset test
    ptr.reset(ptr.get());
}

/* swap() : swap the managed objects */
TEST(UniquePtrTest, SwapTest){
    int* first = new int(42);
    int* second = new int(17);

    dev::unique_ptr<int> p1(first);
    dev::unique_ptr<int> p2(second);

    std::swap(p1, p2);

    EXPECT_EQ(p2 == first && p1 == second, true);
    EXPECT_EQ(*p1 == 17 && *p2 == 42, true);
}

// Observers
/* get() : Returns a pointer to the managed object or nullptr*/
TEST(UniquePtrTest, GetTest){
    double* resource = new double(0.50);
    dev::unique_ptr p(resource);

    EXPECT_EQ(p.get() == resource, true);
    EXPECT_EQ(*(p.get()) == 0.50, true);
}

/* operator bool() : Checks whether *this owns an object*/
TEST(UniquePtrTest, OperatorBoolTest){
    int* resource = new int(28);
    dev::unique_ptr p1;
    dev::unique_ptr p2(resource);

    EXPECT_EQ(p1, false);
    EXPECT_EQ(p2, true);
}

// Pointer-like functions
TEST(UniquePtrTest, IndirectionOperatorTest) {
    /* indirection operator* to dereference pointer to managed object,
       member access operator -> to call member function*/
    struct X {
        int n;
        int foo() { return n; }
    };

    dev::unique_ptr<X> ptr(new X(10));
    EXPECT_EQ((*ptr).n == 10, true);
    EXPECT_EQ(ptr->foo() == 10, true);
}
