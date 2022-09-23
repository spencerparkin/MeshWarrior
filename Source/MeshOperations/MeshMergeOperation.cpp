#include "MeshMergeOperation.h"
#include "../Mesh.h"

using namespace MeshWarrior;

MeshMergeOperation::MeshMergeOperation()
{
}

/*virtual*/ MeshMergeOperation::~MeshMergeOperation()
{
}

/*virtual*/ Mesh* MeshMergeOperation::Calculate(const Mesh* meshA, const Mesh* meshB)
{
	std::vector<Mesh::ConvexPolygon> polygonArrayA, polygonArrayB;

	meshA->ToPolygonArray(polygonArrayA);
	meshB->ToPolygonArray(polygonArrayB);

	std::vector<Mesh::ConvexPolygon> mergedPolygonArray;

	for (Mesh::ConvexPolygon& polygon : polygonArrayA)
		mergedPolygonArray.push_back(polygon);

	for (Mesh::ConvexPolygon& polygon : polygonArrayB)
		mergedPolygonArray.push_back(polygon);

	Mesh* mergedMesh = new Mesh();
	mergedMesh->FromPolygonArray(mergedPolygonArray);

	return mergedMesh;
}