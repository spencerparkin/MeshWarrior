#include "MeshSetOperation.h"
#include "../Mesh.h"
#include "../Polyline.h"
#include "../Ray.h"
#if MW_DEBUG_DUMP_REFINED_MESHES || MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES || MW_DEBUG_DUMP_CUT_CASE
#	include "../FileFormats/OBJFormat.h"
#endif
#include <set>
#include <assert.h>

using namespace MeshWarrior;

MeshSetOperation::MeshSetOperation(int flags)
{
	this->flags = flags;
	this->faceSet = new std::set<Face*>();
#if MW_DEBUG_USE_STACK_HEAP
	this->faceHeap = new StackHeap<Face>(1024 * 1024);
#else
	this->faceHeap = new TypeHeap<Face>();
#endif
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

	if (meshA == nullptr || meshB == nullptr)
	{
		*this->error = "Both given meshes must be non-null.";
		return false;
	}

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
	MW_ASSERT(totalGuests == this->faceSet->size());

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

		if (newFaceSetA.size() > 0)
		{
			this->faceSet->erase(pair.faceA);
			for (Face* face : newFaceSetA)
				this->faceSet->insert(face);
		}

		if (newFaceSetB.size() > 0)
		{
			this->faceSet->erase(pair.faceB);
			for (Face* face : newFaceSetB)
				this->faceSet->insert(face);
		}

		if (newFaceSetA.size() > 0 || newFaceSetB.size() > 0)
		{
			std::set<Face*> oldFaceSetA, oldFaceSetB;

			iter = collisionPairQueue.begin();
			while (iter != collisionPairQueue.end())
			{
				std::list<CollisionPair>::iterator nextIter = iter;
				nextIter++;

				CollisionPair& existingPair = *iter;
				MW_ASSERT(!(existingPair.faceA == pair.faceA && existingPair.faceB == pair.faceB));

				if (newFaceSetA.size() > 0 && existingPair.faceA == pair.faceA)
				{
					oldFaceSetB.insert(existingPair.faceB);
					collisionPairQueue.erase(iter);
				}

				if (newFaceSetB.size() > 0 && existingPair.faceB == pair.faceB)
				{
					oldFaceSetA.insert(existingPair.faceA);
					collisionPairQueue.erase(iter);
				}

				iter = nextIter;
			}

			if (newFaceSetA.size() > 0)
			{
				this->faceHeap->Deallocate(pair.faceA);
				for (Face* faceA : newFaceSetA)
					for (Face* faceB : oldFaceSetB)
						collisionPairQueue.push_back(CollisionPair(faceA, faceB));
			}

			if (newFaceSetB.size() > 0)
			{
				this->faceHeap->Deallocate(pair.faceB);
				for (Face* faceB : newFaceSetB)
					for (Face* faceA : oldFaceSetA)
						collisionPairQueue.push_back(CollisionPair(faceA, faceB));
			}
		}
	}

	//
	// Note that at this point, there does not have to be any cutting that
	// was performed, and therefore, any cut boundary generated.  In the
	// absense of any cutting, we can still generate a meaningful result.
	// That said, any cut boundaries generated should be line-loops, so let's
	// verify that now.
	//
	
	// TODO: There is a problem here where the cut-boundaries are not coming together
	//       and forming line-loops.  In theory, I believe that if the cutting process
	//       is done correctly (and that by itself is very hard to do), then they
	//       should form line-loops.  That said, this whole approach might not be very
	//       good (e.g., not very numerically stable), because we're making the boundary
	//       detection process dependant on the cutting process.  Rather, the cutting
	//       process should probably be dependant upon an algorithm that performs the
	//       boundary detection.  In any case, I wish there was an easy way to visualize
	//       the cut-boundary being generated.  3DS Max won't do polylines in OBJ format,
	//       but maybe I can just spit out a bunch of degenerate triangles; it seems to
	//       draw those as line segments.

	Polyline::GeneratePolylines(*this->cutBoundarySegmentArray, *this->cutBoundaryPolylineArray);

	/*for (Polyline* polyline : *this->cutBoundaryPolylineArray)
	{
		if (!polyline->IsLineLoop(1e-3))
		{
			*this->error = "Generated cut boundary did not form a line-loop.";
			return false;
		}
	}*/

	// 
	// Bucket sort the chopped-up polygons into their respective meshes.
	//

	polygonArrayA.clear();
	polygonArrayB.clear();

	for (Face* face : *this->faceSet)
	{
		MW_ASSERT(face->family == Face::FAMILY_A || face->family == Face::FAMILY_B);

		if (face->family == Face::FAMILY_A)
			polygonArrayA.push_back(face->polygon);
		else if (face->family == Face::FAMILY_B)
			polygonArrayB.push_back(face->polygon);
	}

	this->refinedMeshA.FromPolygonArray(polygonArrayA);
	this->refinedMeshB.FromPolygonArray(polygonArrayB);

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
	// Now generate a graph for each refined mesh.  This makes it easier for
	// us to traverse over the surface of each refined mesh.  Also, use a
	// polyline for the cut boundary since it's easier to work with.
	//

	this->graphA->Generate(&this->refinedMeshA);
	this->graphB->Generate(&this->refinedMeshB);

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
	// Finally, color the graphs.  That is, in each graph, determine which faces are
	// inside and which are oustide.  The trickiest part is determining the side for
	// an initial face.  Once known, a BFS can be used to walk the graph.  Adjacent
	// faces inherit the side of the face from which they came, unless a cut-boundary
	// is crossed, in which case, we flip from outside to in, or vice-versa.
	//

	if (!this->ColorGraph(this->graphA, nodeList) || !this->ColorGraph(this->graphB, nodeList))
	{
		*this->error = "Failed to color graph.";
		return false;
	}
	
	//
	// Bucket sort the polygons by color (side).  Reverse-wind any inside polygons while we're at it.
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

