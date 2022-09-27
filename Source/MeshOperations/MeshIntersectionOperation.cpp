#include "MeshIntersectionOperation.h"

using namespace MeshWarrior;

MeshIntersectionOperation::MeshIntersectionOperation()
{
}

/*virtual*/ MeshIntersectionOperation::~MeshIntersectionOperation()
{
}

/*virtual*/ Mesh* MeshIntersectionOperation::Calculate(const Mesh* meshA, const Mesh* meshB)
{
	this->ProcessMeshes(meshA, meshB);

	Mesh* resultMesh = nullptr;

	if (this->error->length() == 0)
	{
		resultMesh = new Mesh();

		std::vector<Graph::Node*> nodeArray;

		this->graphA->ForAllElements([&resultMesh](MeshGraph::GraphElement* element) -> bool {
			Graph::Node* node = dynamic_cast<Graph::Node*>(element);
			if (node && node->side == Graph::Node::INSIDE)
				resultMesh->AddFace(node->MakePolygon().ReverseWinding());
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