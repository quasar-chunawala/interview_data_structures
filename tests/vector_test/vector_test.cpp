#include <gtest/gtest.h>
#include "vector.h"

struct AllocCounter{
    static inline uint default_ctor_count{0};
    static inline uint copy_ctor_count{0};
    static inline uint move_ctor_count{0};
    static inline uint dtor_count{0};

    AllocCounter(){
        ++default_ctor_count;
    }

    AllocCounter(const AllocCounter&){
        ++copy_ctor_count;
    }

    AllocCounter(AllocCounter&&){
        ++move_ctor_count;
    }

    ~AllocCounter(){
        ++dtor_count;
    }
};

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
    for(int i{0};i<v.size();++i){
        EXPECT_EQ(v[i],i+1);
    }
}

TEST(VectorTest, ParameterizedConstructorTest)
{
    dev::vector v(10, 5.5);

    EXPECT_EQ(v.size() == 10, true);
    EXPECT_EQ(v[0] == 5.5, true);

    AllocCounter allocCounter;
    dev::vector vec(10, allocCounter);
    EXPECT_EQ(AllocCounter::default_ctor_count,1);
    EXPECT_EQ(AllocCounter::copy_ctor_count,10);
}

TEST(VectorTest, CopyConstructorTest){
    dev::vector v1{1.0, 2.0, 3.0, 4.0, 5.0};
    dev::vector v2(v1);

    EXPECT_EQ(v1.size() == v2.size(),true);

    for(int i{0};i<v1.size();++i)
        EXPECT_EQ(v1[i] == v2[i], true);
}

TEST(VectorTest, MoveConstructorTest)
{
    dev::vector<int> v1{1, 2, 3};
    dev::vector<int> v2(std::move(v1));

    EXPECT_EQ(v1.size(), 0);
    EXPECT_EQ(v1.capacity(), 0);
    EXPECT_EQ(v2.size(), 3);
    EXPECT_EQ(v2[0], 1);
}

TEST(VectorTest, CopyAssignmentTest)
{
    dev::vector<int> v1{1, 2, 3};
    dev::vector<int> v2;
    v2 = v1;

    EXPECT_EQ(v1.size(), v2.size());
    for (int i = 0; i < v1.size(); ++i)
    {
        EXPECT_EQ(v1[i], v2[i]);
    }
}

TEST(VectorTest, MoveAssignmentTest)
{
    dev::vector<int> v1{1, 2, 3};
    dev::vector<int> v2;
    v2 = std::move(v1);

    EXPECT_EQ(v1.size(), 0);
    EXPECT_EQ(v1.capacity(), 0);
    EXPECT_EQ(v2.size(), 3);
    EXPECT_EQ(v2[0], 1);
}

TEST(VectorTest, AtTest)
{
    dev::vector<int> v{1, 2, 3};
    EXPECT_EQ(v.at(0), 1);
    EXPECT_EQ(v.at(1), 2);
    EXPECT_EQ(v.at(2), 3);

    EXPECT_THROW(v.at(3), std::out_of_range);
}

TEST(VectorTest, SubscriptOperatorTest)
{
    dev::vector<int> v{1, 2, 3};
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(VectorTest, FrontAndBackTest)
{
    dev::vector<int> v{1, 2, 3};
    EXPECT_EQ(v.front(), 1);
    EXPECT_EQ(v.back(), 3);
}

TEST(VectorTest, EmptyTest)
{
    dev::vector<int> v;
    EXPECT_EQ(v.empty(), true);

    v.push_back(42);
    EXPECT_EQ(v.empty(), false);
}

TEST(VectorTest, SizeAndCapacityTest)
{
    dev::vector<int> v;
    EXPECT_EQ(v.size(), 0);
    EXPECT_GE(v.capacity(), 0);

    v.push_back(42);
    EXPECT_EQ(v.size(), 1);
    EXPECT_GT(v.capacity(), 0);
}

TEST(VectorTest, ReserveTest)
{
    dev::vector<int> v;
    v.reserve(10);
    EXPECT_GE(v.capacity(), 10);
    EXPECT_EQ(v.size(), 0);
}

TEST(VectorTest, ResizeTest)
{
    dev::vector<int> v{1, 2, 3};
    v.resize(5);

    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v[3], 0);
    EXPECT_EQ(v[4], 0);

    v.resize(2);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST(VectorTest, PushBackTest)
{
    dev::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(VectorTest, PopBackTest)
{
    dev::vector<int> v{1, 2, 3};
    v.pop_back();

    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST(VectorTest, EmplaceBackTest)
{
    struct Point
    {
        int x, y;
        Point(int a, int b) : x(a), y(b) {}
    };

    dev::vector<Point> v;
    v.emplace_back(1, 2);
    v.emplace_back(3, 4);

    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0].x, 1);
    EXPECT_EQ(v[0].y, 2);
    EXPECT_EQ(v[1].x, 3);
    EXPECT_EQ(v[1].y, 4);
}


TEST(VectorTest, InsertTest)
{
    dev::vector<int> v{1, 2, 4};
    auto pos = v.insert(v.begin() + 2, 200); // overload (1)

    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 200);
    EXPECT_EQ(v[3], 4);
}


TEST(VectorTest, EraseTest)
{
    dev::vector<int> v{1, 2, 3, 4};
    v.erase(v.begin() + 1);

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 3);
    EXPECT_EQ(v[2], 4);
}