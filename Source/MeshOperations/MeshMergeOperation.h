#pragma once

#include "../MeshOperation.h"

namespace MeshWarrior
{
	class MESH_WARRIOR_API MeshMergeOperation : public MeshOperation
	{
	public:
		MeshMergeOperation();
		virtual ~MeshMergeOperation();

		virtual Mesh* Calculate(const Mesh* meshA, const Mesh* meshB) override;
	};
}