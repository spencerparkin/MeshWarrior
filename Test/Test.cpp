#include "FileFormats/OBJFormat.h"
#include "MeshOperations/MeshSetOperation.h"
#include "MeshOperations/MeshMergeOperation.h"
#include "Mesh.h"
#include <iostream>

int main()
{
	using namespace MeshWarrior;

	OBJFormat objFormat;

	Mesh* meshA = objFormat.LoadMesh("Box.OBJ");
	Mesh* meshB = objFormat.LoadMesh("Sphere.OBJ");

	int flags = MW_FLAG_UNION_SET_OP | MW_FLAG_INTERSECTION_SETP_OP | MW_FLAG_A_MINUS_B_SET_OP | MW_FLAG_B_MINUS_A_SET_OP;

	Mesh* meshResult = nullptr;
	MeshOperation* meshOp = new MeshSetOperation(flags);

	std::vector<Mesh*> inputMeshArray;
	inputMeshArray.push_back(meshA);
	inputMeshArray.push_back(meshB);

	std::vector<Mesh*> outputMeshArray;

	if (!meshOp->Calculate(inputMeshArray, outputMeshArray))
	{
		std::cerr << "Mesh calculation failed!" << std::endl;
		std::cerr << meshOp->error->c_str() << std::endl;
	}
	else
	{
		std::vector<FileObject*> fileObjectArray;
		for (Mesh* mesh : outputMeshArray)
			fileObjectArray.push_back(mesh);

		objFormat.Save("Result.OBJ", fileObjectArray);
	}

	for (Mesh* mesh : inputMeshArray)
		delete mesh;

	for (Mesh* mesh : outputMeshArray)
		delete mesh;

	delete meshOp;

	return 0;
}