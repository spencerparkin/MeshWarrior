#pragma once

#include "Defines.h"
#include "Vector.h"
#include <vector>

namespace MeshWarrior
{
	class MESH_WARRIOR_API ConvexPolygon
	{
	public:
		ConvexPolygon();
		virtual ~ConvexPolygon();

		bool IsValid() const;

		void ReverseWinding();

		std::vector<Vector>* vertexArray;
	};
}