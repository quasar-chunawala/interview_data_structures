#include "vector.h"
#include <gtest/gtest.h>

struct AllocCounter
{
    int value;
    static inline uint default_ctor_count{ 0 };
    static inline uint copy_ctor_count{ 0 };
    static inline uint move_ctor_count{ 0 };
    static inline uint copy_assignment_count{ 0 };
    static inline uint dtor_count{ 0 };

    AllocCounter()
      : value{ 0 }
    {
        ++default_ctor_count;
    }
    AllocCounter(const AllocCounter& other)
      : value(other.value)
    {
        ++copy_ctor_count;
    }

    AllocCounter& operator=(const AllocCounter& other)
    {
        AllocCounter(other).swap(*this);
        ++copy_assignment_count;
        return *this;
    }

    void swap(AllocCounter& other) noexcept { std::swap(value, other.value); }

    AllocCounter(AllocCounter&& other) noexcept
      : value{ other.value }
    {
        ++move_ctor_count;
    }

    AllocCounter(int val)
      : value{ val }
    {
    }

    ~AllocCounter() { ++dtor_count; }

    static void reset()
    {
        default_ctor_count = 0;
        move_ctor_count = 0;
        copy_ctor_count = 0;
        dtor_count = 0;
    }

    bool operator==(const AllocCounter& other) const { return value == other.value; }
};

TEST(VectorTest, DefaultConstructorTest)
{
    dev::vector<int> v;

    EXPECT_EQ(v.empty(), true);
}

TEST(VectorTest, InitializerListTest)
{
    AllocCounter::reset();
    dev::vector<int> v{ 1, 2, 3, 4, 5 };

    EXPECT_EQ(!v.empty(), true);
    EXPECT_EQ(v.size() == 5, true);
    EXPECT_EQ(v.capacity() > 0, true);
    for (int i{ 0 }; i < v.size(); ++i) {
        EXPECT_EQ(v[i], i + 1);
    }

    dev::vector vec{ AllocCounter(), AllocCounter(), AllocCounter() };

    EXPECT_EQ(!vec.empty(), true);
    EXPECT_EQ(vec.size() == 3, true);
    EXPECT_EQ(AllocCounter::default_ctor_count == 3, true);
    EXPECT_EQ(AllocCounter::copy_ctor_count == 3, true);
}

TEST(VectorTest, ParameterizedConstructorTest)
{
    AllocCounter::reset();
    dev::vector v(10, 5.5);

    EXPECT_EQ(v.size() == 10, true);
    EXPECT_EQ(v[0] == 5.5, true);

    AllocCounter::reset();
    AllocCounter allocCounter;
    dev::vector vec(10, allocCounter);
    EXPECT_EQ(AllocCounter::default_ctor_count, 1);
    EXPECT_EQ(AllocCounter::copy_ctor_count, 10);
}

TEST(VectorTest, CopyConstructorTest)
{
    dev::vector v1{ 1.0, 2.0, 3.0, 4.0, 5.0 };
    dev::vector v2(v1);

    EXPECT_EQ(v1.size() == v2.size(), true);

    for (int i{ 0 }; i < v1.size(); ++i)
        EXPECT_EQ(v1[i] == v2[i], true);
}

TEST(VectorTest, MoveConstructorTest)
{
    AllocCounter::reset();
    dev::vector<int> v1{ 1, 2, 3 };
    dev::vector<int> v2(std::move(v1));

    EXPECT_EQ(v1.size(), 0);
    EXPECT_EQ(v1.capacity(), 0);
    EXPECT_EQ(v2.size(), 3);
    EXPECT_EQ(v2[0], 1);

    dev::vector vec1(10, AllocCounter());
    dev::vector vec2(std::move(vec1));
    EXPECT_EQ(AllocCounter::default_ctor_count, 1);
    EXPECT_EQ(AllocCounter::copy_ctor_count,
              10); // We just re-wire the internal
                   // pointers, so the copy ctor is invoked only 10 times.
}

TEST(VectorTest, CopyAssignmentTest)
{
    dev::vector<int> v1{ 1, 2, 3 };
    dev::vector<int> v2;
    v2 = v1;

    EXPECT_EQ(v1.size(), v2.size());
    for (int i = 0; i < v1.size(); ++i) {
        EXPECT_EQ(v1[i], v2[i]);
    }
}

