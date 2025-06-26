#include "Core/ThreadPool.hpp"
#include <atomic>
#include <catch2/catch_all.hpp>
#include <vector>

TEST_CASE("ThreadPool basic functionality", "[ThreadPool]") {
  Core::ThreadPool pool(4);

  SECTION("Submit single task") {
    auto future = pool.Submit([]() { return 42; });
    REQUIRE(future.get() == 42);
  }

  SECTION("Submit multiple tasks") {
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 10; ++i) {
      futures.push_back(pool.Submit([i]() { return i * i; }));
    }

    for (int i = 0; i < 10; ++i) {
      REQUIRE(futures[i].get() == i * i);
    }
  }

  SECTION("Parallel for each") {
    std::vector<int> data(100, 0);
    std::atomic<int> sum{0};

    pool.ParallelForEach(data, [&sum](int &value) {
      value = 1;
      sum.fetch_add(1);
    });

    REQUIRE(sum.load() == 100);
    REQUIRE(
        std::all_of(data.begin(), data.end(), [](int v) { return v == 1; }));
  }
}