#include "OBJFormat.h"
#include "../Mesh.h"

using namespace MeshWarrior;

OBJFormat::OBJFormat()
{
	this->data = new Data();
}

/*virtual*/ OBJFormat::~OBJFormat()
{
	delete this->data;
}

/*virtual*/ Mesh* OBJFormat::Load(const std::string& meshFile)
{
	Mesh* mesh = nullptr;

	std::ifstream fileStream(meshFile, std::ios::in);
	if (fileStream.is_open())
	{
		this->data->pointArray.clear();
		this->data->normalArray.clear();
		this->data->colorArray.clear();
		this->data->texCoordsArray.clear();
		this->data->polygonArray.clear();

		std::string line;
		while (std::getline(fileStream, line))
		{
			std::vector<std::string> tokenArray;
			this->TokenizeLine(line, ' ', tokenArray, true);
			this->ProcessTokenizedLine(tokenArray, mesh);
		}

		mesh = new Mesh();
		mesh->FromPolygonArray(this->data->polygonArray);

		fileStream.close();
	}

	return mesh;
}

void OBJFormat::TokenizeLine(const std::string& line, char delimeter, std::vector<std::string>& tokenArray, bool stripEmptyTokens)
{
	std::stringstream stringStream(line);
	std::string token;
	while (std::getline(stringStream, token, delimeter))
		if(!stripEmptyTokens || token.size() > 0)
			tokenArray.push_back(token);
}

void OBJFormat::ProcessTokenizedLine(const std::vector<std::string>& tokenArray, Mesh* mesh)
{
	if (tokenArray.size() == 0 || tokenArray[0] == "#")
		return;
	
	if (tokenArray[0] == "v")
	{
		Vector point;
		point.x = (tokenArray.size() > 1) ? ::atof(tokenArray[1].c_str()) : 0.0;
		point.y = (tokenArray.size() > 2) ? ::atof(tokenArray[2].c_str()) : 0.0;
		point.z = (tokenArray.size() > 3) ? ::atof(tokenArray[3].c_str()) : 0.0;
		this->data->pointArray.push_back(point);
	}
	else if (tokenArray[0] == "vt")
	{
		Vector texCoords;
		texCoords.x = (tokenArray.size() > 1) ? ::atof(tokenArray[1].c_str()) : 0.0;
		texCoords.y = (tokenArray.size() > 2) ? ::atof(tokenArray[2].c_str()) : 0.0;
		texCoords.z = (tokenArray.size() > 3) ? ::atof(tokenArray[3].c_str()) : 0.0;
		this->data->texCoordsArray.push_back(texCoords);
	}
	else if (tokenArray[0] == "vn")
	{
		Vector normal;
		normal.x = (tokenArray.size() > 1) ? ::atof(tokenArray[1].c_str()) : 0.0;
		normal.y = (tokenArray.size() > 2) ? ::atof(tokenArray[2].c_str()) : 0.0;
		normal.z = (tokenArray.size() > 3) ? ::atof(tokenArray[3].c_str()) : 0.0;
		this->data->normalArray.push_back(normal);
	}
	else if (tokenArray[0] == "f" && tokenArray.size() > 1)
	{
		Mesh::ConvexPolygon polygon;
		
		for (const std::string& token : tokenArray)
		{
			if (token == "f")
				continue;

			std::vector<std::string> vertexTokenArray;
			this->TokenizeLine(token, '/', vertexTokenArray, false);

			Mesh::Vertex vertex;

			int point_i = (vertexTokenArray.size() > 0 && vertexTokenArray[0].size() > 0) ? ::atoi(vertexTokenArray[0].c_str()) : INT_MAX;
			int texCoords_i = (vertexTokenArray.size() > 1 && vertexTokenArray[1].size() > 0) ? ::atoi(vertexTokenArray[1].c_str()) : INT_MAX;
			int normal_i = (vertexTokenArray.size() > 2 && vertexTokenArray[2].size() > 0) ? ::atoi(vertexTokenArray[2].c_str()) : INT_MAX;

			this->LookupAndAssign(this->data->pointArray, point_i, vertex.point);
			this->LookupAndAssign(this->data->texCoordsArray, texCoords_i, vertex.texCoords);
			this->LookupAndAssign(this->data->normalArray, normal_i, vertex.normal);

			polygon.vertexArray.push_back(vertex);
		}

		this->data->polygonArray.push_back(polygon);
	}
}

void OBJFormat::LookupAndAssign(const std::vector<Vector>& vectorArray, int i, Vector& result)
{
	if (vectorArray.size() > 0 && i != INT_MAX)
	{
		// 1, 2, 3, ... becomes 0, 1, 2, ...
		// -1, -2, -3, ... becomes N-1, N-2, N-3, ...
		if (i > 0)
			i--;
		else if (i < 0)
			i = vectorArray.size() + i;

		// Clamp the offset to be in range.
		if (i < 0)
			i = 0;
		else if (i >= (signed)vectorArray.size())
			i = vectorArray.size() - 1;

		result = vectorArray[i];
	}
}

/*virtual*/ bool OBJFormat::Save(const std::string& meshFile, const Mesh& mesh)
{
	return false;
}