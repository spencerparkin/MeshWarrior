#pragma once

#include "Defines.h"
#include <vector>
#include <assert.h>

namespace MeshWarrior
{
	template<typename Type>
	class MESH_WARRIOR_API TypeHeap
	{
	public:
		TypeHeap()
		{
		}

		virtual ~TypeHeap()
		{
		}

		virtual Type* Allocate()
		{
			return new Type();
		}

		virtual void Deallocate(Type* type)
		{
			delete type;
		}
	};

	// Many algorithms use sets of pointers and traverse those sets.
	// This works, but the problem is that if the pointers are taken from the
	// usual memory heap, then the set iteration is not deterministic across
	// runs of the software.  This can make tracking down bugs very difficult.
	// So here, if set members are allocated from this heap, the base offset
	// of the heap may be inconsistent across runs, but the relative ordering
	// of the members will be consistent.  In release, we don't want to use it,
	// though, because it usually puts a very small cap on the size of the heap.
	template<typename Type>
	class MESH_WARRIOR_API StackHeap : public TypeHeap<Type>
	{
	public:
		StackHeap(int stackSize)
		{
			this->stack = new std::vector<int>();

			for (int i = 0; i < stackSize; i++)
				this->stack->push_back(i);

			this->memorySize = stackSize * sizeof(Type);
			this->memory = new char[this->memorySize];
		}

		virtual ~StackHeap()
		{
			delete[] this->memory;
			delete this->stack;
		}

		virtual Type* Allocate() override
		{
			if (this->stack->size() == 0)
			{
				MW_ASSERT(false);
				return nullptr;
			}

			int i = *this->stack->rbegin();
			this->stack->pop_back();
			int j = i * sizeof(Type);
			Type* type = reinterpret_cast<Type*>(&this->memory[j]);
			new (type) Type();
			return type;
		}

		virtual void Deallocate(Type* type) override
		{
			int j = (char*)type - this->memory;
			MW_ASSERT(0 <= j && j < this->memorySize);
			if (0 <= j && j < this->memorySize)
			{
				type->~Type();
				int i = j / sizeof(Type);
				this->stack->push_back(i);
			}
		}

	private:

		char* memory;
		int memorySize;
		std::vector<int>* stack;
	};
}