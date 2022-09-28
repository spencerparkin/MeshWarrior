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

		static void GeneratePolylines(const std::vector<LineSegment*>& lineSegmentArray, std::vector<Polyline*>& polylineArray, double eps = 1e-6);

		void Reduce();
		bool HasVertex(const Vector& vertex, double eps = 1e-6) const;
		bool IsLineLoop(double eps = 1e-6) const;

		std::vector<Vector>* vertexArray;
	};
}