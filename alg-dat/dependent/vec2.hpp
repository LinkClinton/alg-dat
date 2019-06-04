#pragma once

#include <algorithm>
#include <cassert>

#include "../utility.hpp"

namespace alg_dat {
	
	template<typename T>
	struct vec2_t {
		T x;
		T y;

		vec2_t() : x(0), y(0) {}

		vec2_t(T x, T y) : x(x), y(y) {}

		vec2_t(T value) : x(value), y(value) {}

		T& operator [](size_t index) {
			assert(index < 2);

			if (index == 0) return x;

			return y;
		}

		vec2_t operator+(const vec2_t &vec) const {
			return vec2_t(
				x + vec.x,
				y + vec.y);
		}

		vec2_t operator-(const vec2_t &vec) const {
			return vec2_t(
				x - vec.x,
				y - vec.y);
		}

		vec2_t operator*(const T &value) const {
			return  vec2_t(
				x * value, y * value);
		}

		bool operator==(const vec2_t &vec) const {
			return x == vec.x && y == vec.y;
		}

		static vec2_t min(const vec2_t &v0, const vec2_t &v1);

		static vec2_t max(const vec2_t &v0, const vec2_t &v1);
	};

	template <typename T>
	vec2_t<T> vec2_t<T>::min(const vec2_t& v0, const vec2_t& v1) {
		return vec2_t(
			std::min(v0.x, v1.x),
			std::min(v0.y, v1.y));
	}

	template <typename T>
	vec2_t<T> vec2_t<T>::max(const vec2_t& v0, const vec2_t& v1) {
		return vec2_t(
			std::max(v0.x, v1.x),
			std::max(v0.y, v1.y));
	}

	using vec2 = vec2_t<real>;

}
