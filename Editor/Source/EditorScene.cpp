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
		glMultMatrixd(modelMat);

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

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	this->DrawMesh();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0.5f, 0.5f, 0.5f);
	this->DrawMesh();
}

void EditorScene::MeshRenderObject::DrawMesh() const
{
	glBegin(GL_TRIANGLES);

	for (int i = 0; i < (int)this->triMesh->GetNumFaces(); i++)
	{
		const Mesh::Face* face = this->triMesh->GetFace(i);
		wxASSERT(face->vertexArray.size() == 3);

		for (int j = 0; j < (int)face->vertexArray.size(); j++)
		{
			int k = face->vertexArray[j];
			const Mesh::Vertex* vertex = this->triMesh->GetVertex(k);

			//glNormal3d(vertex->normal.x, vertex->normal.y, vertex->normal.z);
			//glTexCoord3d(vertex->texCoords.x, vertex->texCoords.y, vertex->texCoords.z);
			//glColor3d(vertex->color.x, vertex->color.y, vertex->color.z);
			glVertex3d(vertex->point.x, vertex->point.y, vertex->point.z);
		}
	}

	glEnd();
}

/*virtual*/ void EditorScene::MeshRenderObject::SolidifyTransform()
{
	// TODO: Apply the local-to-world transform to the mesh, then reset the local-to-world transform.
	//       This is also where we would rebuild our vertex buffers, by the way.



	this->localToWorldTransform.SetIdentity();
}