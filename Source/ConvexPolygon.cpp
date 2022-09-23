#include "ConvexPolygon.h"

using namespace MeshWarrior;

ConvexPolygon::ConvexPolygon()
{
	this->vertexArray = new std::vector<Vector>();
}

/*virtual*/ ConvexPolygon::~ConvexPolygon()
{
	delete this->vertexArray;
}

bool ConvexPolygon::IsValid() const
{
	//...
	return true;
}

void ConvexPolygon::ReverseWinding()
{
	//...
}