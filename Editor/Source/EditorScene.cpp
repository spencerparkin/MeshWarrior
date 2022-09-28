#include "EditorScene.h"

using namespace MeshWarrior;

EditorScene::EditorScene()
{
}

/*virtual*/ EditorScene::~EditorScene()
{
	this->Clear();
}

void EditorScene::Clear()
{
	for (FileObject* fileObject : this->fileObjectArray)
		delete fileObject;

	this->fileObjectArray.clear();
}

void EditorScene::Render(GLenum renderMode) const
{
	// TODO: Handle GL_SELECT and GL_RENDER render modes.
}