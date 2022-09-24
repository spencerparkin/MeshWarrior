#include "FileObject.h"
#include <sstream>

using namespace MeshWarrior;

FileObject::FileObject()
{
	this->name = new std::string;

	static int i = 0;
	std::stringstream stringStream;
	stringStream << "Poopers_" << i++;

	*this->name = stringStream.str();
}

/*virtual*/ FileObject::~FileObject()
{
	delete this->name;
}

/*static*/ void FileObject::DeleteArray(std::vector<FileObject*>& fileObjectArray)
{
	for (FileObject* fileObject : fileObjectArray)
		delete fileObject;

	fileObjectArray.clear();
}