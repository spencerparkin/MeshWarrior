#include "Plane.h"

using namespace MeshWarrior;

Plane::Plane()
{
	this->constant = 0.0;
}

/*virtual*/ Plane::~Plane()
{
}

double Plane::CalcSignedDistanceToPoint(const Vector& point) const
{
	return this->constant + Vector::Dot(this->unitNormal, point);
}

void Plane::ConstructFromConvexPolygon(const ConvexPolygon& polygon)
{
	// TODO: Take best cross product as normal.
}

void Plane::FitToPoints(const std::vector<Vector>& pointArray)
{
	// TODO: Perform a least squares linear regression here.
}