#include "FileFormats/OBJFormat.h"
#include "MeshOperations/MeshUnionOperation.h"
#include "MeshOperations/MeshIntersectionOperation.h"
#include "MeshOperations/MeshDifferenceOperation.h"
#include "MeshOperations/MeshMergeOperation.h"
#include "Mesh.h"

int main()
{
	using namespace MeshWarrior;

	OBJFormat objFormat;

	Mesh* meshA = objFormat.LoadMesh("Sphere.OBJ");
	Mesh* meshB = objFormat.LoadMesh("Box.OBJ");

	Mesh* meshResult = nullptr;
	MeshOperation* meshOp = new MeshIntersectionOperation();

	if (meshA && meshB)
		meshResult = meshOp->Calculate(meshA, meshB);

	delete meshA;
	delete meshB;

	if (meshResult)
	{
		objFormat.SaveMesh("Result.OBJ", *meshResult);
		delete meshResult;
	}

	delete meshOp;

	return 0;
}