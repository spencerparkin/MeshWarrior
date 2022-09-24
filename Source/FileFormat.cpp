#include "FileFormat.h"
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

	if (fileObjectArray.size() != 1 || !dynamic_cast<Mesh*>(fileObjectArray[0]))
	{
		FileObject::DeleteArray(fileObjectArray);
		return nullptr;
	}

	return (Mesh*)fileObjectArray[0];
}

bool FileFormat::SaveMesh(const std::string& meshFile, const Mesh& mesh)
{
	std::vector<FileObject*> fileObjectArray;
	fileObjectArray.push_back(const_cast<Mesh*>(&mesh));

	return this->Save(meshFile, fileObjectArray);
}