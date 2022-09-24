#include "AlgebraicSurface.h"

using namespace MeshWarrior;

//--------------------------------- AlgebraicSurface ---------------------------------

AlgebraicSurface::AlgebraicSurface()
{
}

/*virtual*/ AlgebraicSurface::~AlgebraicSurface()
{
}

//--------------------------------- QuadraticSurface ---------------------------------

QuadraticSurface::QuadraticSurface()
{
	this->a = 0.0;
	this->b = 0.0;
	this->c = 0.0;
	this->d = 0.0;
	this->e = 0.0;
	this->f = 0.0;
	this->g = 0.0;
	this->h = 0.0;
	this->i = 0.0;
}

/*virtual*/ QuadraticSurface::~QuadraticSurface()
{
}

/*virtual*/ double QuadraticSurface::Evaluate(const Vector& point) const
{
	double x = point.x;
	double y = point.y;
	double z = point.z;
	double xx = x * x;
	double xy = x * y;
	double xz = x * z;
	double yy = y * y;
	double yz = y * z;
	double zz = z * z;

	return a * xx + b * yy + c * zz + d * xy + e * yz + f * x + g * y + h * z + i;
}

/*virtual*/ Vector QuadraticSurface::EvaluateGradient(const Vector& point) const
{
	return Vector(0.0, 0.0, 0.0);	// TODO: Write this.
}