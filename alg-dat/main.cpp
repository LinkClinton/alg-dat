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

auto fun() -> stack_allocator<T> {
	
	return stack_allocator<T>();
}

int main() {
	stack_allocator<T> x;


	auto a = std::move(std::vector<T>());
	
	std::cout << 2;
}