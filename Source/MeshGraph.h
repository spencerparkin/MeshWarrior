#pragma once

#include "Defines.h"
#include <vector>

namespace MeshWarrior
{
	class MESH_WARRIOR_API MeshGraph
	{
	public:
		
		class Face;

		class Edge
		{
		public:
			Face* face[2];
		};

		class Face
		{
		public:
			std::vector<Edge*> edgeList;
			void* userData;
		};

		std::vector<Edge*> edgeList;
		std::vector<Face*> faceList;
	};
}