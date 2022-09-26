#pragma once

#include "Defines.h"
#include <vector>
#include <functional>
#include <list>

namespace MeshWarrior
{
	// Regardless of the order of the given array, this should consistently compress if
	// the given compression function is both associative and commutative.
	template<typename Type>
	void CompressArray(std::vector<Type*>& givenArray, std::function<Type*(const Type*, const Type*)> compressorFunc)
	{
		struct Pair
		{
			int i, j;
		};

		std::list<Pair> pairQueue;
		for (int i = 0; i < (signed)givenArray.size(); i++)
			for (int j = i + 1; j < (signed)givenArray.size(); j++)
				pairQueue.push_back(Pair{ i, j });

		while (pairQueue.size() > 0)
		{
			typename std::list<Pair>::iterator iter = pairQueue.begin();
			Pair pair = *iter;
			pairQueue.erase(iter);

			const Type* typeA = givenArray[pair.i];
			const Type* typeB = givenArray[pair.j];

			if (typeA && typeB)
			{
				// If the compressor function returns a new allocation, it should delete the given allocations.
				Type* newType = compressorFunc(typeA, typeB);
				if (newType)
				{
					givenArray[pair.i] = nullptr;
					givenArray[pair.j] = nullptr;

					givenArray.push_back(newType);

					for (int i = 0; i < (signed)givenArray.size() - 1; i++)
						if (givenArray[i] != nullptr)
							pairQueue.push_back(Pair{ i, (signed)givenArray.size() - 1 });
				}
			}
		}

		std::vector<Type*> contiguousArray;
		for (int i = 0; i < (signed)givenArray.size(); i++)
			if (givenArray[i] != nullptr)
				contiguousArray.push_back(givenArray[i]);

		givenArray = contiguousArray;
	}
}