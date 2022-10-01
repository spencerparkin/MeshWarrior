#include "FileFormat.h"
#include "MeshOperations/MeshMergeOperation.h"
#include "Mesh.h"

using namespace MeshWarrior;

FileFormat::FileFormat()
{
}

/*virtual*/ FileFormat::~FileFormat()
{
}

Mesh* FileFormat::LoadMesh(const std::string& meshFile)
{
	std::vector<FileObject*> fileObjectArray;
	this->Load(meshFile, fileObjectArray);

	std::vector<Mesh*> meshArray;
	for (FileObject*& fileObject : fileObjectArray)
	{
		Mesh* mesh = dynamic_cast<Mesh*>(fileObject);
		if (mesh)
		{
			meshArray.push_back(mesh);
			fileObject = nullptr;
		}
	}

	FileObject::DeleteArray(fileObjectArray);

	Mesh* resultMesh = nullptr;

	if (meshArray.size() == 1)
		resultMesh = meshArray[0];
	else
	{
		MeshMergeOperation mergeOp;
		std::vector<Mesh*> resultArray;
		if (mergeOp.Calculate(meshArray, resultArray) && resultArray.size() == 1)
			resultMesh = resultArray[0];

		for (Mesh* mesh : meshArray)
			delete mesh;
	}

	return resultMesh;
}

bool FileFormat::SaveMesh(const std::string& meshFile, const Mesh& mesh)
{
	std::vector<FileObject*> fileObjectArray;
	fileObjectArray.push_back(const_cast<Mesh*>(&mesh));

	return this->Save(meshFile, fileObjectArray);
}