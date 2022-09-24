#include "MeshSetOperation.h"
#include "Mesh.h"
#include "FileFormats/OBJFormat.h"	// DEBUG INCLUDE
#include <set>

using namespace MeshWarrior;

MeshSetOperation::MeshSetOperation()
{
	this->faceSet = new std::set<Face*>();
}

/*virtual*/ MeshSetOperation::~MeshSetOperation()
{
	for (Face* face : *this->faceSet)
		delete face;

	delete this->faceSet;
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
		this->faceSet->insert(new Face(Face::FAMILY_A, polygon));

	for (Mesh::ConvexPolygon& polygon : polygonArrayB)
		this->faceSet->insert(new Face(Face::FAMILY_B, polygon));

	//
	// Throw all the faces into a spacial sorting data-structure.
	// This one isn't very good, but it's better than nothing.
	//

	AxisAlignedBox rootBox;

	for (Face* face : *this->faceSet)
		rootBox.MinimallyExpandToContainBox(face->CalcBoundingBox());

	this->tree.Clear();
	this->tree.SetRootBox(rootBox);

	for (Face* face : *this->faceSet)
		this->tree.AddGuest(face);

	//
	// Find all the initial collision pairs.
	//

	std::list<CollisionPair> collisionPairQueue;
	for (Face* faceA : *this->faceSet)
	{
		if (faceA->family == Face::FAMILY_A)
		{
			std::list<BoundingBoxTree::Guest*> guestList;
			this->tree.FindGuests(faceA->CalcBoundingBox(), guestList);

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

				if (existingPair.faceA == pair.faceA)
				{
					oldFaceSetB.insert(pair.faceB);
					collisionPairQueue.erase(iter);
				}

				if (existingPair.faceB == pair.faceB)
				{
					oldFaceSetA.insert(pair.faceA);
					collisionPairQueue.erase(iter);
				}

				iter = nextIter;
			}

			delete pair.faceA;
			delete pair.faceB;

			for (Face* faceA : newFaceSetA)
				for (Face* faceB : oldFaceSetB)
					collisionPairQueue.push_back(CollisionPair(faceA, faceB));

			for (Face* faceA : oldFaceSetA)
				for (Face* faceB : newFaceSetB)
					collisionPairQueue.push_back(CollisionPair(faceA, faceB));
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
			Face* face = new Face(Face::FAMILY_A);
			face->FromBasicPolygon(newPolygonA);
			newFaceSetA.insert(face);
		}

		for (ConvexPolygon& newPolygonB : polygonArrayB)
		{
			Face* face = new Face(Face::FAMILY_B);
			face->FromBasicPolygon(newPolygonB);
			newFaceSetB.insert(face);
		}
	}
}

MeshSetOperation::Face::Face(Family family)
{
	this->family = family;
}

MeshSetOperation::Face::Face(Family family, const Mesh::ConvexPolygon& polygon)
{
	this->family = family;
	this->polygon = polygon;
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