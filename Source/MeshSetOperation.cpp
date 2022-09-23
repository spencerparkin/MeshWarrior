#include "MeshSetOperation.h"
#include "Mesh.h"
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

	// TODO: Just dump out all the polygons at this point so we can debug the result.

	// TODO: Now create a mesh graph for the A faces and a mesh graph for the B faces.
	//       Label each node of each graph as "inside" or "outside" while performing
	//       a BFS of each graph.  Knowing when we transition from outside to inside,
	//       or vice-versa, will require data gathered from the cutting process.  Also,
	//       knowing the initial label of the initial node will require some work.
	//       Note that there are two graphs here, not one.  This greatly simplifies
	//       the original thinking on the matter.
}

void MeshSetOperation::ProcessCollisionPair(const CollisionPair& pair, std::set<Face*>& newFaceSetA, std::set<Face*>& newFaceSetB)
{
	newFaceSetA.clear();
	newFaceSetB.clear();

	//...
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