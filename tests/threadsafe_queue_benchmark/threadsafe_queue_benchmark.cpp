// filepath: /home/quantdev/repo/interview_data_structures/tests/threadsafe_queue_benchmark/threadsafe_queue_benchmark.cpp
#include "threadsafe_queue.h"
#include <benchmark/benchmark.h>
#include <thread>
#include <vector>

void producer(dev::threadsafe_queue<int>& queue, int num_items) {
    for (int i = 0; i < num_items; ++i) {
        queue.push(i);
    }
}

void consumer(dev::threadsafe_queue<int>& queue, int num_items) {
    for (int i = 0; i < num_items; ++i) {
        queue.pop();
    }
}

static void bench_create(benchmark::State& state){
    for(auto _ : state){
        dev::threadsafe_queue<int> queue;
    }
}

BENCHMARK(bench_create)->Arg(1);

static void bench_push(benchmark::State& state) {
    for (auto _ : state) {
        dev::threadsafe_queue<int> queue;
        for (int i = 0; i < state.range(0); ++i) {
            queue.push(i);
        }
    }
}
BENCHMARK(bench_push)->Arg(1000)->Arg(10000)->Arg(100000);

static void bench_pop(benchmark::State& state) {
    for (auto _ : state) {
        dev::threadsafe_queue<int> queue;
        for (int i = 0; i < state.range(0); ++i) {
            queue.push(i);
        }
        for (int i = 0; i < state.range(0); ++i) {
            queue.pop();
        }
    }
}
BENCHMARK(bench_pop)->Arg(1000)->Arg(10000)->Arg(100000);

static void bench_producer_consumer(benchmark::State& state) {
    for (auto _ : state) {
        dev::threadsafe_queue<int> queue;
        int num_items = state.range(0);
        std::thread producer_thread(producer, std::ref(queue), num_items);
        std::thread consumer_thread(consumer, std::ref(queue), num_items);
        producer_thread.join();
        consumer_thread.join();
    }
}
BENCHMARK(bench_producer_consumer)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();