#include "MeshSetOperation.h"
#include "../Mesh.h"
#include "../Polyline.h"
#if MW_DEBUG_DUMP_REFINED_MESHES || MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES
#	include "FileFormats/OBJFormat.h"
#endif
#include <set>
#include <assert.h>

using namespace MeshWarrior;

MeshSetOperation::MeshSetOperation(int flags)
{
	this->flags = flags;
	this->faceSet = new std::set<Face*>();
	this->faceHeap = new StackHeap<Face>(1024);	// TODO: Use regular heap in release?
	this->cutBoundarySegmentArray = new std::vector<LineSegment*>();
	this->cutBoundaryPolylineArray = new std::vector<Polyline*>();
	this->graphA = new Graph();
	this->graphB = new Graph();
}

/*virtual*/ MeshSetOperation::~MeshSetOperation()
{
	for (Face* face : *this->faceSet)
		this->faceHeap->Deallocate(face);

	delete this->faceSet;
	delete this->faceHeap;

	for (LineSegment* lineSegment : *this->cutBoundarySegmentArray)
		delete lineSegment;

	for (Polyline* polyline : *this->cutBoundaryPolylineArray)
		delete polyline;

	delete this->cutBoundarySegmentArray;
	delete this->cutBoundaryPolylineArray;

	this->graphA;
	this->graphB;
}

