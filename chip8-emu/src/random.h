#pragma once

// Returns random numbers using std::rand(). Provided to allow injecting tests.
class Random {
 public:
  Random();
  explicit Random(unsigned int seed);
  virtual ~Random() = default;

  virtual int rand();
};