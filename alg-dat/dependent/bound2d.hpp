#pragma once

#include "vec2.hpp"

namespace alg_dat {
	
	template<typename T>
	class bound2_t {
	public:
		bound2_t() : min(FLT_MAX), max(FLT_MIN) {}

		bound2_t(
			const vec2_t<T> &min,
			const vec2_t<T> &max);

		bound2_t(
			const bound2_t &b0,
			const bound2_t &b1);

		void apply(const bound2_t &bound);

		void apply(const vec2_t<T> &point);

		auto max_dimension() const -> int;

		auto max_property() const -> vec2_t<T>;

		auto min_property() const -> vec2_t<T>;

		auto centroid() const -> vec2_t<T>;

		auto centroid(int dim) const -> T;

		auto surface_area() const -> T;

		bool intersect(const bound2_t &bound) const;
	private:
		vec2_t<T> min;
		vec2_t<T> max;
	};

	template <typename T>
	bound2_t<T>::bound2_t(const vec2_t<T>& min, const vec2_t<T>& max) :
		min(vec2_t<T>::min(min, max)), max(vec2_t<T>::max(min, max)) {

	}

	template <typename T>
	bound2_t<T>::bound2_t(const bound2_t& b0, const bound2_t& b1) {
		min = vec2_t<T>::min(b0.min, b1.min);
		max = vec2_t<T>::max(b0.max, b1.max);
	}

	template <typename T>
	void bound2_t<T>::apply(const bound2_t& bound) {
		min = vec2_t<T>::min(min, bound.min);
		max = vec2_t<T>::max(max, bound.max);
	}

	template <typename T>
	void bound2_t<T>::apply(const vec2_t<T>& point) {
		min = vec2_t<T>::min(min, point);
		max = vec2_t<T>::max(max, point);
	}

	template <typename T>
	auto bound2_t<T>::max_dimension() const -> int {
		if (max.x - min.x > max.y - min.y) return 0;
		return 1;
	}

	template <typename T>
	auto bound2_t<T>::max_property() const -> vec2_t<T> {
		return max;
	}

	template <typename T>
	auto bound2_t<T>::min_property() const -> vec2_t<T> {
		return min;
	}

	template <typename T>
	auto bound2_t<T>::centroid() const -> vec2_t<T> {
		return (min + max) * static_cast<T>(0.5);
	}

	template <typename T>
	auto bound2_t<T>::centroid(int dim) const -> T {
		return centroid()[dim];
	}

	template <typename T>
	auto bound2_t<T>::surface_area() const -> T {
		return (max.x - min.x) * (max.y - min.y);
	}

	template <typename T>
	bool bound2_t<T>::intersect(const bound2_t& bound) const {
		if (min.x > bound.max.x || max.x < bound.min.x) return false;
		if (min.y > bound.max.y || max.y < bound.min.y) return false;

		return true;
	}

	using bound2 = bound2_t<real>;
}
