#pragma once

#include "../MeshSetOperation.h"

namespace MeshWarrior
{
	class MESH_WARRIOR_API MeshIntersectionOperation : public MeshSetOperation
	{
	public:
		MeshIntersectionOperation();
		virtual ~MeshIntersectionOperation();

		virtual Mesh* Calculate(const Mesh* meshA, const Mesh* meshB) override;
	};
}