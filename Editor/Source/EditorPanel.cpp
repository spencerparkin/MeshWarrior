#include "EditorPanel.h"

using namespace MeshWarrior;

wxIMPLEMENT_DYNAMIC_CLASS(EditorPanel, wxPanel);

EditorPanel::EditorPanel()
{
}

/*virtual*/ EditorPanel::~EditorPanel()
{
}

/*virtual*/ bool EditorPanel::MakePaneInfo(wxAuiPaneInfo& paneInfo)
{
	return false;
}

/*virtual*/ bool EditorPanel::MakeControls(void)
{
	return false;
}

/*virtual*/ void EditorPanel::DoIdleProcessing(void)
{
}