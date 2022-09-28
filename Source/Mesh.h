#pragma once

#include "FileObject.h"
#include "Vector.h"
#include "AxisAlignedBox.h"
#include <vector>
#include <map>
#include <string>

namespace MeshWarrior
{
	class ConvexPolygon;

	// TODO: There should be a way to transform meshes and a way to fixup/calculate normals and UV coordinates.
	class MESH_WARRIOR_API Mesh : public FileObject
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

		struct ConvexPolygon;

		// It's assumed that each face consists of a set of coplanar
		// vertices forming a convex polygon wound CCW when viewing
		// the front side of the face.
		struct Face
		{
			std::vector<int> vertexArray;

			ConvexPolygon GeneratePolygon(const Mesh* mesh) const;
		};

		// The same assumption is made here as well.
		struct ConvexPolygon
		{
			// Vertex color/texture information, etc., is lost here.
			void ToBasicPolygon(MeshWarrior::ConvexPolygon& polygon) const;
			void FromBasicPolygon(const MeshWarrior::ConvexPolygon& polygon);

			bool HasVertex(const Vector& point, double eps = MW_EPS) const;

			ConvexPolygon& ReverseWinding();

			std::vector<Vertex> vertexArray;
		};

		Vertex* GetVertex(int i);
		const Vertex* GetVertex(int i) const;
		bool SetVertex(int i, const Vertex& vertex);
		bool IsValidVertex(int i) const;
		int GetNumVertices() const;

		Face* GetFace(int i);
		const Face* GetFace(int i) const;
		bool SetFace(int i, const Face& face);
		bool IsValidFace(int i) const;
		int GetNumFaces() const;

		void Clear();
		int AddVertex(const Vertex& vertex);
		bool AddFace(const Face& face);
		void AddFace(const ConvexPolygon& convexPolygon, double eps = MW_EPS);
		int FindOrCreateVertex(const Vertex& vertex, bool canCreate = true, double eps = MW_EPS);
		int FindVertex(const Vertex& vertex, double eps = MW_EPS) const;

		void ToPolygonArray(std::vector<ConvexPolygon>& polygonArray, bool appendOnly = false) const;
		void FromPolygonArray(const std::vector<ConvexPolygon>& polygonArray);

		AxisAlignedBox CalcBoundingBox() const;

		static Mesh* GenerateConvexHull(const std::vector<Vector>& pointArray);

		void RebuildIndexIfNeeded();

	private:

		std::vector<Vertex>* vertexArray;
		std::vector<Face>* faceArray;

		// TODO: Create an index based on a BSP tree.  Maybe?  The main limitation of
		//       this index is that it requires an exact match.  Sometimes we need a
		//       bit of tolerance when looking, approximately, for a given vertex.
		class Index
		{
		public:
			Index();
			virtual ~Index();

			int FindOrCreateVertex(const Vertex& vertex, Mesh* mesh, bool canCreate);
			void Rebuild(const Mesh* mesh);
			bool IsValid(const Mesh* mesh) const;

		private:

			std::string MakeKey(const Vertex& vertex) const;

			std::map<std::string, int>* vertexMap;
		};

		Index* index;
	};
}