/*virtual*/ bool MeshSetOperation::Calculate(const std::vector<Mesh*>& inputMeshArray, std::vector<Mesh*>& outputMeshArray)
{
	*this->error = "";

	//
	// Make sure we were given the right number of arguments, and given some flags.
	//

	if (inputMeshArray.size() != 2)
	{
		*this->error = "The mesh set operation needs exactly two meshes as input.";
		return false;
	}

	if (this->flags == 0)
	{
		*this->error = "No flags given for mesh set operation.";
		return false;
	}

	const Mesh* meshA = inputMeshArray[0];
	const Mesh* meshB = inputMeshArray[1];

	//
	// Throw all the polygons from each mesh into a single set.
	// Label each polygon so that we can continue to differentiate between them.
	//

	std::vector<Mesh::ConvexPolygon> polygonArrayA, polygonArrayB;

	meshA->ToPolygonArray(polygonArrayA);
	meshB->ToPolygonArray(polygonArrayB);

	for (Mesh::ConvexPolygon& polygon : polygonArrayA)
	{
		Face* face = this->faceHeap->Allocate();
		face->family = Face::FAMILY_A;
		face->polygon = polygon;
		this->faceSet->insert(face);
	}

	for (Mesh::ConvexPolygon& polygon : polygonArrayB)
	{
		Face* face = this->faceHeap->Allocate();
		face->family = Face::FAMILY_B;
		face->polygon = polygon;
		this->faceSet->insert(face);
	}

	//
	// Throw all the faces into a spacial sorting data-structure.
	// This one isn't very good, but it's better than nothing.
	//

	AxisAlignedBox rootBox;

	for (Face* face : *this->faceSet)
		rootBox.MinimallyExpandToContainBox(face->CalcBoundingBox());

	this->faceTree.Clear();
	this->faceTree.SetRootBox(rootBox);

	for (Face* face : *this->faceSet)
		this->faceTree.AddGuest(face);

	int totalGuests = this->faceTree.TotalGuests();
	assert(totalGuests == this->faceSet->size());

	//
	// Find all the initial collision pairs.
	//

	std::list<CollisionPair> collisionPairQueue;
	for (Face* faceA : *this->faceSet)
	{
		if (faceA->family == Face::FAMILY_A)
		{
			std::list<BoundingBoxTree::Guest*> guestList;
			this->faceTree.FindGuests(faceA->CalcBoundingBox(), guestList);

			for (BoundingBoxTree::Guest* guest : guestList)
			{
				Face* faceB = (Face*)guest;
				if (faceB->family == Face::FAMILY_B)
				{
					CollisionPair pair(faceA, faceB);
					collisionPairQueue.push_back(pair);
				}
			}
		}
	}

	//
	// Process the collision pair queue, cutting polygons up, until it's empty.
	// Proper termination of this algorithm depends on the correctness of the cutting algorithm.
	//

	while (collisionPairQueue.size() > 0)
	{
		std::list<CollisionPair>::iterator iter = collisionPairQueue.begin();
		CollisionPair pair = *iter;
		collisionPairQueue.erase(iter);

		std::set<Face*> newFaceSetA, newFaceSetB;
		this->ProcessCollisionPair(pair, newFaceSetA, newFaceSetB);

		if (newFaceSetA.size() > 0 || newFaceSetB.size() > 0)
		{
			this->faceSet->erase(pair.faceA);
			this->faceSet->erase(pair.faceB);

			for (Face* face : newFaceSetA)
				this->faceSet->insert(face);

			for (Face* face : newFaceSetB)
				this->faceSet->insert(face);

			std::set<Face*> oldFaceSetA, oldFaceSetB;

			iter = collisionPairQueue.begin();
			while (iter != collisionPairQueue.end())
			{
				std::list<CollisionPair>::iterator nextIter = iter;
				nextIter++;

				CollisionPair& existingPair = *iter;

				if (existingPair.faceA == pair.faceA && existingPair.faceB == pair.faceB)
					collisionPairQueue.erase(iter);		// This shouldn't happen, but account for it anyway.
				else if (existingPair.faceA == pair.faceA)
				{
					oldFaceSetB.insert(existingPair.faceB);
					collisionPairQueue.erase(iter);
				}
				else if (existingPair.faceB == pair.faceB)
				{
					oldFaceSetA.insert(existingPair.faceA);
					collisionPairQueue.erase(iter);
				}

				iter = nextIter;
			}

			this->faceHeap->Deallocate(pair.faceA);
			this->faceHeap->Deallocate(pair.faceB);

			// Note that rather than skipping invalid polygons here, we really
			// should just never be producing any invalid polygons.

			for (Face* faceA : newFaceSetA)
			{
				ConvexPolygon polygon;
				faceA->polygon.ToBasicPolygon(polygon);
				if (!polygon.IsValid(1e-4))
					continue;

				for (Face* faceB : oldFaceSetB)
					collisionPairQueue.push_back(CollisionPair(faceA, faceB));
			}

			for (Face* faceB : newFaceSetB)
			{
				ConvexPolygon polygon;
				faceB->polygon.ToBasicPolygon(polygon);
				if (!polygon.IsValid(1e-4))
					continue;

				for (Face* faceA : oldFaceSetA)
					collisionPairQueue.push_back(CollisionPair(faceA, faceB));
			}
		}
	}

	//
	// Note that at this point, there does not have to be any cutting that
	// was performed, and therefore, any cut boundary generated.  In the
	// absense of any cutting, we can still generate a meaningful result.
	// Bucket sort the chopped-up polygons into their respective meshes.
	//

	polygonArrayA.clear();
	polygonArrayB.clear();

	for (Face* face : *this->faceSet)
	{
		if (face->family == Face::FAMILY_A)
			polygonArrayA.push_back(face->polygon);
		else if (face->family == Face::FAMILY_B)
			polygonArrayB.push_back(face->polygon);
	}

	this->refinedMeshA.FromPolygonArray(polygonArrayA);
	this->refinedMeshB.FromPolygonArray(polygonArrayB);

	//
	// Now generate a graph for each refined mesh.  This makes it easier for
	// us to traverse over the surface of each refined mesh.  Also, use a
	// polyline for the cut boundary since it's easier to work with.
	//

	this->graphA->Generate(&this->refinedMeshA);
	this->graphB->Generate(&this->refinedMeshB);

	// Do these need to be line-loops in all cases?
	Polyline::GeneratePolylines(*this->cutBoundarySegmentArray, *this->cutBoundaryPolylineArray);

#if MW_DEBUG_DUMP_REFINED_MESHES
	*refinedMeshA.name = "refined_mesh_A";
	*refinedMeshB.name = "refined_mesh_B";

	OBJFormat objFormat;

	std::vector<FileObject*> fileObjectArray;
	fileObjectArray.push_back(&refinedMeshA);
	fileObjectArray.push_back(&refinedMeshB);

#if MW_DEBUG_DUMP_CUT_BOUNDARY
	for (Polyline* polyline : *this->cutBoundaryPolylineArray)
		fileObjectArray.push_back(polyline);
#endif //MW_DEBUG_DUMP_CUT_BOUNDARY

	objFormat.Save("refined_meshes.obj", fileObjectArray);
#endif //MW_DEBUG_DUMP_REFINED_MESHES

	//
	// Gather the nodes from both graphs into one list.
	//

	std::list<Graph::Node*> nodeList;

	this->graphA->ForAllElements([&nodeList](MeshGraph::GraphElement* element) -> bool {
		Graph::Node* node = dynamic_cast<Graph::Node*>(element);
		if (node)
			nodeList.push_back(node);
		return false;
	});

	this->graphB->ForAllElements([&nodeList](MeshGraph::GraphElement* element) -> bool {
		Graph::Node* node = dynamic_cast<Graph::Node*>(element);
		if (node)
			nodeList.push_back(node);
		return false;
	});

	//
	// Find a face on refined mesh A that we believe to be on the outside.
	// Do the same for refined mesh B.  If none is found for either, then
	// we're in trouble.  However, if none is found for just one, then we
	// will conclude (approximately) that one mesh entirely envelops the other.
	//

	rootBox.ScaleAboutCenter(2.0);
	Sphere sphere;
	sphere.center = rootBox.CalcCenter();
	sphere.radius = rootBox.CalcRadius();

	Graph::Node* initialNodeA = this->FindOutsideNode(&this->refinedMeshA, &sphere, nodeList);
	Graph::Node* initialNodeB = this->FindOutsideNode(&this->refinedMeshB, &sphere, nodeList);
	if (initialNodeA && initialNodeB)
	{
		initialNodeA->side = Graph::Node::OUTSIDE;
		initialNodeB->side = Graph::Node::OUTSIDE;
	}
	else if (initialNodeA)
	{
		initialNodeA->side = Graph::Node::OUTSIDE;
		initialNodeB = this->FindAnyNode(&this->refinedMeshB, nodeList);
		if (initialNodeB)
			initialNodeB->side = Graph::Node::INSIDE;
	}
	else if(initialNodeB)
	{
		initialNodeB->side = Graph::Node::OUTSIDE;
		initialNodeA = this->FindAnyNode(&this->refinedMeshA, nodeList);
		if (initialNodeA)
			initialNodeA->side = Graph::Node::INSIDE;
	}

	if (!initialNodeA || !initialNodeB)
	{
		*this->error = "Failed to find initial graph nodes for graph coloring.";
		return false;
	}

	//
	// Finally, color the graphs.  That is, determine whether each node is inside or outside.
	// We do this with a BFS starting from a node for whom's side we do initially know.  Then,
	// whenever we cross a cut boundary, we switch from outside to inside, or vise-versa.
	//

	this->ColorGraph(initialNodeA);
	this->ColorGraph(initialNodeB);
	
	//
	// Bucket sort the polygons.  Reverse-wind any inside polygons at the same time.
	//

	std::vector<Mesh::ConvexPolygon> outsidePolygonArrayA, outsidePolygonArrayB;
	std::vector<Mesh::ConvexPolygon> insidePolygonArrayA, insidePolygonArrayB;

	this->graphA->ForAllElements([&outsidePolygonArrayA, &insidePolygonArrayA](MeshGraph::GraphElement* element) -> bool {
		Graph::Node* node = dynamic_cast<Graph::Node*>(element);
		if (node)
		{
			if (node->side == Graph::Node::OUTSIDE)
				outsidePolygonArrayA.push_back(node->MakePolygon());
			else if (node->side == Graph::Node::INSIDE)
				insidePolygonArrayA.push_back(node->MakePolygon().ReverseWinding());
		}
		return false;
	});

	this->graphB->ForAllElements([&outsidePolygonArrayB, &insidePolygonArrayB](MeshGraph::GraphElement* element) -> bool {
		Graph::Node* node = dynamic_cast<Graph::Node*>(element);
		if (node)
		{
			if (node->side == Graph::Node::OUTSIDE)
				outsidePolygonArrayB.push_back(node->MakePolygon());
			else if (node->side == Graph::Node::INSIDE)
				insidePolygonArrayB.push_back(node->MakePolygon().ReverseWinding());
		}
		return false;
	});

	//
	// Lastly, form the results called-for by the given flags.
	//

	outputMeshArray.clear();

	if ((this->flags & MW_FLAG_UNION_SET_OP) != 0)
	{
		Mesh* mesh = new Mesh();
		*mesh->name = "union";
		for (Mesh::ConvexPolygon& polygon : outsidePolygonArrayA)
			mesh->AddFace(polygon);
		for (Mesh::ConvexPolygon& polygon : outsidePolygonArrayB)
			mesh->AddFace(polygon);
		outputMeshArray.push_back(mesh);
	}

	if ((this->flags & MW_FLAG_INTERSECTION_SETP_OP) != 0)
	{
		Mesh* mesh = new Mesh();
		*mesh->name = "intersection";
		for (Mesh::ConvexPolygon& polygon : insidePolygonArrayA)
			mesh->AddFace(polygon);
		for (Mesh::ConvexPolygon& polygon : insidePolygonArrayB)
			mesh->AddFace(polygon);
		outputMeshArray.push_back(mesh);
	}

	if ((this->flags & MW_FLAG_A_MINUS_B_SET_OP) != 0)
	{
		Mesh* mesh = new Mesh();
		*mesh->name = "a_minus_b";
		for (Mesh::ConvexPolygon& polygon : outsidePolygonArrayA)
			mesh->AddFace(polygon);
		for (Mesh::ConvexPolygon& polygon : insidePolygonArrayB)
			mesh->AddFace(polygon);
		outputMeshArray.push_back(mesh);
	}

	if ((this->flags & MW_FLAG_B_MINUS_A_SET_OP) != 0)
	{
		Mesh* mesh = new Mesh();
		*mesh->name = "b_minus_a";
		for (Mesh::ConvexPolygon& polygon : outsidePolygonArrayB)
			mesh->AddFace(polygon);
		for (Mesh::ConvexPolygon& polygon : insidePolygonArrayA)
			mesh->AddFace(polygon);
		outputMeshArray.push_back(mesh);
	}

	return true;
}

