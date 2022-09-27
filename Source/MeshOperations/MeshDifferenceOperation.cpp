#include "MeshDifferenceOperation.h"

using namespace MeshWarrior;

MeshDifferenceOperation::MeshDifferenceOperation()
{
}

/*virtual*/ MeshDifferenceOperation::~MeshDifferenceOperation()
{
}

/*virtual*/ Mesh* MeshDifferenceOperation::Calculate(const Mesh* meshA, const Mesh* meshB)
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
			if (node && node->side == Graph::Node::INSIDE)
				resultMesh->AddFace(node->MakePolygon().ReverseWinding());
			return false;
		});
	}

	return resultMesh;
}