#include "Shape.h"
#include "Ray.h"
#include <assert.h>

using namespace MeshWarrior;

//--------------------------------- Shape ---------------------------------

Shape::Shape()
{
}

/*virtual*/ Shape::~Shape()
{
}

/*virtual*/ bool Shape::ContainsPoint(const Vector& point, double eps /*= MW_EPS*/) const
{
	double distance = ::fabs(this->ShortestSignedDistanceToPoint(point));
	return distance <= eps;
}

/*virtual*/ Shape* Shape::IntersectWith(const Shape* shape) const
{
	// Some intersections are non-trivial, but when they are
	// easy to calculate, a derived class can do that with an
	// override of this method.
	return nullptr;
}

/*virtual*/ bool Shape::RayCast(const Ray& ray, double& rayAlpha) const
{
	rayAlpha = 0.0;
	return false;
}

//--------------------------------- Point ---------------------------------

Point::Point()
{
}

Point::Point(const Vector& center)
{
	this->center = center;
}

/*virtual*/ Point::~Point()
{
}

/*virtual*/ double Point::ShortestSignedDistanceToPoint(const Vector& point) const
{
	return (point - this->center).Length();
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

/*virtual*/ Shape* Plane::IntersectWith(const Shape* shape) const
{
	const Line* line = dynamic_cast<const Line*>(shape);
	if (line)
	{
		double numer = Vector::Dot(this->center - line->center, this->unitNormal);
		double denom = Vector::Dot(this->unitNormal, line->unitNormal);
		if (denom != 0.0)
		{
			double lambda = numer / denom;
			if (!(lambda != lambda || ::isnan(lambda) || ::isinf(lambda)))
			{
				Vector point = line->center + line->unitNormal * lambda;

				// We still have to check here, because we could be a line segment.
				if (line->ContainsPoint(point))
				{
					// Sanity check: Is the point on the plane?
					MW_ASSERT(this->ContainsPoint(point));

					return new Point(point);
				}
			}
		}
	}

	return nullptr;
}

/*virtual*/ bool Plane::RayCast(const Ray& ray, double& rayAlpha) const
{
	double numer = Vector::Dot(this->center - ray.origin, this->unitNormal);
	double denom = Vector::Dot(ray.direction, this->unitNormal);

	if (denom == 0.0)
		return false;

	rayAlpha = numer / denom;
	if (rayAlpha != rayAlpha || ::isinf(rayAlpha) || ::isnan(rayAlpha))
		return false;

	return true;
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

/*virtual*/ bool Disk::ContainsPoint(const Vector& point, double eps /*= MW_EPS*/) const
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

LineSegment::LineSegment(const Vector& pointA, const Vector& pointB)
{
	this->center = (pointA + pointB) / 2.0;
	this->unitNormal = pointB - pointA;
	this->unitNormal.Normalize(nullptr, &this->radius);
	this->radius /= 2.0;
}

/*virtual*/ LineSegment::~LineSegment()
{
}

/*virtual*/ double LineSegment::ShortestSignedDistanceToPoint(const Vector& point) const
{
	double signedLength = Vector::Dot(point - this->center, this->unitNormal);
	if (fabs(signedLength) <= this->radius)
		return Line::ShortestSignedDistanceToPoint(point);

	Vector nearestEdgePoint = this->center + this->unitNormal * this->radius * MW_SIGN(signedLength);
	return (point - nearestEdgePoint).Length();
}

/*virtual*/ bool LineSegment::ContainsPoint(const Vector& point, double eps /*= MW_EPS*/) const
{
	if (!Line::ContainsPoint(point, eps))
		return false;

	Vector vector = point - this->center;
	Vector projection;
	projection.Project(vector, this->unitNormal);
	return projection.Length() <= this->radius + eps;
}

Vector LineSegment::GetPoint(int i) const
{
	if (i % 2 == 0)
		return this->center - this->unitNormal * this->radius;
	else
		return this->center + this->unitNormal * this->radius;
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