TEST(VectorTest, MoveAssignmentTest)
{
    dev::vector<int> v1{ 1, 2, 3 };
    dev::vector<int> v2;
    v2 = std::move(v1);

    EXPECT_EQ(v1.size(), 0);
    EXPECT_EQ(v1.capacity(), 0);
    EXPECT_EQ(v2.size(), 3);
    EXPECT_EQ(v2[0], 1);
}

TEST(VectorTest, AtTest)
{
    dev::vector<int> v{ 1, 2, 3 };
    EXPECT_EQ(v.at(0), 1);
    EXPECT_EQ(v.at(1), 2);
    EXPECT_EQ(v.at(2), 3);

    EXPECT_THROW(v.at(3), std::out_of_range);
}

TEST(VectorTest, SubscriptOperatorTest)
{
    dev::vector<int> v{ 1, 2, 3 };
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(VectorTest, FrontAndBackTest)
{
    dev::vector<int> v{ 1, 2, 3 };
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

    v.push_back(v.back());
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[1], 42);
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
    dev::vector<int> v{ 1, 2, 3 };
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

    // The design of push_back/insert is slightly hard to get right.
    // If the vector is full and you reallocate(grow) the vector right
    // in the beginning, then value in vec.push_back(value) becomes
    // a dangling reference, if it refers to the old storage (an element of the vector
    // itself e.g. vec.back()). This test is meant for such an edge case.
    dev::vector<int> vec{ 1 };
    for (int i = 0; i < 10; ++i) {
        vec.push_back(vec.back());
        EXPECT_EQ(vec.back(), 1);
    }
}

TEST(VectorTest, PopBackTest)
{
    dev::vector<int> v{ 1, 2, 3 };
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
        Point(int a, int b)
          : x(a)
          , y(b)
        {
        }
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
    dev::vector<int> v{ 1, 2, 4 };
    auto pos = v.insert(v.begin() + 2, 200); // overload (1)

    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 200);
    EXPECT_EQ(v[3], 4);

    dev::vector vec{ 1 };
    for (int i{ 0 }; i < 50; ++i) {
        vec.insert(vec.begin(), vec.back());
        EXPECT_EQ(vec.size(), i + 2);
        EXPECT_EQ(vec.back(), 1);
    }
}

TEST(VectorTest, EraseTest)
{
    dev::vector<int> v{ 1, 2, 3, 4 };
    v.erase(v.begin() + 1);

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 3);
    EXPECT_EQ(v[2], 4);
}

TEST(VectorTest, InsertRangeTest)
{
    // Create a dev::vector and populate it with initial values
    dev::vector<int> v1{ 1, 2, 3, 7, 8 };

    // Create a standard vector to use as the source range
    std::vector<int> source{ 4, 5, 6 };

    // Insert the range [source.begin(), source.end()) into v at position v.begin() + 3
    auto pos = v1.insert(v1.begin() + 3, source.begin(), source.end());

    // Check that the returned iterator points to the first inserted element
    EXPECT_EQ(*pos, 4);

    // Check the size of the vector after insertion
    EXPECT_EQ(v1.size(), 8);

    // Check the contents of the vector after insertion
    for (int i{ 0 }; i < v1.size(); ++i) {
        EXPECT_EQ(v1[i], i + 1);
    }

    dev::vector<int> v2{ 17, 5, 28 };
    dev::vector<int> rng2{ 42, 3, 16, 4 };

    auto idx = v2.insert(v2.begin(), rng2.begin(), rng2.end());
    EXPECT_EQ(v2[0], 42);
    EXPECT_EQ(v2[1], 3);
    EXPECT_EQ(v2[2], 16);
    EXPECT_EQ(v2[3], 4);
    EXPECT_EQ(v2[4], 17);
    EXPECT_EQ(v2[5], 5);
    EXPECT_EQ(v2[6], 28);

    dev::vector v3{ 1, 3, 5, 7 };
    dev::vector rng3{ 4, 5, 6 };
    v3.insert(v3.begin(), rng3.begin(), rng3.end());
    EXPECT_EQ(v3[0], 4);
    EXPECT_EQ(v3[1], 5);
    EXPECT_EQ(v3[2], 6);
    EXPECT_EQ(v3[3], 1);
    EXPECT_EQ(v3[4], 3);

    EXPECT_EQ(rng3.size(), 3); // Check the size of the source range
}

