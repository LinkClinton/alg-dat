#pragma once

#include <functional>
#include <cassert>
#include <memory>

namespace alg_dat {
	
	template<typename ExpandClass>
	class allocator_interface {
	public:
		using size_type = size_t;
		using expand_class = ExpandClass;
	public:
		size_type size() const { return mMemorySize; }

		size_type space() const { return mMemorySpace; }

		size_type factor() const { return mExpandFactor; }
	protected:
		allocator_interface() = default;

		allocator_interface(size_type space, size_type factor) :
			mExpandFactor(factor), mMemorySpace(space) {}
		
		virtual ~allocator_interface() = default;
	protected:
		size_type mExpandFactor = 0;
		size_type mMemorySpace = 0;
		size_type mMemorySize = 0;
	};

	struct expand_class_add {
		using size_type = allocator_interface<expand_class_add>::size_type;
		
		size_type operator()(
			const allocator_interface<expand_class_add>* allocator, 
			size_type space_need) const
		{
			assert(allocator->factor() != 0);
			
			return (space_need - allocator->space() + 1) / allocator->factor();
		}
	};

	struct expand_class_mul {
		using size_type = allocator_interface<expand_class_mul>::size_type;

		size_type operator()(
			const allocator_interface<expand_class_mul>* allocator,
			size_type space_need) const
		{
			assert(allocator->factor() != 0);
			assert(allocator->factor() != 1);
			
			auto space = allocator->space();
			
			while (space < space_need) space = space * allocator->factor();

			return space - allocator->space();
		}
	};
	
	template<typename Element, typename ExpandClass>
	class element_allocator_interface : public allocator_interface<ExpandClass> {
	public:
		using base = allocator_interface<ExpandClass>;
		using typename base::expand_class;
		using typename base::size_type;
		using type = Element;
	protected:
		element_allocator_interface() = default;

		element_allocator_interface(size_type space, size_type factor) :
			base(space, factor) {}
		
		~element_allocator_interface() = default;

		void expand_if_not_enough(size_type space_need) {
			static ExpandClass expandSpace;
			
			//when space we have more than space we need, we do nothing.
			if (base::mMemorySpace > space_need) return;
			
			//compute the space we need, we have three way to expand.
			//the space is the number of elements, not the size in bytes of elements.
			base::mMemorySpace = expandSpace(this, space_need);

			assert(base::mMemorySpace > space_need);

			//malloc the memory we need
			const auto temp = static_cast<Element*>(std::malloc(base::mMemorySpace * sizeof(Element)));

			//we use std::move to move the elements, not copy
			//and we free the old memory
			std::move(mElements, mElements + base::mMemorySize, temp);
			std::free(mElements);

			mElements = temp;
		}
	protected:
		Element* mElements = nullptr;
	};

	template<typename Element, typename ExpandClass = expand_class_mul>
	class stack_allocator : public element_allocator_interface<Element, ExpandClass> {
	public:
		using base = element_allocator_interface<Element, ExpandClass>;
		using base::expand_class;
		using base::size_type;
		using base::type;
	public:
		stack_allocator() : stack_allocator(255, 2) {}
		
		stack_allocator(size_type space, size_type factor = 2) :
			base(space, factor)
		{
			assert(space != 0);

			base::mElements = static_cast<Element*>(std::malloc(base::mMemorySpace * sizeof(Element)));
		}

		~stack_allocator() { std::free(base::mElements); }

		auto allocate(size_type count = 1)->Element* {
			//check the memory if enough
			base::expand_if_not_enough(base::mMemorySize + count);

			const auto begin = base::mElements + base::mMemorySize;
			const auto end = base::mElements + base::mMemorySize + count;

			//construct the elements
			for (auto it = begin; it != end; ++it) 
			
			base::mMemorySize = base::mMemorySize + count;
			
			return base::mElements + base::mMemorySize - count;
		}

		auto deallocate(size_type count = 1) {
			assert(base::mMemorySize >= count);

			const auto begin = base::mElements + base::mMemorySize - 1;
			const auto end = base::mElements + base::mMemorySize - count - 1;

			for (auto it = begin; it != end; --it) it->~Element();

			base::mMemorySize = base::mMemorySize - count;
		}
	};
}