#include "EditorCanvas.h"
#include "EditorApp.h"
#include "EditorScene.h"

using namespace MeshWarrior;

int EditorCanvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

EditorCanvas::EditorCanvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize)
{
	this->context = nullptr;

	this->Bind(wxEVT_PAINT, &EditorCanvas::OnPaint, this);
	this->Bind(wxEVT_SIZE, &EditorCanvas::OnSize, this);

	Vector xAxis(1.0, 0.0, 0.0);
	Vector yAxis(0.0, 1.0, 0.0);
	Vector zAxis(0.0, 0.0, 1.0);

	this->cameraTransform.matrix.SetCol(0, xAxis);
	this->cameraTransform.matrix.SetCol(0, yAxis);
	this->cameraTransform.matrix.SetCol(0, zAxis);

	this->cameraTransform.translation = Vector(0.0, 0.0, -10.0);
}

/*virtual*/ EditorCanvas::~EditorCanvas()
{
	delete this->context;
}

void EditorCanvas::BindContext()
{
	if (!this->context)
		this->context = new wxGLContext(this);

	this->SetCurrent(*this->context);
}

void EditorCanvas::OnPaint(wxPaintEvent& event)
{
	this->BindContext();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: Setup view matrices here.

	EditorScene* scene = EditorApp::Get()->scene;
	if (scene)
		scene->Render(GL_RENDER);

	glFlush();

	this->SwapBuffers();
}

void EditorCanvas::OnSize(wxSizeEvent& event)
{
	this->BindContext();

	wxSize size = event.GetSize();
	glViewport(0, 0, size.GetWidth(), size.GetHeight());

	this->Refresh();
}