TEST(VectorTest, InsertRangeClassTypeTest)
{
    static_assert(std::is_nothrow_move_constructible_v<AllocCounter>);
    // Reset counters
    AllocCounter::reset();

    // Create a dev::vector of AllocCounter
    dev::vector<AllocCounter> v1{
        AllocCounter(1), AllocCounter(2), AllocCounter(6), AllocCounter(7)
    };

    // Create a source range of TestObject
    std::vector<AllocCounter> source{ AllocCounter(3), AllocCounter(4), AllocCounter(5) };

    // Insert the range [source.begin(), source.end()) into v1 at position v1.begin() + 2
    auto pos = v1.insert(v1.begin() + 2, source.begin(), source.end());

    // Check that the returned iterator points to the first inserted element
    EXPECT_EQ(pos->value, 3);

    // Check the size of the vector after insertion
    EXPECT_EQ(v1.size(), 7);

    // Check the contents of the vector after insertion
    EXPECT_EQ(v1[0].value, 1);
    EXPECT_EQ(v1[1].value, 2);
    EXPECT_EQ(v1[2].value, 3);
    EXPECT_EQ(v1[3].value, 4);
    EXPECT_EQ(v1[4].value, 5);
    EXPECT_EQ(v1[5].value, 6);
    EXPECT_EQ(v1[6].value, 7);

    EXPECT_EQ(source.size(), 3);
    EXPECT_EQ(source[0].value, 3);
    EXPECT_EQ(source[1].value, 4);
    EXPECT_EQ(source[2].value, 5);

    GTEST_LOG_(INFO) << "\n"
                     << "AllocCounter::copy_ctor_count = "
                     << AllocCounter::copy_ctor_count;
    GTEST_LOG_(INFO) << "\n"
                     << "AllocCounter::default_ctor_count = "
                     << AllocCounter::default_ctor_count;
    GTEST_LOG_(INFO) << "\n"
                     << "AllocCounter::move_ctor_count = "
                     << AllocCounter::move_ctor_count;
    GTEST_LOG_(INFO) << "\n" << "AllocCounter::dtor_count = " << AllocCounter::dtor_count;
}

TEST(VectorTest, InsertInitializerListTest)
{
    // Create a vector and populate it with initial values
    dev::vector<int> v{ 1, 2, 6, 7 };

    // Insert an initializer list at position v.begin() + 2
    auto pos = v.insert(v.begin() + 2, { 3, 4, 5 });

    // Check that the returned iterator points to the first inserted element
    EXPECT_EQ(*pos, 3);

    // Check the size of the vector after insertion
    EXPECT_EQ(v.size(), 7);

    // Check the contents of the vector after insertion
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
    EXPECT_EQ(v[4], 5);
    EXPECT_EQ(v[5], 6);
    EXPECT_EQ(v[6], 7);
}

TEST(VectorTest, AssignTest)
{
    // Test assigning from a range of integers
    dev::vector<int> v;
    dev::vector<int> source{ 1, 2, 3, 4, 5 };

    v.assign(source.begin(), source.end());

    // Check the size and contents of the vector
    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v.capacity() >= 5, true);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
    EXPECT_EQ(v[4], 5);

    // Test assigning a smaller range
    dev::vector<int> smaller_source{ 10, 20 };
    v.assign(smaller_source.begin(), smaller_source.end());

    // Check the size and contents of the vector
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);

    // Test assigning a larger range
    dev::vector<int> larger_source{ 100, 200, 300, 400, 500, 600 };
    v.assign(larger_source.begin(), larger_source.end());

    // Check the size and contents of the vector
    EXPECT_EQ(v.size(), 6);
    EXPECT_EQ(v[0], 100);
    EXPECT_EQ(v[1], 200);
    EXPECT_EQ(v[2], 300);
    EXPECT_EQ(v[3], 400);
    EXPECT_EQ(v[4], 500);
    EXPECT_EQ(v[5], 600);

    // Test assigning an empty range
    dev::vector<int> empty_source;
    v.assign(empty_source.begin(), empty_source.end());

    // Check the size of the vector
    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(v.empty(), true);
}