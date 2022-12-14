#pragma once

#include "FileFormat.h"
#include <wx/frame.h>
#include <wx/aui/aui.h>
#include <wx/timer.h>

namespace MeshWarrior
{
	class EditorPanel;

	class EditorFrame : public wxFrame
	{
	public:
		EditorFrame(wxWindow* parent, const wxPoint& pos, const wxSize& size);
		virtual ~EditorFrame();

		void OnExit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);
		void OnImport(wxCommandEvent& event);
		void OnExport(wxCommandEvent& event);
		void OnClear(wxCommandEvent& event);
		void OnUpdateMenuItemUI(wxUpdateUIEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnTimer(wxTimerEvent& event);

		bool MakePanels();
		EditorPanel* FindPanel(wxClassInfo* classInfo);
		FileFormat* FindFileFormat(const wxString& filePath);
		void ShowErrorMessage(const wxArrayString& errorArray);

		enum
		{
			ID_Exit = wxID_HIGHEST,
			ID_About,
			ID_Import,
			ID_Export,
			ID_Clear,
			ID_Timer
		};

		wxAuiManager* auiManager;
		wxTimer timer;
		std::vector<FileFormat*> fileFormatArray;
	};
}