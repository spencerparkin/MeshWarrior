#include "MeshIntersectionOperation.h"

using namespace MeshWarrior;

MeshIntersectionOperation::MeshIntersectionOperation()
{
}

/*virtual*/ MeshIntersectionOperation::~MeshIntersectionOperation()
{
}

/*virtual*/ Mesh* MeshIntersectionOperation::Calculate(const Mesh* meshA, const Mesh* meshB)
{
	this->ProcessMeshes(meshA, meshB);

	// Take all inside A polygons with all inside B polygons.
	// Reverse all taken polygons.

	return nullptr;
}