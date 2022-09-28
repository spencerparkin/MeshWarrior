#include "EditorScene.h"
#include "EditorCanvas.h"
#include "Mesh.h"

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
	for (RenderObject* renderObject : this->renderObjectList)
		delete renderObject;

	this->renderObjectList.clear();
}

void EditorScene::Render(GLenum renderMode) const
{
	glEnable(GL_DEPTH_TEST);

	for (RenderObject* renderObject : this->renderObjectList)
	{
		GLdouble modelMat[16];
		EditorCanvas::MakeOpenGLMatrix(renderObject->localToWorldTransform, modelMat);

		glPushMatrix();
		glLoadMatrixd(modelMat);

		renderObject->Render(renderMode);

		glPopMatrix();
	}
}

bool EditorScene::AddFileObject(FileObject* fileObject)
{
	Mesh* mesh = dynamic_cast<Mesh*>(fileObject);
	if (mesh)
	{
		this->renderObjectList.push_back(new MeshRenderObject(mesh));
		return true;
	}

	return true;
}

void EditorScene::GetAllFileObjects(std::vector<FileObject*>& fileObjectArray)
{
	// TODO: Write this.  Probably want to solidify transforms first.
}

//----------------------------- RenderObject -----------------------------

EditorScene::RenderObject::RenderObject()
{
}

/*virtual*/ EditorScene::RenderObject::~RenderObject()
{
}

//----------------------------- MeshRenderObject -----------------------------

EditorScene::MeshRenderObject::MeshRenderObject(Mesh* mesh)
{
	this->mesh = mesh;
	this->triMesh = nullptr;
}

/*virtual*/ EditorScene::MeshRenderObject::~MeshRenderObject()
{
	delete this->mesh;
	delete this->triMesh;
}

/*virtual*/ void EditorScene::MeshRenderObject::Render(GLenum renderMode) const
{
	// TODO: Should try to use modern OpenGL with vertex buffers and shader programs here.

	if (!this->triMesh)
	{
		this->triMesh = this->mesh->GenerateTriangleMesh();
		wxASSERT(this->triMesh->IsTriangleMesh());
	}

	glBegin(GL_TRIANGLES);

	for (int i = 0; i < (int)this->triMesh->GetNumFaces(); i++)
	{
		const Mesh::Face* face = this->triMesh->GetFace(i);
		wxASSERT(face->vertexArray.size() == 3);

		for (int j = 0; j < (int)face->vertexArray.size(); j++)
		{
			const Mesh::Vertex* vertex = this->triMesh->GetVertex(j);
			
			//glNormal3f(vertex->normal.x, vertex->normal.y, vertex->normal.z);
			//glTexCoord3f(vertex->texCoords.x, vertex->texCoords.y, vertex->texCoords.z);
			//glColor3f(vertex->color.x, vertex->color.y, vertex->color.z);
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(vertex->point.x, vertex->point.y, vertex->point.z);
		}
	}

	glEnd();

	// TODO: To draw edges, OpenGL might have a wire-frame mode we could use.
}

/*virtual*/ void EditorScene::MeshRenderObject::SolidifyTransform()
{
	// TODO: Apply the local-to-world transform to the mesh, then reset the local-to-world transform.
	//       This is also where we would rebuild our vertex buffers, by the way.



	this->localToWorldTransform.SetIdentity();
}