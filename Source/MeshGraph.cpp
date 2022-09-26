#include "MeshGraph.h"
#include "Polygon.h"
#include "Shape.h"
#include "Compressor.h"

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
				this->graphElementArray->push_back(edge);
			}
		}
	}
}

MeshGraph::Edge* MeshGraph::FindCommonEdge(Node* nodeA, Node* nodeB)
{
	Edge* edge = nullptr;

	const Mesh::Face* faceA = this->targetMesh->GetFace(nodeA->polygon_i);
	const Mesh::Face* faceB = this->targetMesh->GetFace(nodeB->polygon_i);

	ConvexPolygon polygon[2];

	faceA->GeneratePolygon(this->targetMesh).ToBasicPolygon(polygon[0]);
	faceB->GeneratePolygon(this->targetMesh).ToBasicPolygon(polygon[1]);

	std::vector<Point*> pointArray;

	for (int i = 0; i < 2; i++)
	{
		ConvexPolygon* polygonA = &polygon[i];
		ConvexPolygon* polygonB = &polygon[1 - i];

		for (int j = 0; j < (int)polygonA->vertexArray->size(); i++)
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

		Mesh::Vertex vertexA, vertexB;
		vertexA.point = pointArray[0]->center;
		vertexB.point = pointArray[1]->center;

		int vertexA_i = this->targetMesh->FindVertex(vertexA);
		int vertexB_i = this->targetMesh->FindVertex(vertexB);

		if (this->targetMesh->IsValidVertex(vertexA_i))
			edge->edgeVertex[0] = new EdgeVertexExisting(edge, vertexA_i);
		else
			edge->edgeVertex[0] = new EdgeVertexNew(edge, vertexA);

		if (this->targetMesh->IsValidVertex(vertexB_i))
			edge->edgeVertex[1] = new EdgeVertexExisting(edge, vertexB_i);
		else
			edge->edgeVertex[1] = new EdgeVertexNew(edge, vertexB);
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

MeshGraph::EdgeVertexExisting::EdgeVertexExisting(Edge* edge, int i) : EdgeVertex(edge)
{
	this->vertex_i = i;
}

/*virtual*/ MeshGraph::EdgeVertexExisting::~EdgeVertexExisting()
{
}

/*virtual*/ const Mesh::Vertex* MeshGraph::EdgeVertexExisting::GetVertex(void)
{
	return this->edge->meshGraph->targetMesh->GetVertex(this->vertex_i);
}

//--------------------------------- EdgeVertexNew ---------------------------------

MeshGraph::EdgeVertexNew::EdgeVertexNew(Edge* edge, const Mesh::Vertex& vertex) : EdgeVertex(edge)
{
	this->vertex = vertex;
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