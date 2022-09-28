#pragma once

#include "FileObject.h"
#include "EditorCanvas.h"
#include <vector>

namespace MeshWarrior
{
	class EditorScene
	{
	public:
		EditorScene();
		virtual ~EditorScene();

		void Render(GLenum renderMode) const;
		void Clear();

		std::vector<FileObject*> fileObjectArray;
	};
}