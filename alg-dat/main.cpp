#include "datastructure/bounding_volume_hierarchies.hpp"
#include "dependent/bound2d.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>

using namespace alg_dat;

typedef std::chrono::high_resolution_clock time_point;

class segment {
public:
	vec2 start;
	vec2 end;

	segment() : start(0), end(0) {}

	segment(const vec2 &start, const vec2 &end) :
		start(start), end(end) {}

	bool intersect(segment* segment) const;

	bool intersect(segment* segment, vec2& point) const;
};

inline bool segment::intersect(segment* segment) const {
	const auto& p0 = start;
	const auto& p1 = end;
	const auto& p2 = segment->start;
	const auto& p3 = segment->end;

	const auto s1 = p1 - p0;
	const auto s2 = p3 - p2;


	const auto s = (-s1.y * (p0.x - p2.x) + s1.x * (p0.y - p2.y)) / (-s2.x *
		s1.y + s1.x *
		s2.y);

	const auto t = (s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) / (-s2.x *
		s1.y + s1.x *
		s2.y);

	return s >= 0 && s <= 1 && t >= 0 && t <= 1;
}

inline bool segment::intersect(segment* segment, vec2& point) const {
	const auto& p0 = start;
	const auto& p1 = end;
	const auto& p2 = segment->start;
	const auto& p3 = segment->end;

	const auto s1 = p1 - p0;
	const auto s2 = p3 - p2;

	const auto s = (-s1.y * (p0.x - p2.x) + s1.x * (p0.y - p2.y)) / (-s2.x *
		s1.y + s1.x *
		s2.y);

	const auto t = (s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) / (-s2.x *
		s1.y + s1.x *
		s2.y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		point = p0 + s1 * t;

		return true;
	}

	return false;
}

int main() {
	std::random_device rd;
	std::mt19937 random_engine(rd());//rd()

	const std::uniform_real_distribution<real> gen_real(0, 1000000);
	const std::uniform_int_distribution<size_t> gen_size(0, 10000);

	const auto test_case = 10000; //gen_size(random_engine);

	std::vector<segment*> segments_ptr;
	std::vector<segment> segments;
	std::vector<bound2> volumes;

	for (size_t i = 0; i < test_case; i++) {
		auto segment = ::segment(
			vec2(gen_real(random_engine), gen_real(random_engine)),
			vec2(gen_real(random_engine), gen_real(random_engine)));

		segments.emplace_back(segment);

		volumes.emplace_back(
			bound2(
				vec2::min(segment.start, segment.end),
				vec2::max(segment.start, segment.end)));
	}

	size_t brute_count = 0;
	
	const auto brute_start_time = time_point::now();

	for (size_t i = 0; i < segments.size(); i++) {
		for (size_t j = i; j < segments.size(); j++) {
			if (segments[i].intersect(&segments[j])) brute_count++;
		}
	}

	const auto brute_end_time = time_point::now();

	for (size_t i = 0; i < segments.size(); i++)
		segments_ptr.emplace_back(&segments[i]);

	bvh_accelerator<bound2, segment> bvh(volumes, segments_ptr, bvh_build_mode::surface_area_heuristic);

	size_t bvh_count = 0;

	const auto bvh_start_time = time_point::now();

	auto ptr = bvh.elements();
	
	for (size_t i = 0; i < segments.size(); i++) {
		auto contacts = bvh.enumerate_contacts(volumes[i]);

		for (auto &contact : contacts) {
			const auto offset = std::get<0>(contact);
			const auto count = std::get<1>(contact);

			for (int j = offset; j < offset + count; j++)
				if (segments[i].intersect(ptr[j])) bvh_count++;
		}
	}

	const auto bvh_end_time = time_point::now();

	const auto brute_time = std::chrono::duration_cast<std::chrono::duration<float>>(brute_end_time - brute_start_time).count();
	const auto bvh_time = std::chrono::duration_cast<std::chrono::duration<float>>(bvh_end_time - bvh_start_time).count();

	std::cout << brute_count << " " << bvh_count / 2 << std::endl;
	std::cout << test_case << ", " << brute_time << ", " << bvh_time << std::endl;

	system("pause");
}