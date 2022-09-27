#include "MeshUnionOperation.h"

using namespace MeshWarrior;

MeshUnionOperation::MeshUnionOperation()
{
}

/*virtual*/ MeshUnionOperation::~MeshUnionOperation()
{
}

/*virtual*/ Mesh* MeshUnionOperation::Calculate(const Mesh* meshA, const Mesh* meshB)
{
	this->ProcessMeshes(meshA, meshB);

	Mesh* resultMesh = nullptr;

	if (this->error->length() == 0)
	{
		resultMesh = new Mesh();

		this->graphA->ForAllElements([&resultMesh](MeshGraph::GraphElement* element) -> bool {
			Graph::Node* node = dynamic_cast<Graph::Node*>(element);
			if (node && node->side == Graph::Node::OUTSIDE)
				resultMesh->AddFace(node->MakePolygon());
			return false;
		});

		this->graphB->ForAllElements([&resultMesh](MeshGraph::GraphElement* element) -> bool {
			Graph::Node* node = dynamic_cast<Graph::Node*>(element);
			if (node && node->side == Graph::Node::OUTSIDE)
				resultMesh->AddFace(node->MakePolygon());
			return false;
		});
	}

	return resultMesh;
}