#pragma once

/*
 * bounding_volume_hierarchies.hpp
 * A bvh data structure with three build mode.
 * middle : split with centroid bound's center.
 * equal_counts : split with same size.
 * surface_area_heuristic : find the min-cost of split method.
 * 
 * bvh_allocator : a simple allocator for bvh_accelerator.
 * bvh_accelerator : bvh data structure.
 * 
 * bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>
 * BoundingBox : bounding box used for bvh
 * Element : element in the bounding box.
 * BoundingBoxHelper : help function for bvh.
 * 
 * If you want to use bvh_accelerator, a BoundingBoxHelper(see example in line 78, it is the default helper) need be provided.
 * Some member or member function in BoundingBox we do not to use it, so user need to provide a BoundingBoxHelper and override its function.
 * If you use the default helper, the BoundingBox need to provide some member function.
 * "struct bvh_bounding_helper" is a good example for this.
 * 
 * Also, you can use "bound2d" in "bound2d.hpp" with default helper function.
 */

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

namespace alg_dat {

	using real = float;

	/**
	 * \brief bounding volume hierarchies build mode
	 */
	enum class bvh_build_mode {
		middle, 
		equal_counts, 
		surface_area_heuristic
	};

	/**
	 * \brief bvh node, a bvh node can contain more than one element
	 * \tparam BoundingBox Bounding Box
	 * \tparam Element Element
	 */
	template<typename BoundingBox, typename Element>
	struct bvh_node {
		BoundingBox bound;

		bvh_node* left = nullptr;
		bvh_node* right = nullptr;

		int axis = 0;
		int offset = 0;
		int count = 0;

		bool is_leaf() const { return (left == nullptr && right == nullptr); }

		static bvh_node leaf(const BoundingBox &box, size_t offset, size_t count);

		static bvh_node node(int axis, bvh_node* left, bvh_node* right);
	};

	/**
	 * \brief A simple allocator for bvh
	 * \tparam BoundingBox Bounding Box
	 * \tparam Element Element
	 */
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

	/**
	 * \brief bvh bounding box helper. It provides some help function of bounding box.
	 * \tparam BoundingBox Bounding Box
	 */
	template<typename BoundingBox>
	struct bvh_bounding_helper {
		/**
		 * \brief union two bounding box
		 * \param bound first bounding box(result)
		 * \param other second bounding box
		 */
		static void apply(BoundingBox &bound, const BoundingBox &other) {
			bound.apply(other);
		}

		/**
		 * \brief union a point to bounding box
		 * \tparam Point Point Type
		 * \param bound bounding box
		 * \param point point
		 */
		template<typename Point>
		static void apply(BoundingBox &bound, const Point &point) {
			bound.apply(point);
		}

		/**
		 * \brief get the max dimension(max - min)
		 * \param bound Bounding Box
		 * \return max dimension
		 */
		static auto max_dimension(const BoundingBox &bound) -> int {
			return bound.max_dimension();
		}

		/**
		 * \brief get max property
		 * \param bound bounding box
		 * \param dim dimension(x, y, z, w, ...)
		 * \return max value of dimension
		 */
		static auto max_property(const BoundingBox& bound, int dim) {
			return bound.max_property()[dim];
		}

		/**
		 * \brief get min property
		 * \param bound bounding box
		 * \param dim dimension(x, y, z, w, ...)
		 * \return min value of dimension
		 */
		static auto min_property(const BoundingBox& bound, int dim) {
			return bound.min_property()[dim];
		}

		/**
		 * \brief get centroid
		 * \param bound bounding box
		 * \return centroid of bounding box
		 */
		static auto centroid(const BoundingBox& bound) {
			return bound.centroid();
		}

		/**
		 * \brief get centroid with dimension
		 * \param bound bounding box
		 * \param dim dimension
		 * \return centroid of bounding box with dimension
		 */
		static auto centroid(const BoundingBox& bound, int dim) {
			return bound.centroid(dim);
		}

		/**
		 * \brief get surface area of bounding box
		 * \param bound bounding box
		 * \return surface area
		 */
		static auto surface_area(const BoundingBox& bound) {
			return bound.surface_area();
		}

		/**
		 * \brief if two bounding box are intersect
		 * \param b0 first bounding box
		 * \param b1 second bounding box
		 * \return true or false
		 */
		static auto intersect(const BoundingBox& b0, const BoundingBox& b1) {
			return b0.intersect(b1);
		}
	};

