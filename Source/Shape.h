#pragma once

#include "Defines.h"
#include "Vector.h"

namespace MeshWarrior
{
	class Ray;

	class MESH_WARRIOR_API Shape
	{
	public:
		Shape();
		virtual ~Shape();

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const = 0;
		virtual bool ContainsPoint(const Vector& point, double eps = MW_EPS) const;
		virtual Shape* IntersectWith(const Shape* shape) const;
		virtual bool RayCast(const Ray& ray, double& rayAlpha) const;
	};

	class MESH_WARRIOR_API Point : public Shape
	{
	public:
		Point();
		Point(const Vector& center);
		virtual ~Point();

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;

		Vector center;
	};

	class MESH_WARRIOR_API Plane : public Shape
	{
	public:
		Plane();
		Plane(const Vector& center, const Vector& normal);

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;
		virtual Shape* IntersectWith(const Shape* shape) const override;
		virtual bool RayCast(const Ray& ray, double& rayAlpha) const override;

		Vector center;
		Vector unitNormal;
	};

	class MESH_WARRIOR_API Line : public Shape
	{
	public:
		Line();
		Line(const Vector& center, const Vector& normal);

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;
		
		Vector center;
		Vector unitNormal;
	};

	class MESH_WARRIOR_API Disk : public Plane
	{
	public:
		Disk();
		Disk(const Vector& center, const Vector& normal, double radius);
		virtual ~Disk();

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;
		virtual bool ContainsPoint(const Vector& point, double eps = MW_EPS) const override;

		double radius;
	};

	class MESH_WARRIOR_API LineSegment : public Line
	{
	public:
		LineSegment();
		LineSegment(const Vector& center, const Vector& normal, double radius);
		LineSegment(const Vector& pointA, const Vector& pointB);
		virtual ~LineSegment();

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;
		virtual bool ContainsPoint(const Vector& point, double eps = MW_EPS) const override;

		Vector GetPoint(int i) const;

		double radius;
	};

	class MESH_WARRIOR_API Sphere : public Shape
	{
	public:
		Sphere();
		Sphere(const Vector& center, double radius);
		virtual ~Sphere();

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;

		Vector center;
		double radius;
	};

	class MESH_WARRIOR_API Cylinder : public Line
	{
	public:
		Cylinder();
		Cylinder(const Vector& center, const Vector& normal, double radius);
		virtual ~Cylinder();

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;

		double radius;
	};
}