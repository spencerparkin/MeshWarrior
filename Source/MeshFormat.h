#pragma once

#include "Defines.h"
#include <string>

namespace MeshWarrior
{
	class Mesh;

	class MESH_WARRIOR_API MeshFormat
	{
	public:
		MeshFormat();
		virtual ~MeshFormat();

		virtual Mesh* Load(const std::string& meshFile) = 0;
		virtual bool Save(const std::string& meshFile, const Mesh& mesh) = 0;
	};
}