#include "EditorCanvas.h"

using namespace MeshWarrior;

int EditorCanvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

EditorCanvas::EditorCanvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize)
{
	this->context = nullptr;

	this->Bind(wxEVT_PAINT, &EditorCanvas::OnPaint, this);
	this->Bind(wxEVT_SIZE, &EditorCanvas::OnSize, this);
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

	// TODO: Call render function which can also handle picking mode.

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