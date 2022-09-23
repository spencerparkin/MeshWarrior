#include "MeshUnionOperation.h"

using namespace MeshWarrior;

MeshUnionOperation::MeshUnionOperation()
{
}

/*virtual*/ MeshUnionOperation::~MeshUnionOperation()
{
}

/*virtual*/ Mesh* MeshUnionOperation::Calculate(const Mesh* meshA, const Mesh* meshB)
{
	this->ProcessMeshes(meshA, meshB);

	return nullptr;
}