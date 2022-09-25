#include "MeshSetOperation.h"
#include "Mesh.h"
#include "FileFormats/OBJFormat.h"	// DEBUG INCLUDE
#include <set>
#include <assert.h>

using namespace MeshWarrior;

MeshSetOperation::MeshSetOperation()
{
	this->faceSet = new std::set<Face*>();
	this->faceHeap = new StackHeap<Face>(1024);	// TODO: Use regular heap in release?
}

/*virtual*/ MeshSetOperation::~MeshSetOperation()
{
	for (Face* face : *this->faceSet)
		this->faceHeap->Deallocate(face);

	delete this->faceSet;
	delete this->faceHeap;
}

void MeshSetOperation::ProcessMeshes(const Mesh* meshA, const Mesh* meshB)
{
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

#if false
		Mesh debugMesh;
		debugMesh.AddFace(pair.faceA->polygon);
		debugMesh.AddFace(pair.faceB->polygon);

		OBJFormat objFormat;
		objFormat.SaveMesh("debug.obj", debugMesh);
#endif

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
				faceA->ToBasicPolygon(polygon);
				if (!polygon.IsValid())
					continue;

				for (Face* faceB : oldFaceSetB)
					collisionPairQueue.push_back(CollisionPair(faceA, faceB));
			}

			for (Face* faceB : newFaceSetB)
			{
				ConvexPolygon polygon;
				faceB->ToBasicPolygon(polygon);
				if (!polygon.IsValid())
					continue;

				for (Face* faceA : oldFaceSetA)
					collisionPairQueue.push_back(CollisionPair(faceA, faceB));
			}
		}
	}

	//
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

	Mesh* refinedMeshA = new Mesh();
	Mesh* refinedMeshB = new Mesh();

	refinedMeshA->FromPolygonArray(polygonArrayA);
	refinedMeshB->FromPolygonArray(polygonArrayB);

	// For debugging purposes, dump the refined meshes for examination.
#if true
	OBJFormat objFormat;
	objFormat.SaveMesh("refined_mesh_A.obj", *refinedMeshA);
	objFormat.SaveMesh("refined_mesh_B.obj", *refinedMeshB);
#endif

	// TODO: Now create a mesh graph for the A faces and a mesh graph for the B faces.
	//       Label each node of each graph as "inside" or "outside" while performing
	//       a BFS of each graph.  Knowing when we transition from outside to inside,
	//       or vice-versa, will require data gathered from the cutting process.  Also,
	//       knowing the initial label of the initial node will require some work.
	//       Note that there are two graphs here, not one.  This greatly simplifies
	//       the original thinking on the matter.  Zero or more line-loops should be
	//       generated from the cutting process.  If zero, the situation seems non-
	//       trivial in many cases.  Not a big deal if we require it to be non-zero.
	//       Note that if we can't find the initial outside A face or the initial
	//       outside B face, then it might be fair to say that the operation is ambiguous,
	//       and therefore, cannot be completed.  Add error information.

	// TODO: After determining which faces are inside/outside, maybe reverse winding of all inside faces?
}

void MeshSetOperation::ProcessCollisionPair(const CollisionPair& pair, std::set<Face*>& newFaceSetA, std::set<Face*>& newFaceSetB)
{
	newFaceSetA.clear();
	newFaceSetB.clear();

	ConvexPolygon polygonA, polygonB;
	pair.faceA->ToBasicPolygon(polygonA);
	pair.faceB->ToBasicPolygon(polygonB);

	// Do they actually intersect in a non-trivial way?
	Shape* shape = polygonA.IntersectWith(&polygonB);
	if (shape)
	{
		delete shape; // TODO: No, I think we need to save these for the second phase of the algorithm.

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
			face->FromBasicPolygon(newPolygonA);
			newFaceSetA.insert(face);
		}

		for (ConvexPolygon& newPolygonB : polygonArrayB)
		{
			Face* face = this->faceHeap->Allocate();
			face->family = Face::FAMILY_B;
			face->FromBasicPolygon(newPolygonB);
			newFaceSetB.insert(face);
		}
	}
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

void MeshSetOperation::Face::ToBasicPolygon(Polygon& polygon) const
{
	polygon.vertexArray->clear();
	for (const Mesh::Vertex& vertex : this->polygon.vertexArray)
		polygon.vertexArray->push_back(vertex.point);
}

void MeshSetOperation::Face::FromBasicPolygon(const Polygon& polygon)
{
	Plane plane;
	polygon.CalcPlane(plane);

	this->polygon.vertexArray.clear();
	for (const Vector& point : *polygon.vertexArray)
	{
		Mesh::Vertex vertex;
		vertex.point = point;
		vertex.normal = plane.unitNormal;
		this->polygon.vertexArray.push_back(vertex);
	}
}