#include "Shape.h"

using namespace MeshWarrior;

//--------------------------------- Shape ---------------------------------

Shape::Shape()
{
}

/*virtual*/ Shape::~Shape()
{
}

/*virtual*/ bool Shape::ContainsPoint(const Vector& point, double eps /*= 1e-5*/) const
{
	double distance = ::fabs(this->ShortestSignedDistanceToPoint(point));
	return distance <= eps;
}

//--------------------------------- Plane ---------------------------------

Plane::Plane()
{
}

Plane::Plane(const Vector& center, const Vector& normal)
{
	this->center = center;
	this->unitNormal = normal;
	this->unitNormal.Normalize();
}

/*virtual*/ double Plane::ShortestSignedDistanceToPoint(const Vector& point) const
{
	return Vector::Dot(point - this->center, this->unitNormal);
}

//--------------------------------- Line ---------------------------------

Line::Line()
{
}

Line::Line(const Vector& center, const Vector& normal)
{
	this->center = center;
	this->unitNormal = normal;
	this->unitNormal.Normalize();
}

/*virtual*/ double Line::ShortestSignedDistanceToPoint(const Vector& point) const
{
	Vector vector;
	vector.Reject(point - this->center, this->unitNormal);
	return vector.Length();
}

//--------------------------------- Disk ---------------------------------

Disk::Disk()
{
}

Disk::Disk(const Vector& center, const Vector& normal, double radius) : Plane(center, normal)
{
	this->radius = radius;
}

/*virtual*/ Disk::~Disk()
{
}

/*virtual*/ double Disk::ShortestSignedDistanceToPoint(const Vector& point) const
{
	Vector vector;
	vector.Reject(point - this->center, this->unitNormal);
	double length = vector.Length();
	if (length <= this->radius)
		return Plane::ShortestSignedDistanceToPoint(point);

	vector.Scale(this->radius / length);
	return (point - this->center + vector).Length();
}

/*virtual*/ bool Disk::ContainsPoint(const Vector& point, double eps /*= 1e-5*/) const
{
	if (!Plane::ContainsPoint(point, eps))
		return false;

	return (point - this->center).Length() <= this->radius + eps;
}

//--------------------------------- LineSegment ---------------------------------

LineSegment::LineSegment()
{
}

LineSegment::LineSegment(const Vector& center, const Vector& normal, double radius) : Line(center, normal)
{
	this->radius = radius;
}

/*virtual*/ LineSegment::~LineSegment()
{
}

/*virtual*/ double LineSegment::ShortestSignedDistanceToPoint(const Vector& point) const
{
	Vector vector;
	vector.Project(point - this->center, this->unitNormal);
	double length = vector.Length();
	if (length <= this->radius)
		return Line::ShortestSignedDistanceToPoint(point);

	vector.Scale(this->radius / length);
	return (point - this->center + vector).Length();
}

/*virtual*/ bool LineSegment::ContainsPoint(const Vector& point, double eps /*= 1e-5*/) const
{
	if (!Line::ContainsPoint(point, eps))
		return false;

	return (point - this->center).Length() <= this->radius + eps;
}

//--------------------------------- Sphere ---------------------------------

Sphere::Sphere()
{
}

Sphere::Sphere(const Vector& center, double radius)
{
	this->center = center;
	this->radius = radius;
}

/*virtual*/ Sphere::~Sphere()
{
}

/*virtual*/ double Sphere::ShortestSignedDistanceToPoint(const Vector& point) const
{
	return (point - this->center).Length() - this->radius;
}

//--------------------------------- Cylinder ---------------------------------

Cylinder::Cylinder()
{
}

Cylinder::Cylinder(const Vector& center, const Vector& normal, double radius) : Line(center, normal)
{
	this->radius = radius;
}

/*virtual*/ Cylinder::~Cylinder()
{
}

/*virtual*/ double Cylinder::ShortestSignedDistanceToPoint(const Vector& point) const
{
	double distance = Line::ShortestSignedDistanceToPoint(point);
	return this->radius - distance;
}