#include "datastructure/bounding_volume_hierarchies.hpp"

using namespace alg_dat;

struct bounding_box {
	float min;
	float max;

	bounding_box() : min(0), max(0) {}

	bounding_box(
		const bounding_box &b0,
		const bounding_box &b1) {
		min = std::min(b0.min, b1.min);
		max = std::max(b0.max, b1.max);
	}
};

struct bounding_box_helper {
	static bounding_box merge(
		const bounding_box &b0,
		const bounding_box &b1) {
		return bounding_box(b0, b1);
	}

	static void apply(bounding_box &bound, const bounding_box &other) {
		bound.min = std::min(bound.min, other.min);
		bound.max = std::max(bound.max, other.max);
	}

	static void apply(bounding_box &bound, const float &point) {
		bound.min = std::min(bound.min, point);
		bound.max = std::max(bound.max, point);
	}

	static auto max_dimension(const bounding_box &bound) -> int {
		return 0;
	}

	static auto max_property(const bounding_box& bound, int dim) {
		return bound.max;
	}

	static auto min_property(const bounding_box& bound, int dim) {
		return bound.min;
	}

	static auto centroid(const bounding_box& bound) -> float{
		return (bound.max + bound.min) / 2;
	}

	static auto centroid(const bounding_box& bound, int dim) {
		return (bound.max + bound.min) / 2;
	}

	static auto surface_area(const bounding_box& bound) {
		return bound.max - bound.min;
	}
};

int main() {
	bvh_accelerator<bounding_box, int, bounding_box_helper> a(
		{ bounding_box() },{nullptr}
	);

	
}