void MeshSetOperation::ProcessCollisionPair(const CollisionPair& pair, std::set<Face*>& newFaceSetA, std::set<Face*>& newFaceSetB)
{
	newFaceSetA.clear();
	newFaceSetB.clear();

	ConvexPolygon polygonA, polygonB;
	pair.faceA->polygon.ToBasicPolygon(polygonA);
	pair.faceB->polygon.ToBasicPolygon(polygonB);

#if MW_DEBUG_DUMP_CUT_CASE
	Mesh originalA, originalB, splitA, splitB;
	*originalA.name = "original_A";
	*originalB.name = "original_B";
	originalA.AddFace(pair.faceA->polygon);
	originalB.AddFace(pair.faceB->polygon);
	*splitA.name = "split_A";
	*splitB.name = "split_B";
#endif //MW_DEBUG_DUMP_CUT_CASE

	// Do they actually intersect in a non-trivial way?
	Shape* shape = polygonA.IntersectWith(&polygonB);
	if (shape)
	{
		Plane planeA, planeB;
		polygonA.CalcPlane(planeA);
		polygonB.CalcPlane(planeB);

		std::vector<ConvexPolygon> polygonArrayA, polygonArrayB;
		polygonA.SplitAgainstPlane(planeB, polygonArrayA);
		polygonB.SplitAgainstPlane(planeA, polygonArrayB);

		if (polygonArrayA.size() < 2 && polygonArrayB.size() < 2)
			delete shape;
		else
		{
			// Store the intersection for later use.  We'll need it when coloring the graph.
			LineSegment* lineSegment = dynamic_cast<LineSegment*>(shape);
			MW_ASSERT(lineSegment);
			this->cutBoundarySegmentArray->push_back(lineSegment);

			if (polygonArrayA.size() > 1)
			{
				for (ConvexPolygon& newPolygonA : polygonArrayA)
				{
					Face* face = this->faceHeap->Allocate();
					face->family = Face::FAMILY_A;
					face->polygon.FromBasicPolygon(newPolygonA);
					newFaceSetA.insert(face);

#if MW_DEBUG_DUMP_CUT_CASE
					splitA.AddFace(face->polygon);
#endif //MW_DEBUG_DUMP_CUT_CASE
				}
			}

			if (polygonArrayB.size() > 1)
			{
				for (ConvexPolygon& newPolygonB : polygonArrayB)
				{
					Face* face = this->faceHeap->Allocate();
					face->family = Face::FAMILY_B;
					face->polygon.FromBasicPolygon(newPolygonB);
					newFaceSetB.insert(face);

#if MW_DEBUG_DUMP_CUT_CASE
					splitB.AddFace(face->polygon);
#endif //MW_DEBUG_DUMP_CUT_CASE
				}
			}
		}
	}

#if MW_DEBUG_DUMP_CUT_CASE
	std::vector<FileObject*> fileObjectArray;
	fileObjectArray.push_back(&originalA);
	fileObjectArray.push_back(&originalB);
	fileObjectArray.push_back(&splitA);
	fileObjectArray.push_back(&splitB);
	OBJFormat objFormat;
	objFormat.Save("CutCase.OBJ", fileObjectArray);
#endif //MW_DEBUG_DUMP_CUT_CASE
}

