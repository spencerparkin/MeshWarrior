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

	Mesh* meshA = objFormat.LoadMesh("BoxA.OBJ");
	Mesh* meshB = objFormat.LoadMesh("BoxB.OBJ");

	Mesh* meshResult = nullptr;
	MeshOperation* meshOp = new MeshUnionOperation();

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