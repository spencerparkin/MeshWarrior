#include "MeshFormats/OBJFormat.h"
#include "MeshOperations/MeshUnionOperation.h"
#include "MeshOperations/MeshIntersectionOperation.h"
#include "MeshOperations/MeshDifferenceOperation.h"
#include "Mesh.h"

int main()
{
	using namespace MeshWarrior;

	OBJFormat objFormat;

	Mesh* meshA = objFormat.Load("Box.OBJ");
	Mesh* meshB = objFormat.Load("Sphere.OBJ");

	Mesh* meshUnion = nullptr;
	Mesh* meshIntersection = nullptr;
	Mesh* meshDifference = nullptr;

	if (meshA && meshB)
	{
		MeshUnionOperation unionOp;
		meshUnion = unionOp.Calculate(meshA, meshB);

		MeshIntersectionOperation intersectOp;
		meshIntersection = intersectOp.Calculate(meshA, meshB);

		MeshDifferenceOperation diffOp;
		meshDifference = diffOp.Calculate(meshA, meshB);
	}

	delete meshA;
	delete meshB;

	if (meshUnion)
		objFormat.Save("Box_Union_Sphere.OBJ", *meshUnion);

	if (meshIntersection)
		objFormat.Save("Box_Intersect_Sphere.OBJ", *meshIntersection);

	if (meshDifference)
		objFormat.Save("Box_Diff_Sphere.OBJ", *meshDifference);

	delete meshUnion;
	delete meshIntersection;
	delete meshDifference;

	return 0;
}