#pragma once

#include "Defines.h"
#include "Vector.h"
#include "Shape.h"
#include "Mesh.h"
#include <vector>

namespace MeshWarrior
{
	class ConvexPolygon;

	class MESH_WARRIOR_API Polygon : public Shape
	{
	public:
		Polygon();
		Polygon(const Polygon& polygon);
		virtual ~Polygon();

		void operator=(const Polygon& polygon);

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;
		virtual bool ContainsPoint(const Vector& point, double eps = 1e-5) const override;

		virtual bool IsValid(double eps = 1e-5) const;
		virtual void Tessellate(std::vector<ConvexPolygon>& polygonArray) const;

		void ReverseWinding();
		bool CalcPlane(Plane& plane) const;
		bool FitPlane(Plane& plane) const;
		void ProjectOntoPlane(Plane& plane);
		bool SnapToNearestPlane();
		Vector CalcCenter() const;

		std::vector<Vector>* vertexArray;
	};

	// In many cases, if the vertices of this class don't
	// actually form a convex polygon, then we leave the
	// result of the method undefined.
	class MESH_WARRIOR_API ConvexPolygon : public Polygon
	{
	public:
		ConvexPolygon();
		virtual ~ConvexPolygon();

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;
		virtual bool ContainsPoint(const Vector& point, double eps = 1e-5) const override;
		virtual Shape* IntersectWith(const Shape* shape) const override;

		virtual bool IsValid(double eps = 1e-5) const override;
		virtual void Tessellate(std::vector<ConvexPolygon>& polygonArray) const override;

		bool GenerateEdgePlaneArray(std::vector<Plane>& edgePlaneArray) const;
		void AddMeshPolygon(std::vector<Mesh::ConvexPolygon>& polygonList, const Vector& color) const;
		bool SplitAgainstPlane(const Plane& plane, std::vector<ConvexPolygon>& polygonArray) const;
	};
}