#pragma once

#include "Defines.h"
#include "Vector.h"
#include <vector>
#include <map>
#include <string>

namespace MeshWarrior
{
	class MESH_WARRIOR_API Mesh
	{
		friend class Index;

	public:
		Mesh();
		virtual ~Mesh();

		struct Vertex
		{
			Vector point;
			Vector normal;
			Vector color;
			Vector texCoords;
		};

		class Index
		{
		public:
			Index();
			virtual ~Index();

			int FindOrCreateVertex(const Vertex& vertex, Mesh* mesh);

			// Make sure that this index is still valid WRT to the given mesh.
			// Note that this is not meant to be a fast operation, and should
			// only be done once, if at all, before using the index.
			bool IsValid(const Mesh* mesh) const;

		private:

			std::string MakeKey(const Vertex& vertex) const;

			std::map<std::string, int>* vertexMap;
		};

		// It's assumed that each face consists of a set of coplanar
		// vertices forming a convex polygon wound CCW when viewing
		// the front side of the face.
		struct Face
		{
			std::vector<int> vertexArray;
		};

		// The same assumption is made here as well.
		struct ConvexPolygon
		{
			std::vector<Vertex> vertexArray;
		};

		Vertex* GetVertex(int i);
		const Vertex* GetVertex(int i) const;
		bool SetVertex(int i, const Vertex& vertex);
		bool IsValidVertex(int i) const;

		Face* GetFace(int i);
		const Face* GetFace(int i) const;
		bool SetFace(int i, const Face& face);
		bool IsValidFace(int i) const;

		void Clear();
		int AddVertex(const Vertex& vertex);
		bool AddFace(const Face& face);
		void AddFace(const ConvexPolygon& convexPolygon, Index* index = nullptr, double eps = 1e-6);
		int FindOrCreateVertex(const Vertex& vertex, Index* index = nullptr, double eps = 1e-6);

		void ToPolygonArray(std::vector<ConvexPolygon>& polygonArray) const;
		void FromPolygonArray(const std::vector<ConvexPolygon>& polygonArray);

	private:

		std::vector<Vertex>* vertexArray;
		std::vector<Face>* faceArray;
	};
}