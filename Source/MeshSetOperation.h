#pragma once

#include "MeshOperation.h"
#include "BoundingBoxTree.h"
#include "Mesh.h"
#include "Polygon.h"
#include "Polyline.h"
#include "TypeHeap.h"
#include "MeshGraph.h"
#include <set>

#define MW_DEBUG_DUMP_REFINED_MESHES			0
#define MW_DEBUG_DUMP_CUT_BOUNDARY				0
#define MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES		0

namespace MeshWarrior
{
	class LineSegment;
	class Sphere;

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

				Mesh::ConvexPolygon MakePolygon() const;
				Side OppositeSide() const;

				Side side;
			};
		};

		void ProcessMeshes(const Mesh* meshA, const Mesh* meshB);
		void ProcessCollisionPair(const CollisionPair& pair, std::set<Face*>& newFaceSetA, std::set<Face*>& newFaceSetB);
		void ColorGraph(Graph::Node* rootNode);
		bool PointIsOnCutBoundary(const Vector& point, double eps = 1e-6) const;
		Graph::Node* FindOutsideNode(const Mesh* desiredTargetMesh, const Sphere* sphere, const std::list<Graph::Node*>& nodeList);

		std::set<Face*>* faceSet;
		TypeHeap<Face>* faceHeap;
		BoundingBoxTree faceTree;
		Mesh refinedMeshA, refinedMeshB;
		Graph* graphA, *graphB;
		std::vector<LineSegment*>* cutBoundarySegmentArray;
		std::vector<Polyline*>* cutBoundaryPolylineArray;
	};
}