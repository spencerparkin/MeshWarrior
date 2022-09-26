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

			std::vector<Edge*> edgeArray;
			int polygon_i;
		};

		class EdgeVertex
		{
		public:
			EdgeVertex(Edge* edge);
			virtual ~EdgeVertex();

			virtual const Mesh::Vertex* GetVertex(void) = 0;

			Edge* edge;
		};

		class EdgeVertexExisting : public EdgeVertex
		{
		public:
			EdgeVertexExisting(Edge* edge, int i);
			virtual ~EdgeVertexExisting();

			virtual const Mesh::Vertex* GetVertex(void) override;

			int vertex_i;
		};

		class EdgeVertexNew : public EdgeVertex
		{
		public:
			EdgeVertexNew(Edge* edge, const Mesh::Vertex& vertex);
			virtual ~EdgeVertexNew();

			virtual const Mesh::Vertex* GetVertex(void) override;

			Mesh::Vertex vertex;
		};

		class Edge : public GraphElement
		{
		public:
			Edge(MeshGraph* meshGraph);
			virtual ~Edge();

			const Mesh::Vertex* GetVertex(int i);
			Node* GetOtherAdjacency(Node* adjacency);

			Node* adjacentNode[2];
			EdgeVertex* edgeVertex[2];
		};

		class Face : public BoundingBoxTree::Guest
		{
		public:
			Face(Node* node);
			virtual ~Face();

			virtual AxisAlignedBox CalcBoundingBox() const override;

			Node* node;
		};

		bool ForAllElements(std::function<bool(GraphElement*)> iterationFunc);

	private:
		Edge* FindCommonEdge(Node* nodeA, Node* nodeB);

		std::vector<GraphElement*>* graphElementArray;
		const Mesh* targetMesh;
	};
}