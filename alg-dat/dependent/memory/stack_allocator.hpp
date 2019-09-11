#pragma once

/*
 * @name stack_allocator.hpp
 *
 * stack_allocator(filo) is a element_allocator, it allocate Element like stack.
 * The last Element will be destroy first and the stack class is base this memory allocator.
 *
 * We can use allocate or construct to create some elements.
 * But the order of destroy is limit. The first we create will be destroy last.
 */

#include "allocator.hpp"

namespace alg_dat {

	template<typename Element, typename ExpandClass = expand_class_mul>
	class stack_allocator : public element_allocator_interface<Element, ExpandClass> {
	public:
		using base = element_allocator_interface<Element, ExpandClass>;
		using base::expand_class;
		using base::size_type;
		using base::type;
	public:
		stack_allocator() = default;

		stack_allocator(size_type space, size_type factor = 2) : base(space, factor) {}

		stack_allocator(const stack_allocator& allocator) : base(allocator) {}

		stack_allocator(stack_allocator&& allocator) noexcept : base(allocator) {}

		stack_allocator& operator=(const stack_allocator& allocator) {
			base::mExpandFactor = allocator.mExpandFactor;
			base::mMemorySpace = allocator.mMemorySpace;
			base::mMemorySize = allocator.mMemorySize;

			if (base::mElements != nullptr) std::free(base::mElements);

			base::mElements = static_cast<Element*>(std::malloc(base::mMemorySpace * sizeof(Element)));

			std::copy(allocator.mElements, allocator.mElements + allocator.mMemorySize, base::mElements);

			return *this;
		}

		stack_allocator& operator=(stack_allocator&& allocator) noexcept {
			if (this == &allocator) return *this;

			std::swap(base::mElements, allocator.mElements);
			std::swap(base::mExpandFactor, allocator.mExpandFactor);
			std::swap(base::mMemorySpace, allocator.mMemorySpace);
			std::swap(base::mMemorySize, allocator.mMemorySize);

			return *this;
		}

		~stack_allocator() = default;

		auto allocate(size_type count = 1)->Element* {
			//check the memory if enough
			base::expand_if_not_enough(base::mMemorySize + count);

			const auto begin = base::mElements + base::mMemorySize;
			const auto end = base::mElements + base::mMemorySize + count;

			//construct the elements
			for (auto it = begin; it != end; ++it) new (it)Element();

			base::mMemorySize = base::mMemorySize + count;

			return base::mElements + base::mMemorySize - count;
		}

		template<typename ...Types>
		auto construct(Types&& ... args)->Element* {
			base::expand_if_not_enough(++base::mMemorySize);

			return new (base::mElements + base::mMemorySize - 1)Element(std::forward<Types>(args)...);
		}

		void deallocate(size_type count = 1) {
			assert(base::mMemorySize >= count);

			const auto begin = base::mElements + base::mMemorySize - 1;
			const auto end = base::mElements + base::mMemorySize - count - 1;

			for (auto it = begin; it != end; --it) it->~Element();

			base::mMemorySize = base::mMemorySize - count;
		}

		void destroy() const {
			deallocate();
		}
	};
}