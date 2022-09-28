#include "EditorFrame.h"
#include <wx/aboutdlg.h>

using namespace MeshWarrior;

EditorFrame::EditorFrame(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "Mesh Warrior Editor", pos, size)
{
	this->auiManager = new wxAuiManager(this, wxAUI_MGR_LIVE_RESIZE | wxAUI_MGR_DEFAULT);

	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Import, "Import", "Import meshes from a given file location."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Export, "Export", "Export meshes to a given file location."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Exit the application."));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	wxStatusBar* statusBar = new wxStatusBar(this);
	this->SetStatusBar(statusBar);

	this->Bind(wxEVT_MENU, &EditorFrame::OnImport, this, ID_Import);
	this->Bind(wxEVT_MENU, &EditorFrame::OnExport, this, ID_Export);
	this->Bind(wxEVT_MENU, &EditorFrame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &EditorFrame::OnAbout, this, ID_About);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_Import);
	this->Bind(wxEVT_UPDATE_UI, &EditorFrame::OnUpdateMenuItemUI, this, ID_Export);

	this->auiManager->Update();
}

/*virtual*/ EditorFrame::~EditorFrame()
{
	this->auiManager->UnInit();
	delete this->auiManager;
}

void EditorFrame::OnImport(wxCommandEvent& event)
{

}

void EditorFrame::OnExport(wxCommandEvent& event)
{

}

void EditorFrame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}

void EditorFrame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Mesh Warrior Editor");
	aboutDialogInfo.SetVersion("1.0");
	aboutDialogInfo.SetDescription("This program exposes and exhibits the features provided by the Mesh Warrior shared library.");
	aboutDialogInfo.SetCopyright("Copyright (C) 2022 -- Spencer T. Parkin <SpencerTParkin@gmail.com>");

	wxAboutBox(aboutDialogInfo);
}

void EditorFrame::OnUpdateMenuItemUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_Import:
		{
			break;
		}
		case ID_Export:
		{
			break;
		}
	}
}

void EditorFrame::OnClose(wxCloseEvent& event)
{
	event.Skip();
}