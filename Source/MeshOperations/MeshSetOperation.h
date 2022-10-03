#pragma once

#include "../MeshOperation.h"
#include "../BoundingBoxTree.h"
#include "../Mesh.h"
#include "../Polygon.h"
#include "../Polyline.h"
#include "../TypeHeap.h"
#include "../MeshGraph.h"
#include <set>

#define MW_DEBUG_DUMP_REFINED_MESHES			0
#define MW_DEBUG_DUMP_INSIDE_OUTSIDE_MESHES		0
#define MW_DEBUG_DUMP_CUT_BOUNDARY				0
#define MW_DEBUG_USE_STACK_HEAP					1
#define MW_DEBUG_TRAP_GRAPH_COLORING			0
#define MW_DEBUG_DUMP_CUT_CASE					0

#define MW_FLAG_UNION_SET_OP				0x00000001
#define MW_FLAG_INTERSECTION_SETP_OP		0x00000002
#define MW_FLAG_A_MINUS_B_SET_OP			0x00000004
#define MW_FLAG_B_MINUS_A_SET_OP			0x00000008

namespace MeshWarrior
{
	class LineSegment;
	class Sphere;
	class Ray;

	// Note that the algorithm used here won't work with surfaces
	// of certain topologies (e.g., non-orientable surfaces.)  This
	// is not too discouraging, however, because I'm not sure if the
	// set operations make sense in those cases anyway.
	class MESH_WARRIOR_API MeshSetOperation : public MeshOperation
	{
	public:
		MeshSetOperation(int flags);
		virtual ~MeshSetOperation();

	protected:

		int flags;

		virtual bool Calculate(const std::vector<Mesh*>& inputMeshArray, std::vector<Mesh*>& outputMeshArray) override;

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

		void ProcessCollisionPair(const CollisionPair& pair, std::set<Face*>& newFaceSetA, std::set<Face*>& newFaceSetB);
		bool ColorGraph(Graph* graph, std::list<Graph::Node*>& nodeList);
		bool PointIsOnCutBoundary(const Vector& point, double eps = MW_EPS) const;
		Graph::Node* FindRootNodeForColoring(const Mesh* targetMesh, std::list<Graph::Node*>& nodeList);
		Graph::Node* RayCast(const Ray& ray, std::list<Graph::Node*>& nodeList);

		std::set<Face*>* faceSet;
		TypeHeap<Face>* faceHeap;
		BoundingBoxTree faceTree;
		Mesh refinedMeshA, refinedMeshB;
		Graph* graphA, *graphB;
		std::vector<LineSegment*>* cutBoundarySegmentArray;
		std::vector<Polyline*>* cutBoundaryPolylineArray;
	};
}