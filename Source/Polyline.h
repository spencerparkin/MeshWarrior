#pragma once

#include "Defines.h"
#include "FileObject.h"
#include "Vector.h"
#include <vector>

namespace MeshWarrior
{
	class LineSegment;

	class MESH_WARRIOR_API Polyline : public FileObject
	{
	public:
		Polyline();
		virtual ~Polyline();

		static void GeneratePolylines(const std::vector<LineSegment*>& lineSegmentArray, std::vector<Polyline*>& polylineArray, double eps = MW_EPS);

		void Reduce();
		bool HasVertex(const Vector& vertex, double eps = MW_EPS) const;
		bool ContainsPoint(const Vector& point, double eps = MW_EPS) const;
		bool IsLineLoop(double eps = MW_EPS) const;

		std::vector<Vector>* vertexArray;
	};
}