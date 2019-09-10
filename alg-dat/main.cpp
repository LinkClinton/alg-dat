#include "dependent/memory/allocator.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>

using namespace alg_dat;

struct T {
	T() {
		std::cout << "create" << std::endl;
	}

	~T() {
		std::cout << "destroy" << std::endl;
	}
};

int main() {
	stack_allocator<T> a;
	
	auto x = a.allocate(20);

	
	a.deallocate(20);
}