	/**
	 * \brief bvh accelerator
	 * \tparam BoundingBox Bounding Box
	 * \tparam Element Element
	 * \tparam BoundingBoxHelper Help Function
	 */
	template<typename BoundingBox, typename Element, 
		typename BoundingBoxHelper = bvh_bounding_helper<BoundingBox>>
	class bvh_accelerator {
	public:
		/**
		 * \brief ctor
		 * \param bounds bounding boxes
		 * \param elements pointer to elements
		 * \param mode build mode
		 */
		bvh_accelerator(
			const std::vector<BoundingBox> &bounds,
			const std::vector<Element*> &elements,
			bvh_build_mode mode = bvh_build_mode::surface_area_heuristic);

		/**
		 * \brief get elements who are intersect with bound
		 * \param bound bounding box
		 * \return a std::vector indicate a range from first(get(0)) to end(get(1)) of elements in elements()
		 */
		auto enumerate_contacts(const BoundingBox &bound) -> std::vector<std::tuple<int, int>>;

		/**
		 * \brief get first pointer of elements in memory pool.
		 * \return first pointer of elements
		 */
		auto elements() -> Element**;

		/**
		 * \brief max number of elements in one node
		 */
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

		std::vector<Element*> elements_pool;

		auto recursive_build(
			std::vector<element_info> &infos,
			size_t start, size_t end,
			std::vector<Element*> &new_order)->node*;

		void recursive_enumerate_contacts(node* node, const BoundingBox& bound, std::vector<std::tuple<int, int>> &contacts);

		auto split(std::vector<element_info> &infos,
			const BoundingBox &centroid_bound,
			const BoundingBox &bounds, int dim,
			size_t start, size_t end)->size_t;

		auto split_middle(std::vector<element_info> &infos,
			const BoundingBox &centroid_bound,
			const BoundingBox &bounds, int dim,
			size_t start, size_t end)->size_t;

		auto split_equal_counts(std::vector<element_info> &infos,
			const BoundingBox &centroid_bound, 
			const BoundingBox &bounds, int dim,
			size_t start, size_t end)->size_t;

		auto split_surface_area_heuristic(std::vector<element_info> &infos,
			const BoundingBox &centroid_bound, 
			const BoundingBox &bounds, int dim,
			size_t start, size_t end)->size_t;

		auto cost_surface_area_heuristic(const std::vector<bucket_info> &infos,
			const BoundingBox &bounds, size_t location)->real;
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
			BoundingBox(left->bound, right->bound),
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

