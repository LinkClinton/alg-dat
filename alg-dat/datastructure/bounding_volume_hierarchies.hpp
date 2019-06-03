#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

namespace alg_dat {

	using real = float;

	enum class bvh_build_mode {
		middle, equal_counts, surface_area_heuristic
	};

	template<typename BoundingBox, typename Element>
	struct bvh_node {
		BoundingBox box;

		bvh_node* left = nullptr;
		bvh_node* right = nullptr;

		int axis = 0;
		int offset = 0;
		int count = 0;

		bool is_leaf() const { return (left == nullptr && right == nullptr); }

		static bvh_node leaf(const BoundingBox &box, size_t offset, size_t count);

		static bvh_node node(int axis, bvh_node* left, bvh_node* right);
	};

	template<typename BoundingBox, typename Element>
	class bvh_allocator {
	public:
		using node = bvh_node<BoundingBox, Element>;

		bvh_allocator(size_t max_count);

		auto allocate()->node*;

		void free();
	private:
		node* pool;

		size_t count;
		size_t max_count;
	};

	template<typename BoundingBox, typename Element>
	struct bvh_element_info {
		BoundingBox bound;
		Element* element;

		bvh_element_info() : element(nullptr) {}
	};

	template<typename BoundingBox>
	struct bvh_bounding_helper {
		static BoundingBox merge(
			const BoundingBox &b0, 
			const BoundingBox &b1) {
			return BoundingBox(b0, b1);
		}

		static void apply(BoundingBox &bound, const BoundingBox &other) {
			bound.apply(other);
		}

		template<typename Point>
		static void apply(BoundingBox &bound, const Point &point) {
			bound.apply(point);
		}

		static auto max_dimension(const BoundingBox &bound) -> int {
			return 0;
		}

		static auto max_property(const BoundingBox& bound, int dim) {
			return 0;
		}

		static auto min_property(const BoundingBox& bound, int dim) {
			return 0;
		}

		static auto centroid(const BoundingBox& bound) {
			return 0;
		}

		static auto centroid(const BoundingBox& bound, int dim) {
			return 0;
		}

		static auto surface_area(const BoundingBox& bound) {
			return 0;
		}
	};

	template<typename BoundingBox, typename Element, typename BoundingBoxHelper>
	class bvh_accelerator {
	public:
		bvh_accelerator(
			const std::vector<BoundingBox> &bounds,
			const std::vector<Element*> &elements,
			bvh_build_mode mode = bvh_build_mode::surface_area_heuristic);

		inline static size_t max_elements_per_node = 255;
	private:
		using node = bvh_node<BoundingBox, Element>;
		using element_info = bvh_element_info<BoundingBox, Element>;

		struct bucket_info {
			int count;
			BoundingBox bounds;

			bucket_info() : count(0) {}
		};

		bvh_allocator<BoundingBox, Element> allocator;

		node* root;
		bvh_build_mode mode;

		std::vector<Element*> elements;

		auto recursive_build(
			std::vector<element_info> &infos,
			size_t start, size_t end,
			std::vector<Element*> new_order)->node*;

		auto split(std::vector<element_info> &infos,
			const BoundingBox &centroid_bound, int dim,
			size_t start, size_t end)->size_t;

		auto split_middle(std::vector<element_info> &infos,
			const BoundingBox &centroid_bound, int dim,
			size_t start, size_t end)->size_t;

		auto split_equal_counts(std::vector<element_info> &infos,
			const BoundingBox &centroid_bound, int dim,
			size_t start, size_t end)->size_t;

		auto split_surface_area_heuristic(std::vector<element_info> &infos,
			const BoundingBox &centroid_bound, int dim,
			size_t start, size_t end)->size_t;

		auto cost_surface_area_heuristic(const std::vector<bucket_info> &infos,
			const BoundingBox &centroid_bound, size_t location)->real;
	};

