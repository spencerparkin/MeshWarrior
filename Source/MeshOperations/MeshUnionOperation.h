#pragma once

#include "../MeshSetOperation.h"

namespace MeshWarrior
{
	class MESH_WARRIOR_API MeshUnionOperation : public MeshSetOperation
	{
	public:
		MeshUnionOperation();
		virtual ~MeshUnionOperation();

		virtual Mesh* Calculate(const Mesh* meshA, const Mesh* meshB) override;
	};
}