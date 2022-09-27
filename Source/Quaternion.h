#pragma once

#include "Defines.h"
#include "Vector.h"

namespace MeshWarrior
{
	class MESH_WARRIOR_API Quaternion
	{
	public:
		Quaternion();
		virtual ~Quaternion();

		Vector GetRotationAxis() const;
		double GetRotationAngle() const;

		Vector RotateVector(const Vector& vector) const;

		double x, y, z, w;
	};
}