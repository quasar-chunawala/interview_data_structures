#include <gtest/gtest.h>
#include <thread>
#include <atomic>
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

TEST(SharedPtrTest, RefCountingTest){
    int* raw_ptr = new int(42);
    {
        dev::shared_ptr ptr1 = raw_ptr;
        EXPECT_EQ(ptr1.use_count() == 1, true);
        {
            dev::shared_ptr ptr2 = ptr1;
            EXPECT_EQ(ptr1.use_count() == 2, true);
            {
                dev::shared_ptr ptr3 = ptr2;
                EXPECT_EQ(ptr1.use_count() == 3, true);
            }
            EXPECT_EQ(ptr1.use_count() == 2, true);
        }
        EXPECT_EQ(ptr1.use_count() == 1, true);
    }
}

TEST(SharedPtrTest, MultithreadedConstructionAndDestructionTest){
    dev::shared_ptr ptr = new int(42);
    std::atomic<bool> go{false};
    EXPECT_EQ(ptr.use_count() == 1, true);

    std::thread t1([&]{
        dev::shared_ptr<int> ptr1 = ptr;
        while(!go);
        std::cout << "\nRef Count = " << ptr.use_count();
    });

    std::thread t2([&]{
        dev::shared_ptr<int> ptr2 = ptr;
        while(!go);
        std::cout << "\nRef Count = " << ptr.use_count();
    });

    go.store(true);
    t1.join();
    t2.join();
    EXPECT_EQ(ptr.use_count() == 1, true);
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
    dev::shared_ptr<int> p1(new int(42));
    dev::shared_ptr<int> p2(new int(28));
    p2 = std::move(p1);
    EXPECT_EQ(p2.get() != nullptr, true);
    EXPECT_EQ(*p2 == 42, true);
}

/* reset() :  replaces the managed object */
TEST(SharedPtrTest, ResetSharedPtr) {
    dev::shared_ptr<int> ptr(new int(10));
    ptr.reset(new int(20));
    EXPECT_EQ(ptr != nullptr, true);
    EXPECT_EQ(*ptr == 20, true);

    // Self-reset test
    ptr.reset(ptr.get());
}

/* swap() : swap the managed objects */
TEST(SharedPtrTest, SwapTest){
    int* first = new int(42);
    int* second = new int(17);

    dev::shared_ptr<int> p1(first);
    dev::shared_ptr<int> p2(second);

    swap(p1, p2);

    EXPECT_EQ(p2.get() == first && p1.get() == second, true);
    EXPECT_EQ(((*p1) == 17) && ((*p2) == 42), true);
}

// Observers
/* get() : Returns a pointer to the managed object or nullptr*/
TEST(SharedPtrTest, GetTest){
    double* resource = new double(0.50);
    dev::shared_ptr p(resource);

    EXPECT_EQ(p.get() == resource, true);
    EXPECT_EQ(*(p.get()) == 0.50, true);
}

// Pointer-like functions
TEST(SharedPtrTest, IndirectionOperatorTest) {
    /* indirection operator* to dereference pointer to managed object,
       member access operator -> to call member function*/
    struct X {
        int _n;

        X() = default;
        X(int n) : _n{n} {}
        ~X() = default;
        int foo() { return _n; }
    };

    dev::shared_ptr<X> ptr(new X(10));
    EXPECT_EQ((*ptr)._n == 10, true);
    EXPECT_EQ(ptr->foo() == 10, true);
}