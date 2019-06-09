#include "algorithm/radix_sort.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>

using namespace alg_dat;

typedef std::chrono::high_resolution_clock time_point;

const auto test_case = 200000; 

unsigned int origin_array[test_case];
unsigned int std_array[test_case];
unsigned int alg_array[test_case];

int main() {
	std::random_device rd;
	std::mt19937 random_engine(rd());//rd()

	const std::uniform_int_distribution<unsigned int> gen_size(0, 10000000);

	for (auto &item : origin_array) item = gen_size(random_engine);

	std::copy(origin_array, origin_array + test_case, std_array);
	std::copy(origin_array, origin_array + test_case, alg_array);

	const auto std_start_time = time_point::now();
	std::sort(std_array, std_array + test_case);
	const auto std_end_time = time_point::now();

	const auto alg_start_time = time_point::now();
	radix_sort(alg_array, alg_array + test_case);
	const auto alg_end_time = time_point::now();

	const auto std_time = std::chrono::duration_cast<std::chrono::duration<float>>(std_end_time - std_start_time).count();
	const auto alg_time = std::chrono::duration_cast<std::chrono::duration<float>>(alg_end_time - alg_start_time).count();

	for (size_t i = 0; i < test_case; i++) 
		if (std_array[i] != alg_array[i]) std::cout << "error at line: " << i << "." << std::endl;

	std::cout << test_case << ", " << std_time << ", " << alg_time << std::endl;

	system("pause");
}