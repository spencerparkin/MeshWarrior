#pragma once

#include "Defines.h"
#include "Vector.h"

namespace MeshWarrior
{
	class MESH_WARRIOR_API Ray
	{
	public:
		Ray();
		Ray(const Vector& rayOrigin, const Vector& rayDirection);
		virtual ~Ray();

		Vector CalcRayPoint(double rayAlpha) const;
		double CalcRayAlpha(const Vector& rayPoint) const;

		Vector origin;
		Vector direction;
	};
}