bool MeshSetOperation::ColorGraph(Graph* graph, std::list<Graph::Node*>& nodeList)
{
#if MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES
	Mesh outsideMesh, insideMesh;
	*outsideMesh.name = "outside_mesh";
	*insideMesh.name = "inside_mesh";
#endif //MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES

	while (true)
	{
		Graph::Node* rootNode = this->FindRootNodeForColoring(graph->GetTargetMesh(), nodeList);
		if (!rootNode)
			break;

		MW_ASSERT(rootNode->side != Graph::Node::UNKNOWN);

		std::list<Graph::Node*> nodeQueue;
		nodeQueue.push_back(rootNode);

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
#if MW_DEBUG_TRAP_GRAPH_COLORING
					ConvexPolygon debugPolygon;
					adjacentNode->MakePolygon().ToBasicPolygon(debugPolygon);
					Vector debugCenter = debugPolygon.CalcCenter();
					if ((debugCenter - Vector(-4.5936999999999992, 4.9386290987559436, -3.8866872890729072)).Length() < 1e-3)
					{
						node->MakePolygon().ToBasicPolygon(debugPolygon);
						debugCenter = debugPolygon.CalcCenter();
						debugCenter.x = 0.0;
					}
#endif //MW_DEBUG_TRAP_GRAPH_COLORING

					Vector edgePointA = edge->GetVertex(0)->point;
					Vector edgePointB = edge->GetVertex(1)->point;
					Vector edgeMidPoint = (edgePointA + edgePointB) / 2.0;
					bool onCutBoundary = this->PointIsOnCutBoundary(edgeMidPoint);
					if (onCutBoundary)
						adjacentNode->side = node->OppositeSide();
					else
						adjacentNode->side = node->side;

					nodeQueue.push_back(adjacentNode);
				}
			}
		}
	}

#if MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES
	OBJFormat objFormat;
	std::vector<FileObject*> fileObjectArray;
	fileObjectArray.push_back(&insideMesh);
	fileObjectArray.push_back(&outsideMesh);
	if (graph == this->graphA)
		objFormat.Save("DebugMeshA.OBJ", fileObjectArray);
	else if (graph == this->graphB)
		objFormat.Save("DebugMeshB.OBJ", fileObjectArray);
#endif //MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES

	return true;
}

MeshSetOperation::Graph::Node* MeshSetOperation::FindRootNodeForColoring(const Mesh* targetMesh, std::list<Graph::Node*>& nodeList)
{
	// Perform a bunch of ray-casts against the given node-list to find the
	// node that is hit by each ray.  If a ray hits a node for the target
	// mesh, and we don't yet know its color, then return it as an outside
	// node.  If we exhaust all rays, then find the first node of the target
	// mesh without a known color and return it as an inside node.  Note that
	// this algorithm still does not account for all possible cases.  It is still
	// possible for there to exist an outside node that can't be hit by any ray.

	AxisAlignedBox boundingBox;
	for (Graph::Node* node : nodeList)
	{
		Mesh::ConvexPolygon polygon = node->MakePolygon();
		for (Mesh::Vertex& vertex : polygon.vertexArray)
			boundingBox.MinimallyExpandToContainPoint(vertex.point);
	}

	boundingBox.ScaleAboutCenter(1.5);
	Vector center = boundingBox.CalcCenter();
	double radius = boundingBox.CalcRadius();

	int lattitudeCount = 20;
	int longitudeCount = 40;

	Vector xAxis(1.0, 0.0, 0.0);
	Vector yAxis(0.0, 1.0, 0.0);
	Vector zAxis(0.0, 0.0, 1.0);

	for (int i = 0; i <= longitudeCount; i++)
	{
		double theta = (double(i) / double(longitudeCount)) * MW_TWO_PI;
		Vector vectorA = xAxis * cos(theta) + yAxis * sin(theta);

		for (int j = 0; j <= lattitudeCount; j++)
		{
			double phi = (double(j) / double(lattitudeCount)) * MW_PI;
			Vector vectorB = zAxis * cos(phi) + vectorA * sin(phi);

			Vector rayOrigin = center + vectorB * radius;
			Vector rayDirection = center - rayOrigin;
			Ray ray(rayOrigin, rayDirection);

			Graph::Node* node = this->RayCast(ray, nodeList);
			if (node && node->meshGraph->GetTargetMesh() == targetMesh && node->side == Graph::Node::Side::UNKNOWN)
			{
				node->side = Graph::Node::OUTSIDE;
				return node;
			}
		}
	}

	for (Graph::Node* node : nodeList)
	{
		if (node->meshGraph->GetTargetMesh() == targetMesh && node->side == Graph::Node::Side::UNKNOWN)
		{
			node->side = Graph::Node::INSIDE;
			return node;
		}
	}

	return nullptr;
}

MeshSetOperation::Graph::Node* MeshSetOperation::RayCast(const Ray& ray, std::list<Graph::Node*>& nodeList)
{
	double smallestRayAlpha = FLT_MAX;
	Graph::Node* hitNode = nullptr;

	for (Graph::Node* node : nodeList)
	{
		ConvexPolygon polygon;
		node->MakePolygon().ToBasicPolygon(polygon);

		double rayAlpha = 0.0;
		if (polygon.RayCast(ray, rayAlpha))
		{
			if (rayAlpha < smallestRayAlpha)
			{
				smallestRayAlpha = rayAlpha;
				hitNode = node;
			}
		}
	}

	return hitNode;
}

bool MeshSetOperation::PointIsOnCutBoundary(const Vector& point, double eps /*= MW_EPS*/) const
{
	for (int i = 0; i < (int)this->cutBoundaryPolylineArray->size(); i++)
		if ((*this->cutBoundaryPolylineArray)[i]->ContainsPoint(point, 2.0 * eps))	// Why two?  I don't know.
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