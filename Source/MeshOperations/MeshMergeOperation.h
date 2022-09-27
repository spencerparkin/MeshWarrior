#pragma once

#include "../MeshOperation.h"

namespace MeshWarrior
{
	class MESH_WARRIOR_API MeshMergeOperation : public MeshOperation
	{
	public:
		MeshMergeOperation();
		virtual ~MeshMergeOperation();

		virtual bool Calculate(const std::vector<Mesh*>& inputMeshArray, std::vector<Mesh*>& outputMeshArray) override;
	};
}