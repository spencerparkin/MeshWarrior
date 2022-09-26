#include "MeshGraph.h"

using namespace MeshWarrior;

//--------------------------------- MeshGraph ---------------------------------

MeshGraph::MeshGraph()
{
	this->graphElementArray = new std::vector<GraphElement*>();
	this->targetMesh = nullptr;
}

/*virtual*/ MeshGraph::~MeshGraph()
{
	this->Clear();
	delete this->graphElementArray;
}

void MeshGraph::Generate(const Mesh* mesh)
{
	this->Clear();

	this->targetMesh = mesh;

	BoundingBoxTree tree;
	tree.SetRootBox(mesh->CalcBoundingBox());

	for (int i = 0; i < mesh->GetNumFaces(); i++)
	{
		Node* node = this->NodeFactory();
		this->graphElementArray->push_back(node);
		node->polygon_i = i;
		tree.AddGuest(new Face(node));
	}

	std::list<BoundingBoxTree::Guest*> guestList;
	tree.GatherAllGuests(guestList);

	for (BoundingBoxTree::Guest* guest : guestList)
	{
		Face* face = (Face*)guest;
		AxisAlignedBox boundingBox = face->CalcBoundingBox();
		// TODO: Fatten the bounding box, accounting for boxes that may have zero width/height/depth in some dimensions.

		std::list<BoundingBoxTree::Guest*> foundGuestList;
		tree.FindGuests(boundingBox, foundGuestList);

		for (BoundingBoxTree::Guest* foundGuest : foundGuestList)
		{
			Face* foundFace = (Face*)foundGuest;
			if (foundFace == face)
				continue;

			Edge* edge = this->FindCommonEdge(face->node, foundFace->node);
			if (edge)
			{
				face->node->edgeArray.push_back(edge);
				foundFace->node->edgeArray.push_back(edge);
			}
		}
	}
}

MeshGraph::Edge* MeshGraph::FindCommonEdge(Node* nodeA, Node* nodeB)
{
	return nullptr;
}

void MeshGraph::Clear()
{
	for (GraphElement* element : *this->graphElementArray)
		delete element;

	this->graphElementArray->clear();
}

/*virtual*/ MeshGraph::Node* MeshGraph::NodeFactory()
{
	return new Node(this);
}

/*virtual*/ MeshGraph::Edge* MeshGraph::EdgeFactory()
{
	return new Edge(this);
}

//--------------------------------- GraphElement ---------------------------------

MeshGraph::GraphElement::GraphElement(MeshGraph* meshGraph)
{
	this->meshGraph = meshGraph;
}

/*virtual*/ MeshGraph::GraphElement::~GraphElement()
{
}

//--------------------------------- Node ---------------------------------

MeshGraph::Node::Node(MeshGraph* meshGraph) : GraphElement(meshGraph)
{
	this->polygon_i = -1;
}

/*virtual*/ MeshGraph::Node::~Node()
{
}

//--------------------------------- EdgeVertex ---------------------------------

MeshGraph::EdgeVertex::EdgeVertex(Edge* edge)
{
	this->edge = edge;
}

/*virtual*/ MeshGraph::EdgeVertex::~EdgeVertex()
{
}

//--------------------------------- EdgeVertexExisting ---------------------------------

MeshGraph::EdgeVertexExisting::EdgeVertexExisting(Edge* edge) : EdgeVertex(edge)
{
	this->vertex_i = -1;
}

/*virtual*/ MeshGraph::EdgeVertexExisting::~EdgeVertexExisting()
{
}

/*virtual*/ const Mesh::Vertex* MeshGraph::EdgeVertexExisting::GetVertex(void)
{
	return this->edge->meshGraph->targetMesh->GetVertex(this->vertex_i);
}

//--------------------------------- EdgeVertexNew ---------------------------------

MeshGraph::EdgeVertexNew::EdgeVertexNew(Edge* edge) : EdgeVertex(edge)
{
}

/*virtual*/ MeshGraph::EdgeVertexNew::~EdgeVertexNew()
{
}

/*virtual*/ const Mesh::Vertex* MeshGraph::EdgeVertexNew::GetVertex(void)
{
	return &this->vertex;
}

//--------------------------------- Edge ---------------------------------

MeshGraph::Edge::Edge(MeshGraph* meshGraph) : GraphElement(meshGraph)
{
	this->adjacentNode[0] = nullptr;
	this->adjacentNode[1] = nullptr;

	this->edgeVertex[0] = nullptr;
	this->edgeVertex[1] = nullptr;
}

/*virtual*/ MeshGraph::Edge::~Edge()
{
	delete this->edgeVertex[0];
	delete this->edgeVertex[1];
}

const Mesh::Vertex* MeshGraph::Edge::GetVertex(int i)
{
	return this->edgeVertex[i % 2]->GetVertex();
}

//--------------------------------- Face ---------------------------------

MeshGraph::Face::Face(Node* node)
{
	this->node = node;
}

/*virtual*/ MeshGraph::Face::~Face()
{
}

/*virtual*/ AxisAlignedBox MeshGraph::Face::CalcBoundingBox() const
{
	AxisAlignedBox box;
	//...
	return box;
}