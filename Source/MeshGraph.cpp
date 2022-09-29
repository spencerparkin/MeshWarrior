#include "MeshGraph.h"
#include "Polygon.h"
#include "Shape.h"
#include "Compressor.h"
#include <assert.h>

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
		node->polygon = i;
		tree.AddGuest(new Face(node));
	}

	std::list<BoundingBoxTree::Guest*> guestList;
	tree.GatherAllGuests(guestList);

	for (BoundingBoxTree::Guest* guest : guestList)
	{
		Face* face = (Face*)guest;
		AxisAlignedBox boundingBox = face->CalcBoundingBox();
		
		// Fatten the box so that we're sure to overlap with the bounding
		// boxes of all faces adjacent to this face.
		boundingBox.AddMargin(0.5);
		boundingBox.ScaleAboutCenter(2.0);

		std::list<BoundingBoxTree::Guest*> foundGuestList;
		tree.FindGuests(boundingBox, foundGuestList);

		for (BoundingBoxTree::Guest* foundGuest : foundGuestList)
		{
			Face* foundFace = (Face*)foundGuest;
			if (face == foundFace || face->node->LinkedWith(foundFace->node))
				continue;

			Edge* edge = this->FindCommonEdge(face->node, foundFace->node);
			if (edge)
			{
				face->node->edgeArray.push_back(edge);
				foundFace->node->edgeArray.push_back(edge);

				this->graphElementArray->push_back(edge);
			}
		}
	}
}

MeshGraph::Edge* MeshGraph::FindCommonEdge(Node* nodeA, Node* nodeB)
{
	Edge* edge = nullptr;

	const Mesh::Face* faceA = this->targetMesh->GetFace(nodeA->polygon);
	const Mesh::Face* faceB = this->targetMesh->GetFace(nodeB->polygon);

	ConvexPolygon polygon[2];

	faceA->GeneratePolygon(this->targetMesh).ToBasicPolygon(polygon[0]);
	faceB->GeneratePolygon(this->targetMesh).ToBasicPolygon(polygon[1]);

	std::vector<Point*> pointArray;

	for (int i = 0; i < 2; i++)
	{
		ConvexPolygon* polygonA = &polygon[i];
		ConvexPolygon* polygonB = &polygon[1 - i];

		for (int j = 0; j < (int)polygonA->vertexArray->size(); j++)
		{
			const Vector& vertex = (*polygonA->vertexArray)[j];
			if (polygonB->ContainsPoint(vertex))
				pointArray.push_back(new Point(vertex));
		}
	}

	// Remove redundant points to see how many there really are.
	CompressArray<Point>(pointArray, [](const Point* pointA, const Point* pointB) -> Point* {
		if (pointA->ContainsPoint(pointB->center))
		{
			delete pointB;
			return const_cast<Point*>(pointA);
		}
		return nullptr;
	});

	// Do we have a shared edge?
	if (pointArray.size() == 2)
	{
		edge = new Edge(this);

		edge->adjacentNode[0] = nodeA;
		edge->adjacentNode[1] = nodeB;

		edge->vertex[0] = this->targetMesh->FindVertex(pointArray[0]->center);
		edge->vertex[1] = this->targetMesh->FindVertex(pointArray[1]->center);
	}

	for (Point* point : pointArray)
		delete point;

	return edge;
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

bool MeshGraph::ForAllElements(std::function<bool(GraphElement*)> iterationFunc)
{
	for (GraphElement* element : *this->graphElementArray)
		if (iterationFunc(element))
			return true;

	return false;
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
	this->polygon = -1;
}

/*virtual*/ MeshGraph::Node::~Node()
{
}

bool MeshGraph::Node::LinkedWith(const Node* node) const
{
	for (int i = 0; i < (int)this->edgeArray.size(); i++)
		if (this->edgeArray[i]->GetOtherAdjacency(this) == node)
			return true;

	return false;
}

//--------------------------------- Edge ---------------------------------

MeshGraph::Edge::Edge(MeshGraph* meshGraph) : GraphElement(meshGraph)
{
	this->adjacentNode[0] = nullptr;
	this->adjacentNode[1] = nullptr;

	this->vertex[0] = -1;
	this->vertex[1] = -1;
}

/*virtual*/ MeshGraph::Edge::~Edge()
{
}

const Mesh::Vertex* MeshGraph::Edge::GetVertex(int i)
{
	return this->meshGraph->targetMesh->GetVertex(this->vertex[i % 2]);
}

MeshGraph::Node* MeshGraph::Edge::GetOtherAdjacency(Node* adjacency)
{
	if (this->adjacentNode[0] == adjacency)
		return this->adjacentNode[1];
	else if (this->adjacentNode[1] == adjacency)
		return this->adjacentNode[0];
	else
		return nullptr;
}

const MeshGraph::Node* MeshGraph::Edge::GetOtherAdjacency(const Node* adjacency) const
{
	return const_cast<Edge*>(this)->GetOtherAdjacency(const_cast<Node*>(adjacency));
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
	
	const Mesh::Face* face = this->node->meshGraph->targetMesh->GetFace(this->node->polygon);
	MW_ASSERT(face);
	Mesh::ConvexPolygon polygon = face->GeneratePolygon(this->node->meshGraph->targetMesh);
	for (int i = 0; i < (int)polygon.vertexArray.size(); i++)
		box.MinimallyExpandToContainPoint(polygon.vertexArray[i].point);

	return box;
}