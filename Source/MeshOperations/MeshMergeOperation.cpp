#include "MeshMergeOperation.h"
#include "../Mesh.h"

using namespace MeshWarrior;

MeshMergeOperation::MeshMergeOperation()
{
}

/*virtual*/ MeshMergeOperation::~MeshMergeOperation()
{
}

/*virtual*/ bool MeshMergeOperation::Calculate(const std::vector<Mesh*>& inputMeshArray, std::vector<Mesh*>& outputMeshArray)
{
	std::vector<Mesh::ConvexPolygon> polygonArray;
	for (const Mesh* inputMesh : inputMeshArray)
		inputMesh->ToPolygonArray(polygonArray, true);

	Mesh* mergedMesh = new Mesh();
	for (Mesh::ConvexPolygon& polygon : polygonArray)
		mergedMesh->AddFace(polygon);	// This will be slow unless we have a good index mechanism in the mesh data-structure.

	outputMeshArray.clear();
	outputMeshArray.push_back(mergedMesh);
	return true;
}