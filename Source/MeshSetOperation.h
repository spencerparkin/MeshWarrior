#pragma once

#include "MeshOperation.h"
#include "BoundingBoxTree.h"
#include "Mesh.h"
#include "Polygon.h"
#include <set>

namespace MeshWarrior
{
	// Note that the algorithm used here won't work with surfaces
	// of certain topologies (e.g., non-orientable surfaces.)  This
	// is not too discouraging, however, because I'm not sure if the
	// set operations make sense in those cases anyway.
	class MESH_WARRIOR_API MeshSetOperation : public MeshOperation
	{
	public:
		MeshSetOperation();
		virtual ~MeshSetOperation();

	protected:

		class Face : public BoundingBoxTree::Guest
		{
		public:
			enum Family
			{
				FAMILY_A,
				FAMILY_B
			};

			Face(Family family);
			Face(Family family, const Mesh::ConvexPolygon& polygon);
			virtual ~Face();

			virtual AxisAlignedBox CalcBoundingBox() const override;

			// Vertex color/texture information, etc., is lost here.
			void ToBasicPolygon(Polygon& polygon) const;
			void FromBasicPolygon(const Polygon& polygon);

			Family family;
			Mesh::ConvexPolygon polygon;
		};

		struct CollisionPair
		{
			CollisionPair(Face* faceA, Face* faceB)
			{
				this->faceA = faceA;
				this->faceB = faceB;
			}

			Face* faceA;
			Face* faceB;
		};

		void ProcessMeshes(const Mesh* meshA, const Mesh* meshB);
		void ProcessCollisionPair(const CollisionPair& pair, std::set<Face*>& newFaceSetA, std::set<Face*>& newFaceSetB);

		std::set<Face*>* faceSet;

		BoundingBoxTree tree;
	};
}