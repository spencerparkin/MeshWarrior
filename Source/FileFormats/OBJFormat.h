#pragma once

#include "../FileFormat.h"
#include "../Vector.h"
#include "../Mesh.h"
#include "../Polyline.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

namespace MeshWarrior
{
	class MESH_WARRIOR_API OBJFormat : public FileFormat
	{
	public:
		OBJFormat();
		virtual ~OBJFormat();

		virtual bool Load(const std::string& meshFile, std::vector<FileObject*>& fileObjectArray) override;
		virtual bool Save(const std::string& meshFile, const std::vector<FileObject*>& fileObjectArray) override;

	private:

		struct Data
		{
			std::vector<Vector> pointArray;
			std::vector<Vector> normalArray;
			std::vector<Vector> colorArray;
			std::vector<Vector> texCoordsArray;
			std::vector<Mesh::ConvexPolygon> polygonArray;
			std::string name;
		};

		Data* data;
		int totalVertices;
		int totalFaces;

		void TokenizeLine(const std::string& line, char delimeter, std::vector<std::string>& tokenArray, bool stripEmptyTokens);
		void ProcessTokenizedLine(const std::vector<std::string>& tokenArray, std::vector<FileObject*>& fileObjectArray);
		void LookupAndAssign(const std::vector<Vector>& vectorArray, int i, Vector& result);
		void FlushMesh(std::vector<FileObject*>& fileObjectArray);
		void DumpMesh(std::ofstream& fileStream, const Mesh* mesh);
		void DumpPolyline(std::ofstream& fileStream, const Polyline* polyline);
	};
}