#include "EditorApp.h"
#include "EditorFrame.h"

using namespace MeshWarrior;

wxIMPLEMENT_APP(EditorApp);

EditorApp::EditorApp()
{
	this->frame = nullptr;
}

/*virtual*/ EditorApp::~EditorApp()
{
}

/*virtual*/ bool EditorApp::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	this->frame = new EditorFrame(nullptr, wxDefaultPosition, wxSize(800, 600));
	this->frame->Show();

	return true;
}

/*virtual*/ int EditorApp::OnExit(void)
{
	return 0;
}