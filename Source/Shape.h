#pragma once

#include "Defines.h"
#include "Vector.h"

namespace MeshWarrior
{
	// It might be possible generate a mesh using the interface of this base class.
	class MESH_WARRIOR_API Shape
	{
	public:
		Shape();
		virtual ~Shape();

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const = 0;
		virtual bool ContainsPoint(const Vector& point, double eps = 1e-5) const;
	};

	class MESH_WARRIOR_API Plane : public Shape
	{
	public:
		Plane();
		Plane(const Vector& center, const Vector& normal);

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;

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
		virtual bool ContainsPoint(const Vector& point, double eps = 1e-5) const override;

		double radius;
	};

	class MESH_WARRIOR_API LineSegment : public Line
	{
	public:
		LineSegment();
		LineSegment(const Vector& center, const Vector& normal, double radius);
		virtual ~LineSegment();

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;
		virtual bool ContainsPoint(const Vector& point, double eps = 1e-5) const override;

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