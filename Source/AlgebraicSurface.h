#pragma once

#include "Defines.h"
#include "Vector.h"

namespace MeshWarrior
{
	// TODO: Write an algorithm that can generate a mesh that approximates a given algebraic surface.
	class MESH_WARRIOR_API AlgebraicSurface
	{
	public:
		AlgebraicSurface();
		virtual ~AlgebraicSurface();

		virtual double Evaluate(const Vector& point) const = 0;
		virtual Vector EvaluateGradient(const Vector& point) const = 0;
	};

	class MESH_WARRIOR_API QuadraticSurface : public AlgebraicSurface
	{
	public:
		QuadraticSurface();
		virtual ~QuadraticSurface();

		virtual double Evaluate(const Vector& point) const override;
		virtual Vector EvaluateGradient(const Vector& point) const override;

		double a, b, c, d, e, f, g, h, i;
	};
}