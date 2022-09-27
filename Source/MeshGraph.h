#pragma once

#include "Defines.h"
#include "Mesh.h"
#include "BoundingBoxTree.h"
#include <vector>
#include <functional>

namespace MeshWarrior
{
	// This is designed to be meta-data for a given mesh that makes it
	// easier to walk the polygons of that mesh using a BFS, DFS, or whatever.
	// If the target mesh changes out from underneath this graph data-structure,
	// then it should return null when appropriate to do so.
	class MESH_WARRIOR_API MeshGraph
	{
	public:
		MeshGraph();
		virtual ~MeshGraph();
		
		void Generate(const Mesh* mesh);
		void Clear();

		class Edge;
		class Node;

		// Overriders of these can put satilite data in the nodes or edges using derivatives.
		virtual Node* NodeFactory();
		virtual Edge* EdgeFactory();

		class GraphElement
		{
		public:
			GraphElement(MeshGraph* meshGraph);
			virtual ~GraphElement();

			MeshGraph* meshGraph;
		};

		class Node : public GraphElement
		{
		public:
			Node(MeshGraph* meshGraph);
			virtual ~Node();

			bool LinkedWith(const Node* node) const;

			std::vector<Edge*> edgeArray;
			int polygon;
		};

		class Edge : public GraphElement
		{
		public:
			Edge(MeshGraph* meshGraph);
			virtual ~Edge();

			const Mesh::Vertex* GetVertex(int i);
			Node* GetOtherAdjacency(Node* adjacency);
			const Node* GetOtherAdjacency(const Node* adjacency) const;

			Node* adjacentNode[2];
			int vertex[2];
		};

		bool ForAllElements(std::function<bool(GraphElement*)> iterationFunc);

		const Mesh* GetTargetMesh() const { return this->targetMesh; }

	private:

		class Face : public BoundingBoxTree::Guest
		{
		public:
			Face(Node* node);
			virtual ~Face();

			virtual AxisAlignedBox CalcBoundingBox() const override;

			Node* node;
		};

		Edge* FindCommonEdge(Node* nodeA, Node* nodeB);

		std::vector<GraphElement*>* graphElementArray;
		const Mesh* targetMesh;
	};
}