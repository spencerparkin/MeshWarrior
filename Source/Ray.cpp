#include "Ray.h"

using namespace MeshWarrior;

Ray::Ray()
{
}

Ray::Ray(const Vector& rayOrigin, const Vector& rayDirection)
{
	this->origin = rayOrigin;
	this->direction = rayDirection;
	this->direction.Normalize();
}

/*virtual*/ Ray::~Ray()
{
}

Vector Ray::CalcRayPoint(double rayAlpha) const
{
	return this->origin + this->direction * rayAlpha;
}

double Ray::CalcRayAlpha(const Vector& rayPoint) const
{
	return Vector::Dot(rayPoint - this->origin, this->direction);
}