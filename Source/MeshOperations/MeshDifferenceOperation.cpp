#include "MeshDifferenceOperation.h"

using namespace MeshWarrior;

MeshDifferenceOperation::MeshDifferenceOperation()
{
}

/*virtual*/ MeshDifferenceOperation::~MeshDifferenceOperation()
{
}

/*virtual*/ Mesh* MeshDifferenceOperation::Calculate(const Mesh* meshA, const Mesh* meshB)
{
	this->ProcessMeshes(meshA, meshB);

	// Take all outside A polygons with all inside B polygons.
	// Reverse all taken B polygons.

	return nullptr;
}