MeshSetOperation::Graph::Node* MeshSetOperation::FindOutsideNode(const Mesh* desiredTargetMesh, const Sphere* sphere, const std::list<Graph::Node*>& nodeList)
{
	Graph::Node* foundNode = nullptr;

	// Plan to "touch" the node collection from several different perspectives.
	std::list<Plane*> planeList;
	for (double x = -2.0; x <= 2.0; x += 1.0)
	{
		for (double y = -2.0; y <= 2.0; y += 1.0)
		{
			for (double z = -2.0; z <= 2.0; z += 1.0)
			{
				Vector vector(x, y, z);
				if (::fabs(vector.x) == 2.0 || ::fabs(vector.y) == 2.0 || ::fabs(vector.z) == 2.0)
				{
					vector.Normalize();
					Plane* plane = new Plane(sphere->center + vector * sphere->radius, vector);
					planeList.push_back(plane);
				}
			}
		}
	}

	for (Plane* touchPlane : planeList)
	{
		// Find a node that "touches" the plane.
		Graph::Node* closestNode = nullptr;
		double smallestDistance = DBL_MAX;
		for (Graph::Node* node : nodeList)
		{
			const Mesh* targetMesh = node->meshGraph->GetTargetMesh();
			if (targetMesh != desiredTargetMesh)
				continue;

			ConvexPolygon polygon;
			targetMesh->GetFace(node->polygon)->GeneratePolygon(targetMesh).ToBasicPolygon(polygon);
			Vector center = polygon.CalcCenter();
			double distance = ::fabs(touchPlane->ShortestSignedDistanceToPoint(center));
			if (distance < smallestDistance)
			{
				smallestDistance = distance;
				closestNode = node;
			}
		}

		if (closestNode)
		{
			// Tentative declare that we've found the node we're looking for.
			foundNode = closestNode;

			// Does the plane of the node contain all other nodes on or behind it?
			ConvexPolygon polygon;
			const Mesh* targetMesh = closestNode->meshGraph->GetTargetMesh();
			targetMesh->GetFace(closestNode->polygon)->GeneratePolygon(targetMesh).ToBasicPolygon(polygon);
			Plane polygonPlane;
			polygon.CalcPlane(polygonPlane);
			for (Graph::Node* node : nodeList)
			{
				const Mesh* targetMesh = node->meshGraph->GetTargetMesh();
				targetMesh->GetFace(node->polygon)->GeneratePolygon(targetMesh).ToBasicPolygon(polygon);
				Vector center = polygon.CalcCenter();
				double distance = polygonPlane.ShortestSignedDistanceToPoint(center);
				if (distance > 0.0 && !polygonPlane.ContainsPoint(center))
				{
					foundNode = nullptr;
					break;
				}
			}

			if (foundNode)
				break;
		}
	}

	for (Plane* plane : planeList)
		delete plane;

	return foundNode;
}

