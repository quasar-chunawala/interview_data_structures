#include <gtest/gtest.h>
#include "threadsafe_stack.h"
#include <thread>
#include <vector>
#include <algorithm>

TEST(ThreadSafeStackTest, PushAndTopTest) {
    dev::threadsafe_stack<int> stack;
    stack.push(42);
    EXPECT_EQ(stack.top(), 42);
}

TEST(ThreadSafeStackTest, PopTest) {
    dev::threadsafe_stack<int> stack;
    stack.push(42);
    auto popped = stack.pop();
    EXPECT_TRUE(popped.has_value());
    EXPECT_EQ(popped.value(), 42);
    EXPECT_TRUE(stack.empty());
}

TEST(ThreadSafeStackTest, EmptyTest) {
    dev::threadsafe_stack<int> stack;
    EXPECT_TRUE(stack.empty());
    stack.push(42);
    EXPECT_FALSE(stack.empty());
}

TEST(ThreadSafeStackTest, ConcurrentPushTest) {
    dev::threadsafe_stack<int> stack;
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&stack, i]() {
            stack.push(i);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_FALSE(stack.empty());
    EXPECT_TRUE(stack.size() == 10);
}

TEST(ThreadSafeStackTest, ConcurrentPopTest) {
    dev::threadsafe_stack<int> stack;
    for (int i = 0; i < 10; ++i) {
        stack.push(i);
    }

    std::vector<std::thread> threads;
    std::vector<int> results;
    std::mutex mtx;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&stack, &results, &mtx]() {
            auto popped = stack.pop();
            if (popped.has_value()) {
                std::unique_lock<std::mutex> unique_lck(mtx);
                results.push_back(popped.value());
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(results.size(), 10);
}

TEST(ThreadSafeStackTest, ConcurrentSwapTest){
    dev::threadsafe_stack<int> evens, odds;
    for(int i{0};i<5;++i)
        evens.push(2*i);
    
    for(int i{0};i<5;++i)
        odds.push(2*i + 1);  
        
    dev::threadsafe_stack<int> evensCopy {evens};
    dev::threadsafe_stack<int> oddsCopy {odds};            

    std::vector<std::thread> threads;

    for(int j{0};j<2;++j){
        threads.emplace_back([&evens, &odds](){
            evens.swap(odds);
        });
    }

    for(int j{0};j<2;++j)
        threads[j].join();
    
    EXPECT_EQ(evens == evensCopy, true);
    EXPECT_EQ(odds == oddsCopy, true);
}
