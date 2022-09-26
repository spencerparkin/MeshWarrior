#include "MeshOperation.h"

using namespace MeshWarrior;

MeshOperation::MeshOperation()
{
	this->error = new std::string();
}

/*virtual*/ MeshOperation::~MeshOperation()
{
	delete this->error;
}