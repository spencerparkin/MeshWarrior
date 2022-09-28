#pragma once

#include "Defines.h"
#include <string>
#include <vector>

namespace MeshWarrior
{
	class FileObject;
	class Mesh;

	class MESH_WARRIOR_API FileFormat
	{
	public:
		FileFormat();
		virtual ~FileFormat();

		virtual std::string SupportedExtension() = 0;

		virtual bool Load(const std::string& meshFile, std::vector<FileObject*>& fileObjectArray) = 0;
		virtual bool Save(const std::string& meshFile, const std::vector<FileObject*>& fileObjectArray) = 0;

		Mesh* LoadMesh(const std::string& meshFile);
		bool SaveMesh(const std::string& meshFile, const Mesh& mesh);
	};
}