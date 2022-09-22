#include "MeshTopology.h"

using namespace MeshWarrior;

MeshTopology::MeshTopology()
{
}

/*virtual*/ MeshTopology::~MeshTopology()
{
}

bool MeshTopology::FromMesh(const Mesh* mesh)
{
	return false;
}

Mesh* MeshTopology::ToMesh() const
{
	return nullptr;
}