MeshSetOperation::Graph::Node* MeshSetOperation::FindAnyNode(const Mesh* desiredTargetMesh, const std::list<Graph::Node*>& nodeList)
{
	for (Graph::Node* node : nodeList)
		if (node->meshGraph->GetTargetMesh() == desiredTargetMesh)
			return node;

	return nullptr;
}

void MeshSetOperation::ProcessCollisionPair(const CollisionPair& pair, std::set<Face*>& newFaceSetA, std::set<Face*>& newFaceSetB)
{
	newFaceSetA.clear();
	newFaceSetB.clear();

	ConvexPolygon polygonA, polygonB;
	pair.faceA->polygon.ToBasicPolygon(polygonA);
	pair.faceB->polygon.ToBasicPolygon(polygonB);

	// Do they actually intersect in a non-trivial way?
	Shape* shape = polygonA.IntersectWith(&polygonB);
	if (shape)
	{
		// Store the intersection for later use.  We'll need it when coloring the graph.
		LineSegment* lineSegment = dynamic_cast<LineSegment*>(shape);
		assert(lineSegment);
		this->cutBoundarySegmentArray->push_back(lineSegment);

		Plane planeA, planeB;
		polygonA.CalcPlane(planeA);
		polygonB.CalcPlane(planeB);

		std::vector<ConvexPolygon> polygonArrayA, polygonArrayB;
		polygonA.SplitAgainstPlane(planeB, polygonArrayA);
		polygonB.SplitAgainstPlane(planeA, polygonArrayB);

		for (ConvexPolygon& newPolygonA : polygonArrayA)
		{
			Face* face = this->faceHeap->Allocate();
			face->family = Face::FAMILY_A;
			face->polygon.FromBasicPolygon(newPolygonA);
			newFaceSetA.insert(face);
		}

		for (ConvexPolygon& newPolygonB : polygonArrayB)
		{
			Face* face = this->faceHeap->Allocate();
			face->family = Face::FAMILY_B;
			face->polygon.FromBasicPolygon(newPolygonB);
			newFaceSetB.insert(face);
		}
	}
}

