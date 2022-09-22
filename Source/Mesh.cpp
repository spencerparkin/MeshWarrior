#include "Mesh.h"
#include <sstream>
#include <set>

using namespace MeshWarrior;

Mesh::Mesh()
{
	this->vertexArray = new std::vector<Vertex>();
	this->faceArray = new std::vector<Face>();
}

/*virtual*/ Mesh::~Mesh()
{
	delete this->vertexArray;
	delete this->faceArray;
}

Mesh::Vertex* Mesh::GetVertex(int i)
{
	if (!this->IsValidVertex(i))
		return nullptr;

	return &(*this->vertexArray)[i];
}

const Mesh::Vertex* Mesh::GetVertex(int i) const
{
	return const_cast<Mesh*>(this)->GetVertex(i);
}

bool Mesh::SetVertex(int i, const Vertex& vertex)
{
	if (!this->IsValidVertex(i))
		return false;

	(*this->vertexArray)[i] = vertex;
	return true;
}

bool Mesh::IsValidVertex(int i) const
{
	return i >= 0 && i < (signed)this->vertexArray->size();
}

int Mesh::GetNumVertices() const
{
	return this->vertexArray->size();
}

Mesh::Face* Mesh::GetFace(int i)
{
	if (!this->IsValidFace(i))
		return nullptr;

	return &(*this->faceArray)[i];
}

const Mesh::Face* Mesh::GetFace(int i) const
{
	return const_cast<Mesh*>(this)->GetFace(i);
}

bool Mesh::SetFace(int i, const Face& face)
{
	if (!this->IsValidFace(i))
		return false;

	(*this->faceArray)[i] = face;
	return true;
}

bool Mesh::IsValidFace(int i) const
{
	return i >= 0 && i < (signed)this->faceArray->size();
}

int Mesh::GetNumFaces() const
{
	return this->faceArray->size();
}

void Mesh::Clear()
{
	this->vertexArray->clear();
	this->faceArray->clear();
}

int Mesh::AddVertex(const Vertex& vertex)
{
	this->vertexArray->push_back(vertex);
	return this->vertexArray->size() - 1;
}

bool Mesh::AddFace(const Face& face)
{
	for (int i : face.vertexArray)
		if (!this->IsValidVertex(i))
			return false;

	// Hmmm, but is this face already in the mesh?
	this->faceArray->push_back(face);
	return true;
}

void Mesh::AddFace(const ConvexPolygon& convexPolygon, Index* index /* = nullptr*/, double eps /*= 1e-6*/)
{
	Face face;
	for (const Vertex& vertex : convexPolygon.vertexArray)
		face.vertexArray.push_back(this->FindOrCreateVertex(vertex, index));

	this->faceArray->push_back(face);
}

int Mesh::FindOrCreateVertex(const Vertex& vertex, Index* index /*= nullptr*/, double eps /*= 1e-6*/)
{
	// It is up to the caller to make sure that the index is valid.
	if (index)
		return index->FindOrCreateVertex(vertex, this);

	for(int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		const Vertex& existingVertex = (*this->vertexArray)[i];
		if ((existingVertex.point - vertex.point).Length() < eps)
			return i;
	}

	this->vertexArray->push_back(vertex);
	return this->vertexArray->size() - 1;
}

void Mesh::ToPolygonArray(std::vector<ConvexPolygon>& polygonArray) const
{
	polygonArray.clear();
	
	for (const Face& face : *this->faceArray)
	{
		ConvexPolygon polygon;
		for (int i : face.vertexArray)
			polygon.vertexArray.push_back((*this->vertexArray)[i]);
	}
}

void Mesh::FromPolygonArray(const std::vector<ConvexPolygon>& polygonArray)
{
	Index* index = new Index();

	for (const ConvexPolygon& polygon : polygonArray)
		this->AddFace(polygon, index);

	delete index;
}

Mesh::Index::Index()
{
	this->vertexMap = new std::map<std::string, int>();
}

/*virtual*/ Mesh::Index::~Index()
{
	delete this->vertexMap;
}

int Mesh::Index::FindOrCreateVertex(const Vertex& vertex, Mesh* mesh)
{
	int i = -1;

	std::string key = this->MakeKey(vertex);
	std::map<std::string, int>::iterator iter = this->vertexMap->find(key);
	if (iter != this->vertexMap->end())
		i = iter->second;
	else
	{
		mesh->vertexArray->push_back(vertex);
		i = mesh->vertexArray->size() - 1;
		this->vertexMap->insert(std::pair<std::string, int>(key, i));
	}

	return i;
}

std::string Mesh::Index::MakeKey(const Vertex& vertex) const
{
	std::stringstream stringStream;
	stringStream << vertex.point.x << "|" << vertex.point.y << "|" << vertex.point.z;
	std::string key = stringStream.str();
	return key;
}

bool Mesh::Index::IsValid(const Mesh* mesh) const
{
	if (this->vertexMap->size() != mesh->vertexArray->size())
		return false;

	std::set<int> offsetSet;

	for (const std::pair<std::string, int>& pair : *this->vertexMap)
	{
		const std::string& existingKey = pair.first;
		int i = pair.second;
		const Vertex* vertex = mesh->GetVertex(i);
		if (!vertex)
			return false;

		std::string key = this->MakeKey(*vertex);
		if (key != existingKey)
			return false;

		if (offsetSet.find(i) != offsetSet.end())
			return false;

		offsetSet.insert(i);
	}

	return true;
}