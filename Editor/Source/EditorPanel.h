#pragma once

#include <wx/panel.h>
#include <wx/aui/aui.h>

namespace MeshWarrior
{
	class EditorPanel : public wxPanel
	{
	public:
		wxDECLARE_DYNAMIC_CLASS(EditorPanel);

		EditorPanel();
		virtual ~EditorPanel();

		virtual bool MakePaneInfo(wxAuiPaneInfo& paneInfo);
		virtual bool MakeControls(void);
		virtual void DoIdleProcessing(void);
	};
}