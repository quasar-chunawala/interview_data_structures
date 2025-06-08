#include "spsc_queue.h"
#include <gtest/gtest.h>

TEST(SPSCQueueTest, PushAndPop) {
    dev::spsc_queue<int,6> queue; // Create a queue with capacity 8

    // Push elements onto the queue
    for(int i{0}; i<7; ++i){
        queue.try_push( i+1 );
    }

    for(int i{0}; i<7; ++i){
        EXPECT_EQ( queue.try_pop(), i + 1 );
    }
}