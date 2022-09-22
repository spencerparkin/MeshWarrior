#pragma once

#include "../MeshFormat.h"
#include "../Vector.h"
#include "../Mesh.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace MeshWarrior
{
	class MESH_WARRIOR_API OBJFormat : public MeshFormat
	{
	public:
		OBJFormat();
		virtual ~OBJFormat();

		virtual Mesh* Load(const std::string& meshFile) override;
		virtual bool Save(const std::string& meshFile, const Mesh& mesh) override;

	private:

		struct Data
		{
			std::vector<Vector> pointArray;
			std::vector<Vector> normalArray;
			std::vector<Vector> colorArray;
			std::vector<Vector> texCoordsArray;
			std::vector<Mesh::ConvexPolygon> polygonArray;
		};

		Data* data;

		void TokenizeLine(const std::string& line, char delimeter, std::vector<std::string>& tokenArray, bool stripEmptyTokens);
		void ProcessTokenizedLine(const std::vector<std::string>& tokenArray, Mesh* mesh);
		void LookupAndAssign(const std::vector<Vector>& vectorArray, int i, Vector& result);
	};
}