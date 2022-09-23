#include "MeshFormats/OBJFormat.h"
#include "MeshOperations/MeshUnionOperation.h"
#include "MeshOperations/MeshIntersectionOperation.h"
#include "MeshOperations/MeshDifferenceOperation.h"
#include "MeshOperations/MeshMergeOperation.h"
#include "Mesh.h"

int main()
{
	using namespace MeshWarrior;

	OBJFormat objFormat;

	Mesh* meshA = objFormat.Load("Box.OBJ");
	Mesh* meshB = objFormat.Load("Sphere.OBJ");

	Mesh* meshResult = nullptr;
	MeshOperation* meshOp = new MeshMergeOperation();

	if (meshA && meshB)
		meshResult = meshOp->Calculate(meshA, meshB);

	delete meshA;
	delete meshB;

	if (meshResult)
	{
		objFormat.Save("Result.OBJ", *meshResult);
		delete meshResult;
	}

	delete meshOp;

	return 0;
}