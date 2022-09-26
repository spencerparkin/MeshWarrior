#pragma once

#include "MeshOperation.h"
#include "BoundingBoxTree.h"
#include "Mesh.h"
#include "Polygon.h"
#include "TypeHeap.h"
#include "MeshGraph.h"
#include <set>

namespace MeshWarrior
{
	class LineSegment;

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

			Face();
			virtual ~Face();

			virtual AxisAlignedBox CalcBoundingBox() const override;

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

		class Graph : public MeshGraph
		{
		public:
			Graph();
			virtual ~Graph();

			virtual Node* NodeFactory() override;

			class Node : public MeshGraph::Node
			{
			public:
				Node(MeshGraph* meshGraph);
				virtual ~Node();

				enum Side
				{
					UNKNOWN,
					INSIDE,
					OUTSIDE
				};

				Side OppositeSide() const;

				Side side;
			};
		};

		void ProcessMeshes(const Mesh* meshA, const Mesh* meshB);
		void ProcessCollisionPair(const CollisionPair& pair, std::set<Face*>& newFaceSetA, std::set<Face*>& newFaceSetB);
		void ColorGraph(Graph::Node* rootNode);

		std::set<Face*>* faceSet;
		TypeHeap<Face>* faceHeap;
		BoundingBoxTree faceTree;
		Mesh refinedMeshA, refinedMeshB;
		Graph* graphA, *graphB;
		std::vector<LineSegment*>* cutBoundaryArray;
	};
}