void MeshSetOperation::ColorGraph(Graph::Node* rootNode)
{
	assert(rootNode->side != Graph::Node::UNKNOWN);

	std::list<Graph::Node*> nodeQueue;
	nodeQueue.push_back(rootNode);

#if MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES
	Mesh outsideMesh, insideMesh;
#endif //MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES

	while (nodeQueue.size() > 0)
	{
		std::list<Graph::Node*>::iterator iter = nodeQueue.begin();
		Graph::Node* node = *iter;
		nodeQueue.erase(iter);

#if MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES
		Mesh::ConvexPolygon polygon = node->MakePolygon();
		if (node->side == Graph::Node::INSIDE)
			insideMesh.AddFace(polygon);
		else if (node->side == Graph::Node::OUTSIDE)
			outsideMesh.AddFace(polygon);
#endif //MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES

		for (int i = 0; i < (int)node->edgeArray.size(); i++)
		{
			Graph::Edge* edge = node->edgeArray[i];
			Graph::Node* adjacentNode = (Graph::Node*)edge->GetOtherAdjacency(node);
			if (adjacentNode->side == Graph::Node::UNKNOWN)
			{
				bool cut0 = this->PointIsOnCutBoundary(edge->GetVertex(0)->point);
				bool cut1 = this->PointIsOnCutBoundary(edge->GetVertex(1)->point);

				if (cut0 && cut1)
					adjacentNode->side = node->OppositeSide();
				else
					adjacentNode->side = node->side;

				nodeQueue.push_back(adjacentNode);
			}
		}
	}

#if MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES
	OBJFormat objFormat;
	std::vector<FileObject*> fileObjectArray;
	fileObjectArray.push_back(&insideMesh);
	fileObjectArray.push_back(&outsideMesh);
	if (rootNode->meshGraph == this->graphA)
		objFormat.Save("DebugMeshA.OBJ", fileObjectArray);
	else if (rootNode->meshGraph == this->graphB)
		objFormat.Save("DebugMeshB.OBJ", fileObjectArray);
#endif
}

