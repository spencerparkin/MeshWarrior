#pragma once

#include "../MeshSetOperation.h"

namespace MeshWarrior
{
	class MESH_WARRIOR_API MeshDifferenceOperation : public MeshSetOperation
	{
	public:
		MeshDifferenceOperation();
		virtual ~MeshDifferenceOperation();

		virtual Mesh* Calculate(const Mesh* meshA, const Mesh* meshB) override;
	};
}