	template <typename BoundingBox, typename Element>
	bvh_node<BoundingBox, Element> bvh_node<BoundingBox, Element>::leaf(const BoundingBox& box, size_t offset, size_t count) {
		return { box, nullptr, nullptr, 0, 
			static_cast<int>(offset), 
			static_cast<int>(count) };
	}

	template <typename BoundingBox, typename Element>
	bvh_node<BoundingBox, Element> bvh_node<BoundingBox, Element>::node(int axis, bvh_node* left, bvh_node* right) {
		return {
			BoundingBox(left->box, right->box),
			left, right, axis, 0,
			left->count + right->count };
	}

	template <typename BoundingBox, typename Element>
	bvh_allocator<BoundingBox, Element>::bvh_allocator(size_t max_count) :
		pool(new node[max_count]), count(0), max_count(max_count) { }

	template <typename BoundingBox, typename Element>
	auto bvh_allocator<BoundingBox, Element>::allocate() -> node* {
		assert(count < max_count);

		return pool + (count++);
	}

	template <typename BoundingBox, typename Element>
	void bvh_allocator<BoundingBox, Element>::free() {
		delete pool; pool = nullptr;
	}

	template<typename BoundingBox, typename Element, typename BoundingBoxHelper>
	bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::bvh_accelerator(
		const std::vector<BoundingBox>& bounds,
		const std::vector<Element*>& elements, bvh_build_mode mode) : 
			allocator(std::min(bounds.size(), elements.size()) * 2), mode(mode){
		std::vector<element_info> infos(std::min(bounds.size(), elements.size()));

		for (size_t i = 0; i < infos.size(); i++)
			infos[i].bound = bounds[i], infos[i].element = elements[i];

		root = recursive_build(infos, 0, infos.size(), this->elements);
	}

