#pragma once

#include "Defines.h"
#include <vector>

namespace MeshWarrior
{
	class MESH_WARRIOR_API MeshGraph
	{
	public:
		
		// I'm not sure how to design this yet, but we need to be able to
		// walk from one face to another along shared edges, even if those
		// edges aren't perfectly shared.
	};
}