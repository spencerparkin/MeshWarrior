#pragma once

#include "Defines.h"
#include "Vector.h"
#include <vector>

namespace MeshWarrior
{
	class ConvexPolygon;

	class MESH_WARRIOR_API Plane
	{
	public:
		Plane();
		virtual ~Plane();

		double CalcSignedDistanceToPoint(const Vector& point) const;

		void ConstructFromConvexPolygon(const ConvexPolygon& polygon);
		void FitToPoints(const std::vector<Vector>& pointArray);

		double constant;
		Vector unitNormal;
	};
}