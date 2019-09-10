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

		allocator_interface(const allocator_interface &allocator) :
			mExpandFactor(allocator.mExpandFactor),
			mMemorySpace(allocator.mMemorySpace),
			mMemorySize(allocator.mMemorySize) {}

		allocator_interface(allocator_interface &&allocator) noexcept
		{
			std::swap(mExpandFactor, allocator.mExpandFactor);
			std::swap(mMemorySpace, allocator.mMemorySpace);
			std::swap(mMemorySize, allocator.mMemorySize);
		}
		
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
		element_allocator_interface() : element_allocator_interface(255, 2) {}

		element_allocator_interface(size_type space, size_type factor) :
			base(space, factor)
		{
			assert(space != 0);

			mElements = static_cast<Element*>(std::malloc(base::mMemorySpace * sizeof(Element)));
		}

		element_allocator_interface(const element_allocator_interface &allocator) :
			base(allocator)
		{
			mElements = static_cast<Element*>(std::malloc(base::mMemorySpace * sizeof(Element)));

			std::copy(allocator.mElements, allocator.mElements + allocator.mMemorySize, mElements);
		}

		element_allocator_interface(element_allocator_interface &&allocator) noexcept :
			base(allocator)
		{
			std::swap(mElements, allocator.mElements);
		}
		
		~element_allocator_interface()
		{
			if (mElements == nullptr) return;

			std::free(mElements);

			mElements = nullptr;
		}

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
		stack_allocator() = default;
		
		stack_allocator(size_type space, size_type factor = 2) : base(space, factor) {}

		stack_allocator(const stack_allocator& allocator) : base(allocator) {}
		
		stack_allocator(stack_allocator&& allocator) noexcept : base(allocator) {}

		stack_allocator& operator=(const stack_allocator &allocator) {
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
		auto construct(Types&&... args)->Element* {
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

		void destroy(Element* element) const {
			assert(static_cast<size_type>(element - base::mElements) < base::mMemorySize);

			element->~Element();
		}
	};
}