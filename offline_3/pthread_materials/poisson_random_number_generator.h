// #include <iostream>
#include <random>

// Function to generate a Poisson-distributed random number
int get_random_number() {
  std::random_device rd;
  std::mt19937 generator(rd());
  double lambda = 10000.234;
  std::poisson_distribution<int> poissonDist(lambda);
  return poissonDist(generator);
}

/*
  Prepared by: Nafis Tahmid (1905002), Date: 10 November 2024
*/