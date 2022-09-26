#pragma once

#include "Defines.h"
#include <vector>
#include <functional>
#include <list>

namespace MeshWarrior
{
	template<typename Type>
	void CompressArray(std::vector<Type*>& givenArray, std::function<Type*(const Type*, const Type*)> compressorFunc)
	{
		if (givenArray.size() <= 1)
			return;

		int i = (givenArray.size() % 2 == 0) ? (givenArray.size() / 2) : ((givenArray.size() + 1) / 2);
		std::vector<Type*> arrayA, arrayB;
		for (int j = 0; j < i; j++)
			arrayA.push_back(givenArray[j]);
		for (int j = i; j < (signed)givenArray.size(); j++)
			arrayB.push_back(givenArray[j]);

		// Divide and conqur.
		CompressArray(arrayA, compressorFunc);
		CompressArray(arrayB, compressorFunc);

		struct Pair
		{
			int i, j;
		};

		std::list<Pair> pairQueue;
		for (int i = 0; i < (signed)arrayA.size(); i++)
			for (int j = 0; j < (signed)arrayB.size(); j++)
				pairQueue.push_back(Pair{ i, j });

		while (pairQueue.size() > 0)
		{
			typename std::list<Pair>::iterator iter = pairQueue.begin();
			Pair pair = *iter;
			pairQueue.erase(iter);

			const Type* typeA = arrayA[pair.i];
			const Type* typeB = arrayB[pair.j];

			// If the compressor function returns a new allocation, it should delete the given allocations.
			Type* newType = compressorFunc(typeA, typeB);
			if (newType)
			{
				arrayA[pair.i] = nullptr;
				arrayB[pair.j] = nullptr;

				// Forget about checking anything against the deleted types.
				// This is a one-liner in Python!
				iter = pairQueue.begin();
				while (iter != pairQueue.end())
				{
					typename std::list<Pair>::iterator nextIter = iter;
					nextIter++;
					Pair& existingPair = *iter;
					if (existingPair.i == pair.i || existingPair.j == pair.j)
						pairQueue.erase(iter);
					iter = nextIter;
				}

				// Arbitrarily add the new type to arrayB.
				arrayB.push_back(newType);

				// Now check all remaining A types against the new B type.
				for (int i = 0; i < (signed)arrayA.size(); i++)
					if (arrayA[i] != nullptr)
						pairQueue.push_back(Pair{ i, (signed)arrayB.size() - 1 });
			}
		}

		// Laslty, merge the arrays into the given array.
		// Doing this in Python is so much nicer looking.  C++20 has some solutions, but I don't care right now.
		givenArray.clear();
		for (int i = 0; i < (signed)arrayA.size(); i++)
			if (arrayA[i] != nullptr)
				givenArray.push_back(arrayA[i]);
		for (int i = 0; i < (signed)arrayB.size(); i++)
			if (arrayB[i] != nullptr)
				givenArray.push_back(arrayB[i]);
	}
}