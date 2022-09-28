#include "CanvasPanel.h"
#include "../EditorCanvas.h"
#include <wx/sizer.h>

using namespace MeshWarrior;

wxIMPLEMENT_DYNAMIC_CLASS(CanvasPanel, EditorPanel);

CanvasPanel::CanvasPanel()
{
	this->canvas = nullptr;
}

/*virtual*/ CanvasPanel::~CanvasPanel()
{
}

/*virtual*/ bool CanvasPanel::MakePaneInfo(wxAuiPaneInfo& paneInfo)
{
	paneInfo.Caption("3D World");
	paneInfo.CenterPane();
	paneInfo.Name("Canvas");
	return true;
}

/*virtual*/ bool CanvasPanel::MakeControls(void)
{
	this->canvas = new EditorCanvas(this);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(canvas, 1, wxALL | wxGROW, 0);
	this->SetSizer(boxSizer);

	return true;
}

/*virtual*/ void CanvasPanel::DoIdleProcessing(void)
{
}