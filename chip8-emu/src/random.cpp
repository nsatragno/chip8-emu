#include "random.h"

#include <cstdlib>
#include <ctime>

Random::Random() : Random(std::time(nullptr)) {}

Random::Random(unsigned int seed) {
  std::srand(seed);
}

int Random::rand() {
  return std::rand();
}
