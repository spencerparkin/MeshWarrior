#pragma once

#include "Defines.h"
#include <string>
#include <vector>

namespace MeshWarrior
{
	// This is a file-format-independent object that can be read-from and written-to file in a specific format.
	class MESH_WARRIOR_API FileObject
	{
	public:
		FileObject();
		virtual ~FileObject();

		static void DeleteArray(std::vector<FileObject*>& fileObjectArray);

		std::string* name;
	};
}