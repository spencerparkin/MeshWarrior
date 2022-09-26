#include "Mesh.h"
#include "Polygon.h"
#include <sstream>
#include <set>

using namespace MeshWarrior;

//--------------------------------- Mesh ---------------------------------

Mesh::Mesh()
{
	this->vertexArray = new std::vector<Vertex>();
	this->faceArray = new std::vector<Face>();
	this->index = nullptr;
}

/*virtual*/ Mesh::~Mesh()
{
	delete this->vertexArray;
	delete this->faceArray;
	delete this->index;
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
	return (int)this->vertexArray->size();
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
	return (int)this->faceArray->size();
}

void Mesh::Clear()
{
	this->vertexArray->clear();
	this->faceArray->clear();
}

int Mesh::AddVertex(const Vertex& vertex)
{
	this->vertexArray->push_back(vertex);
	return (int)this->vertexArray->size() - 1;
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

void Mesh::AddFace(const ConvexPolygon& convexPolygon, double eps /*= 1e-6*/)
{
	Face face;
	for (const Vertex& vertex : convexPolygon.vertexArray)
		face.vertexArray.push_back(this->FindOrCreateVertex(vertex, true, eps));

	this->faceArray->push_back(face);
}

int Mesh::FindOrCreateVertex(const Vertex& vertex, bool canCreate /*= true*/, double eps /*= 1e-6*/)
{
	if (this->index && eps == 0.0)
		return index->FindOrCreateVertex(vertex, this, canCreate);

	for (int i = 0; i < (int)this->vertexArray->size(); i++)
		if (((*this->vertexArray)[i].point - vertex.point).Length() <= eps)
			return i;

	if (!canCreate)
		return -1;

	this->vertexArray->push_back(vertex);
	return (int)this->vertexArray->size() - 1;
}

int Mesh::FindVertex(const Vertex& vertex, double eps /*= 1e-6*/) const
{
	return const_cast<Mesh*>(this)->FindOrCreateVertex(vertex, false, eps);
}

void Mesh::ToPolygonArray(std::vector<ConvexPolygon>& polygonArray) const
{
	polygonArray.clear();
	
	for (const Face& face : *this->faceArray)
		polygonArray.push_back(face.GeneratePolygon(this));
}

void Mesh::FromPolygonArray(const std::vector<ConvexPolygon>& polygonArray)
{
	for (const ConvexPolygon& polygon : polygonArray)
		this->AddFace(polygon);

	delete index;
}

AxisAlignedBox Mesh::CalcBoundingBox() const
{
	AxisAlignedBox boundingBox;
	
	for (int i = 0; i < (int)this->vertexArray->size(); i++)
		boundingBox.MinimallyExpandToContainPoint((*this->vertexArray)[i].point);

	return boundingBox;
}

Mesh::ConvexPolygon Mesh::Face::GeneratePolygon(const Mesh* mesh) const
{
	ConvexPolygon polygon;

	for (int i : this->vertexArray)
		polygon.vertexArray.push_back((*mesh->vertexArray)[i]);

	return polygon;
}

void Mesh::ConvexPolygon::ToBasicPolygon(MeshWarrior::ConvexPolygon& polygon) const
{
	polygon.vertexArray->clear();
	for (const Mesh::Vertex& vertex : this->vertexArray)
		polygon.vertexArray->push_back(vertex.point);
}

void Mesh::ConvexPolygon::FromBasicPolygon(const MeshWarrior::ConvexPolygon& polygon)
{
	Plane plane;
	polygon.CalcPlane(plane);

	this->vertexArray.clear();
	for (const Vector& point : *polygon.vertexArray)
	{
		Mesh::Vertex vertex;
		vertex.point = point;
		vertex.normal = plane.unitNormal;
		this->vertexArray.push_back(vertex);
	}
}

/*static*/ Mesh* Mesh::GenerateConvexHull(const std::vector<Vector>& pointArray)
{
	// TODO: Write this.
	return nullptr;
}

void Mesh::RebuildIndexIfNeeded()
{
	if (!this->index || !this->index->IsValid(this))
	{
		delete this->index;
		this->index = new Index();
		this->index->Rebuild(this);
	}
}

//--------------------------------- Index ---------------------------------

Mesh::Index::Index()
{
	this->vertexMap = new std::map<std::string, int>();
}

/*virtual*/ Mesh::Index::~Index()
{
	delete this->vertexMap;
}

int Mesh::Index::FindOrCreateVertex(const Vertex& vertex, Mesh* mesh, bool canCreate)
{
	int i = -1;

	std::string key = this->MakeKey(vertex);
	std::map<std::string, int>::iterator iter = this->vertexMap->find(key);
	if (iter != this->vertexMap->end())
		i = iter->second;
	else if (canCreate)
	{
		mesh->vertexArray->push_back(vertex);
		i = (int)mesh->vertexArray->size() - 1;
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

void Mesh::Index::Rebuild(const Mesh* mesh)
{
	this->vertexMap->clear();

	for (int i = 0; i < (int)mesh->vertexArray->size(); i++)
	{
		const Vertex& vertex = (*mesh->vertexArray)[i];
		std::string key = this->MakeKey(vertex);
		(*this->vertexMap)[key] = i;
	}
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