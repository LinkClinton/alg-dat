#pragma once

/*
 * radix_sort.hpp
 * A very fast sort function to sort any elements with unsigned type key.
 * As we know, the radix sort's time complexity is O(n) with large memory complexity.
 * To support any unsigned key, the range of key was divide into some blocks, and sort independent.
 * The last time complexity is O(n * number of blocks) and the memory complexity is O(n + 2^(bits of KeyType / number of blocks))
 * For example, KeyType is unsigned int and number of blocks is 4. 
 * The time complexity is O(n * 4) and memory complexity is O(n + 2 ^ 8).
 * 
 * radix_sort_function(): a function to get the key by element. See more in "default_radix_sort_function".
 */

#include <type_traits>
#include <memory>

namespace alg_dat {

	template<typename T>
	using allow_unsigned_type = std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value>;

	template<typename KeyType, typename T, typename = allow_unsigned_type<KeyType>>
	using radix_sort_function = KeyType(const T &);

	/**
	 * \brief default radix sort function
	 * \tparam KeyType Key Type
	 * \tparam T Element Type
	 * \param element element
	 * \return the key of element
	 */
	template<typename KeyType, typename T, typename = allow_unsigned_type<KeyType>>
	auto default_radix_sort_function(const T &element) -> KeyType {
		return element;
	}
	

	template<typename KeyType, typename T, typename = allow_unsigned_type<KeyType>>
	void radix_sort(T* begin, T* end, radix_sort_function<KeyType, T> function = default_radix_sort_function<KeyType, T>) {
		constexpr auto bits_length = sizeof(KeyType) << 3;
		constexpr auto group_length = static_cast<size_t>(1) << 3;
		constexpr auto group_pass = bits_length / group_length;
		constexpr auto counter_size = static_cast<size_t>(1 << group_length);

		const auto size = static_cast<size_t>(end - begin);

		auto indices = static_cast<KeyType*>(std::malloc(sizeof(KeyType) * size));
		auto pool = static_cast<T*>(std::malloc(sizeof(T) * size));
		
		auto in = begin;
		auto out = pool;

		auto low_bit = static_cast<size_t>(0);
		auto mask = static_cast<size_t>((1 << group_length) - 1);
		
		for (size_t i = 0; i < group_pass; i++) {
			size_t element_counter[counter_size] = { 0 };
			size_t element_sum[counter_size] = { 0 };

			for (size_t element = 0; element < size; element++) {
				auto key = function(in[element]);
				auto index = static_cast<KeyType>((key & mask) >> low_bit);

				indices[element] = index;
				++element_counter[index];
			}

			for (size_t index = 1; index < counter_size; index++)
				element_sum[index] = element_sum[index - 1] + element_counter[index - 1];

			for (size_t element = 0; element < size; element++) 
				out[element_sum[indices[element]]++] = in[element];

			std::swap(in, out);

			low_bit = low_bit + group_length;
			mask = mask << group_length;
		}

		if (out == pool) std::memcpy(begin, pool, sizeof(T) * size);

		std::free(indices);
		std::free(pool);
	}

	template<typename T, typename = allow_unsigned_type<T>>
	void radix_sort(T* begin, T* end) {
		radix_sort<T, T>(begin, end, default_radix_sort_function<T, T>);
	}
}
