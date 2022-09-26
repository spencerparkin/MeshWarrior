#pragma once

#include "Defines.h"
#include "Vector.h"
#include "Shape.h"

namespace MeshWarrior
{
	class MESH_WARRIOR_API AxisAlignedBox : public Shape
	{
	public:
		AxisAlignedBox();
		AxisAlignedBox(const Vector& point);
		AxisAlignedBox(const Vector& min, const Vector& max);
		AxisAlignedBox(const AxisAlignedBox& box);
		virtual ~AxisAlignedBox();

		bool IsValid() const;

		virtual double ShortestSignedDistanceToPoint(const Vector& point) const override;
		virtual bool ContainsPoint(const Vector& point, double eps = 0.0) const override;
		bool ContainsPointOnBoundary(const Vector& point, double eps = 1e-6) const;
		bool ContainsBox(const AxisAlignedBox& box) const;

		void ScaleAboutCenter(double delta);
		void AddMargin(double delta);

		void MinimallyExpandToContainPoint(const Vector& point);
		void MinimallyExpandToContainBox(const AxisAlignedBox& box);

		void Combine(const AxisAlignedBox& boxA, const AxisAlignedBox& boxB);
		bool Intersect(const AxisAlignedBox& boxA, const AxisAlignedBox& boxB);
		bool OverlapsWith(const AxisAlignedBox& box) const;

		void SplitReasonably(AxisAlignedBox& boxA, AxisAlignedBox& boxB) const;

		Vector CalcCenter() const;
		double CalcWidth() const;
		double CalcHeight() const;
		double CalcDepth() const;
		double CalcVolume() const;
		double CalcRadius() const;

		Vector min, max;
	};
}