#pragma once

#include "Defines.h"

namespace MeshWarrior
{
	class Mesh;

	// TODO: Write this class.  I'm not yet sure how to do it, but I have some ideas.
	//       The result should be a useful description of the topology of a given mesh in
	//       terms of all boundaries and holes of the mesh.  A line-loop (not necessarily a
	//       polygon, convex or otherwise) could be used to represent all boundaries, and a
	//       line-loop could also be used to represent the holes in smallest-circumference
	//       form.  These line-loops should just be pointers into the mesh's vertex array.
	//       Note that a Mobius Strip should be tested, as well as a Klein Bottle.
	class MESH_WARRIOR_API MeshTopology
	{
	public:
		MeshTopology();
		virtual ~MeshTopology();

		// Calculate the topology of the given mesh.
		bool FromMesh(const Mesh* mesh);

		// Produce a mesh that visualizes the calculated topology.
		Mesh* ToMesh() const;
	};
}