	template<typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::recursive_build(std::vector<element_info> &infos,
		size_t start, size_t end, std::vector<Element*> new_order) -> node* {
		auto node = allocator.allocate();

		auto bounds = BoundingBox();
		for (auto i = start; i < end; i++) BoundingBoxHelper::apply(bounds, infos[i].bound);

		if (end - start == 1) {
			(*node) = node::leaf(bounds, new_order.size(), 1);

			new_order.push_back(infos[start].element);
		}
		else {
			auto centroid_bound = BoundingBox();
			for (auto i = start; i < end; i++) BoundingBoxHelper::apply(centroid_bound, BoundingBoxHelper::centroid(infos[i].bound));

			auto dim = BoundingBoxHelper::max_dimension(centroid_bound);
			
			//same centroid, so push them into a leaf node
			if (BoundingBoxHelper::max_property(centroid_bound, dim) == BoundingBoxHelper::min_property(centroid_bound, dim)) {
				(*node) = node::leaf(bounds, new_order.size(), end - start);

				for (auto i = start; i < end; i++) new_order.push_back(infos[i].element);
			}
			else {
				auto mid = split(infos, centroid_bound, dim, start, end);

				//leaf node
				if (mid == start || mid == end) {
					(*node) = node::leaf(bounds, new_order.size(), end - start);

					for (auto i = start; i < end; i++) new_order.push_back(infos[i].element);
				}
				else (*node) = node::node(dim,
					recursive_build(infos, start, mid, new_order),
					recursive_build(infos, mid, end, new_order));
			}
		}

		return node;
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::split(std::vector<element_info> &infos,
		const BoundingBox &centroid_bound, int dim,
		size_t start, size_t end) -> size_t {
		switch (mode) {
		case bvh_build_mode::middle: return split_middle(infos, centroid_bound, dim, start, end);
		case bvh_build_mode::equal_counts: return split_equal_counts(infos, centroid_bound, dim, start, end);
		case bvh_build_mode::surface_area_heuristic: return split_surface_area_heuristic(infos, centroid_bound, dim, start, end);
		default: throw std::exception("error : invalid build mode.");
		}
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::split_middle(std::vector<element_info>& infos,
		const BoundingBox& centroid_bound, int dim,
		size_t start, size_t end) -> size_t {
		auto mid_position = (
			BoundingBoxHelper::min_property(centroid_bound, dim) + 
			BoundingBoxHelper::max_property(centroid_bound, dim)) / 2;

		auto mid_ptr = std::partition(
			&infos[start], &infos[end - 1] + 1,
			[dim, mid_position](const element_info &info) {
			return BoundingBoxHelper::centroid(info.bound, dim) < mid_position;
		});

		auto mid = static_cast<size_t>(mid_ptr - &infos[0]);

		if (mid == start || mid == end) return split_equal_counts(infos, centroid_bound, dim, start, end);

		return mid;
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::split_equal_counts(std::vector<element_info>& infos,
		const BoundingBox& centroid_bound, int dim, size_t start, size_t end) -> size_t {
		auto mid = (start + end) >> 1;

		std::nth_element(&infos[start], &infos[mid], &infos[end - 1] + 1,  
			[dim](const element_info &left, const element_info& right) {
			return BoundingBoxHelper::centroid(left.bound, dim) < BoundingBoxHelper::centroid(right.bound, dim);
		});

		return mid;
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::split_surface_area_heuristic(std::vector<element_info>& infos,
		const BoundingBox& centroid_bound, int dim, size_t start, size_t end) -> size_t {
		if (start - end <= 4) return split_equal_counts(infos, centroid_bound, dim, start, end);

		constexpr size_t buckets_count = 12;

		std::vector<bucket_info> buckets(buckets_count);

		for (auto i = start; i < end; i++) {
			auto location = static_cast<size_t>(
				buckets_count * (BoundingBoxHelper::centroid(infos[i].bound, dim) / BoundingBoxHelper::centroid(centroid_bound, dim)));
			
			if (location == buckets_count) location = buckets_count - 1;

			buckets[location].count = buckets[location].count + 1;

			BoundingBoxHelper::apply(buckets[location].bounds, infos[i].bound);
		}

		auto min_cost = cost_surface_area_heuristic(buckets, centroid_bound, 0);
		auto min_cost_location = static_cast<size_t>(0);

		for (size_t i = 1; i < buckets_count - 1; i++) {
			auto cost = cost_surface_area_heuristic(buckets, centroid_bound, i);

			if (min_cost > cost) min_cost = cost, min_cost_location = i;
		}

		auto leaf_cost = end - start;

		if (min_cost < leaf_cost || max_elements_per_node < leaf_cost) {
			auto mid_ptr = std::partition(&infos[start], &infos[end - 1] + 1,
				[=](const element_info &info) {
				auto location = static_cast<size_t>(
					buckets_count * (BoundingBoxHelper::centroid(info.bound, dim) / BoundingBoxHelper::centroid(centroid_bound, dim)));

				if (location == buckets_count) location = buckets_count - 1;

				return location <= min_cost_location;
			});

			return mid_ptr - &infos[0];
		}
		
		return start;
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::cost_surface_area_heuristic(const std::vector<bucket_info>& infos,
		const BoundingBox &centroid_bound, size_t location) -> real {
		assert(location >= 0 && location < infos.size());

		constexpr auto travel_cost = 0.125f;
		constexpr auto test_cost = 1;

		bucket_info info0;
		bucket_info info1;
		
		for (size_t i = 0; i <= location; i++) {
			BoundingBoxHelper::apply(info0.bounds, infos[i].bounds);
			
			info0.count = info0.count + infos[i].count;
		}

		for (size_t i = location + 1; i < infos.size(); i++) {
			BoundingBoxHelper::apply(info1.bounds, infos[i].bounds);

			info1.count = info1.count + infos[i].count;
		}

		return travel_cost + (
			info0.count * BoundingBoxHelper::surface_area(info0.bounds) +
			info1.count * BoundingBoxHelper::surface_area(info1.bounds)) / BoundingBoxHelper::surface_area(centroid_bound) * test_cost;
	}

}