bool MeshSetOperation::PointIsOnCutBoundary(const Vector& point, double eps /*= 1e-6*/) const
{
	for (int i = 0; i < (int)this->cutBoundaryPolylineArray->size(); i++)
		if ((*this->cutBoundaryPolylineArray)[i]->HasVertex(point, eps))
			return true;

	return false;
}

MeshSetOperation::Face::Face()
{
}

/*virtual*/ MeshSetOperation::Face::~Face()
{
}

/*virtual*/ AxisAlignedBox MeshSetOperation::Face::CalcBoundingBox() const
{
	AxisAlignedBox box;

	for (int i = 0; i < (signed)this->polygon.vertexArray.size(); i++)
		box.MinimallyExpandToContainPoint(this->polygon.vertexArray[i].point);

	return box;
}

MeshSetOperation::Graph::Graph()
{
}

/*virtual*/ MeshSetOperation::Graph::~Graph()
{
}

/*virtual*/ MeshGraph::Node* MeshSetOperation::Graph::NodeFactory()
{
	return new MeshSetOperation::Graph::Node(this);
}

MeshSetOperation::Graph::Node::Node(MeshGraph* meshGraph) : MeshGraph::Node(meshGraph)
{
	this->side = Side::UNKNOWN;
}

/*virtual*/ MeshSetOperation::Graph::Node::~Node()
{
}

MeshSetOperation::Graph::Node::Side MeshSetOperation::Graph::Node::OppositeSide() const
{
	if (this->side == Side::OUTSIDE)
		return Side::INSIDE;
	else if (this->side == Side::INSIDE)
		return Side::OUTSIDE;

	return Side::UNKNOWN;
}

Mesh::ConvexPolygon MeshSetOperation::Graph::Node::MakePolygon() const
{
	const Mesh* targetMesh = this->meshGraph->GetTargetMesh();
	return targetMesh->GetFace(this->polygon)->GeneratePolygon(targetMesh);
}