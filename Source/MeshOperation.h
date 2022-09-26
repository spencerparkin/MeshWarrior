#pragma once

#include "Defines.h"
#include <string>

namespace MeshWarrior
{
	class Mesh;

	// Here, a mesh operation produces a mesh as a function of two other meshes.
	class MESH_WARRIOR_API MeshOperation
	{
	public:
		MeshOperation();
		virtual ~MeshOperation();

		virtual Mesh* Calculate(const Mesh* meshA, const Mesh* meshB) = 0;

		std::string* error;
	};
}