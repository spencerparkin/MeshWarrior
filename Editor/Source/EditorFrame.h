#pragma once

#include <wx/frame.h>
#include <wx/aui/aui.h>

namespace MeshWarrior
{
	class EditorFrame : public wxFrame
	{
	public:
		EditorFrame(wxWindow* parent, const wxPoint& pos, const wxSize& size);
		virtual ~EditorFrame();

		void OnExit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);
		void OnImport(wxCommandEvent& event);
		void OnExport(wxCommandEvent& event);
		void OnUpdateMenuItemUI(wxUpdateUIEvent& event);
		void OnClose(wxCloseEvent& event);

		enum
		{
			ID_Exit = wxID_HIGHEST,
			ID_About,
			ID_Import,
			ID_Export
		};

		wxAuiManager* auiManager;
	};
}