		root = recursive_build(infos, 0, infos.size(), this->elements_pool);
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::enumerate_contacts(
		const BoundingBox& bound) -> std::vector<std::tuple<int, int>> {
		std::vector<std::tuple<int, int>> contacts;

		recursive_enumerate_contacts(root, bound, contacts);

		return contacts;
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::elements() -> Element** {
		return elements_pool.data();
	}

	template<typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::recursive_build(std::vector<element_info> &infos,
		size_t start, size_t end, std::vector<Element*> &new_order) -> node* {
		auto node = allocator.allocate();

		auto bounds = BoundingBox(infos[start].bound);
		for (auto i = start; i < end; i++) BoundingBoxHelper::apply(bounds, infos[i].bound);

		if (end - start == 1) {
			(*node) = node::leaf(bounds, new_order.size(), 1);

			new_order.push_back(infos[start].element);
		}
		else {
			auto centroid_bound = BoundingBox(
				BoundingBoxHelper::centroid(infos[start].bound),
				BoundingBoxHelper::centroid(infos[start].bound));
			for (auto i = start; i < end; i++) BoundingBoxHelper::apply(centroid_bound, BoundingBoxHelper::centroid(infos[i].bound));

			auto dim = BoundingBoxHelper::max_dimension(centroid_bound);
			
			//same centroid, so push them into a leaf node
			if (BoundingBoxHelper::max_property(centroid_bound, dim) == BoundingBoxHelper::min_property(centroid_bound, dim)) {
				(*node) = node::leaf(bounds, new_order.size(), end - start);

				for (auto i = start; i < end; i++) new_order.push_back(infos[i].element);
			}
			else {
				auto mid = split(infos, centroid_bound, bounds, dim, start, end);

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
	void bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::recursive_enumerate_contacts(node* node,
		const BoundingBox& bound, std::vector<std::tuple<int, int>> &contacts) {
		if (!BoundingBoxHelper::intersect(node->bound, bound)) return;

		if (node->is_leaf()) {
			contacts.push_back(std::make_tuple(node->offset, node->count));

			return;
		}

		recursive_enumerate_contacts(node->left, bound, contacts);
		recursive_enumerate_contacts(node->right, bound, contacts);
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::split(std::vector<element_info> &infos,
		const BoundingBox &centroid_bound, 
		const BoundingBox &bounds, int dim,
		size_t start, size_t end) -> size_t {
		switch (mode) {
		case bvh_build_mode::middle: return split_middle(infos, centroid_bound, bounds, dim, start, end);
		case bvh_build_mode::equal_counts: return split_equal_counts(infos, centroid_bound, bounds, dim, start, end);
		case bvh_build_mode::surface_area_heuristic: return split_surface_area_heuristic(infos, centroid_bound, bounds, dim, start, end);
		default: throw std::exception("error : invalid build mode.");
		}
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::split_middle(std::vector<element_info>& infos,
		const BoundingBox& centroid_bound, 
		const BoundingBox& bounds, int dim,
		size_t start, size_t end) -> size_t {
		auto mid_position = (
			BoundingBoxHelper::min_property(centroid_bound, dim) + 
			BoundingBoxHelper::max_property(centroid_bound, dim)) * 0.5f;

		auto mid_ptr = std::partition(
			&infos[start], &infos[end - 1] + 1,
			[dim, mid_position](const element_info &info) {
			return BoundingBoxHelper::centroid(info.bound, dim) < mid_position;
		});

		auto mid = static_cast<size_t>(mid_ptr - &infos[0]);

		if (mid == start || mid == end) return split_equal_counts(infos, centroid_bound, bounds, dim, start, end);

		return mid;
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::split_equal_counts(std::vector<element_info>& infos,
		const BoundingBox& centroid_bound,
		const BoundingBox& bounds, int dim, size_t start, size_t end) -> size_t {
		auto mid = (start + end) >> 1;

		std::nth_element(&infos[start], &infos[mid], &infos[end - 1] + 1,  
			[dim](const element_info &left, const element_info& right) {
			return BoundingBoxHelper::centroid(left.bound, dim) < BoundingBoxHelper::centroid(right.bound, dim);
		});

		return mid;
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::split_surface_area_heuristic(std::vector<element_info>& infos,
		const BoundingBox& centroid_bound,
		const BoundingBox& bounds, int dim, size_t start, size_t end) -> size_t {
		if (start - end <= 4) return split_equal_counts(infos, centroid_bound, bounds, dim, start, end);

		constexpr size_t buckets_count = 12;

		std::vector<bucket_info> buckets(buckets_count);

		for (auto i = start; i < end; i++) {
			auto location = static_cast<size_t>(
				buckets_count * (
				(BoundingBoxHelper::centroid(infos[i].bound, dim) - BoundingBoxHelper::min_property(centroid_bound, dim)) /
				(BoundingBoxHelper::max_property(centroid_bound, dim) - BoundingBoxHelper::min_property(centroid_bound, dim))));
			
			if (location == buckets_count) location = buckets_count - 1;

			buckets[location].count = buckets[location].count + 1;

			BoundingBoxHelper::apply(buckets[location].bounds, infos[i].bound);
		}

		auto min_cost = cost_surface_area_heuristic(buckets, bounds, 0);
		auto min_cost_location = static_cast<size_t>(0);

		for (size_t i = 1; i < buckets_count - 1; i++) {
			auto cost = cost_surface_area_heuristic(buckets, bounds, i);

			if (min_cost > cost) min_cost = cost, min_cost_location = i;
		}

		auto leaf_cost = end - start;

		if (min_cost < leaf_cost || max_elements_per_node < leaf_cost) {
			auto mid_ptr = std::partition(&infos[start], &infos[end - 1] + 1,
				[=](const element_info &info) {
				auto location = static_cast<size_t>(
					buckets_count * (
						(BoundingBoxHelper::centroid(info.bound, dim) - BoundingBoxHelper::min_property(centroid_bound, dim)) /
						(BoundingBoxHelper::max_property(centroid_bound, dim) - BoundingBoxHelper::min_property(centroid_bound, dim))));

				if (location == buckets_count) location = buckets_count - 1;

				return location <= min_cost_location;
			});

			return mid_ptr - &infos[0];
		}
		
		return start;
	}

	template <typename BoundingBox, typename Element, typename BoundingBoxHelper>
	auto bvh_accelerator<BoundingBox, Element, BoundingBoxHelper>::cost_surface_area_heuristic(const std::vector<bucket_info>& infos,
		const BoundingBox &bounds, size_t location) -> real {
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
			info1.count * BoundingBoxHelper::surface_area(info1.bounds)) / BoundingBoxHelper::surface_area(bounds) * test_cost;
	}

}
