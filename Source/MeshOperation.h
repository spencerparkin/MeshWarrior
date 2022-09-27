#pragma once

#include "Defines.h"
#include <string>
#include <vector>

namespace MeshWarrior
{
	class Mesh;

	class MESH_WARRIOR_API MeshOperation
	{
	public:
		MeshOperation();
		virtual ~MeshOperation();

		virtual bool Calculate(const std::vector<Mesh*>& inputMeshArray, std::vector<Mesh*>& outputMeshArray) = 0;

		std::string* error;
	};
}