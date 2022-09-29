#pragma once

#include "FileObject.h"
#include "EditorCanvas.h"
#include "Transform.h"
#include <vector>
#include <list>

namespace MeshWarrior
{
	class Mesh;

	class EditorScene
	{
	public:
		EditorScene();
		virtual ~EditorScene();

		void Render(GLenum renderMode) const;
		void Clear();

		bool AddFileObject(FileObject* fileObject);
		void GetAllFileObjects(std::vector<FileObject*>& fileObjectArray);

	private:

		class RenderObject
		{
		public:
			RenderObject();
			virtual ~RenderObject();

			virtual void Render(GLenum renderMode) const = 0;
			virtual void SolidifyTransform() = 0;

			Transform localToWorldTransform;
		};

		class MeshRenderObject : public RenderObject
		{
		public:
			MeshRenderObject(Mesh* mesh);
			virtual ~MeshRenderObject();

			virtual void Render(GLenum renderMode) const override;
			virtual void SolidifyTransform() override;

			void DrawMesh() const;

			Mesh* mesh;
			mutable Mesh* triMesh;
		};

		std::list<RenderObject*> renderObjectList;
	};
}