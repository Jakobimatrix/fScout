#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <library/math.hpp>
#include <sstream>

// https://github.com/catchorg/Catch2/blob/devel/docs/Readme.md

TEST_CASE("Basic test case") {
  REQUIRE(1 + 1 == 2);  // Simple assertion
}

TEST_CASE("Section example") {
  SECTION("First section") { REQUIRE(1 == 1); }
  SECTION("Second section") { REQUIRE(2 == 2); }
}


TEST_CASE("Fibonacci") {
  CHECK(lib::fibonacci(0) == 1);
  CHECK(lib::fibonacci(5) == 8);

  BENCHMARK("Fibonacci 20") { return lib::fibonacci(20); };

  BENCHMARK("Fibonacci 35") { return lib::fibonacci(35); };

  BENCHMARK_ADVANCED("Run with pre initialized data")
  (Catch::Benchmark::Chronometer meter) {
    std::vector<int> data{1, 2, 3, 4};
    meter.measure([&data] {
      for (const int d : data) {
        lib::fibonacci(d);
      }
    });
  };
}
