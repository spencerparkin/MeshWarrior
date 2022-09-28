#include "EditorFrame.h"
#include "EditorPanel.h"
#include "EditorApp.h"
#include "EditorScene.h"
#include "FileFormats/OBJFormat.h"
#include <wx/aboutdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

using namespace MeshWarrior;

EditorFrame::EditorFrame(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "Mesh Warrior Editor", pos, size), timer(this, ID_Timer)
{
	this->fileFormatArray.push_back(new OBJFormat());

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
	this->Bind(wxEVT_TIMER, &EditorFrame::OnTimer, this, ID_Timer);

	this->MakePanels();
	this->auiManager->Update();
	this->timer.Start(1);
}

/*virtual*/ EditorFrame::~EditorFrame()
{
	this->auiManager->UnInit();
	delete this->auiManager;

	for (FileFormat* fileFormat : this->fileFormatArray)
		delete fileFormat;
}

bool EditorFrame::MakePanels()
{
	wxClassInfo* basePanelClassInfo = wxClassInfo::FindClass("EditorPanel");
	if (!basePanelClassInfo)
		return false;

	const wxClassInfo* classInfo = wxClassInfo::GetFirst();
	while (classInfo)
	{
		if (classInfo != basePanelClassInfo && classInfo->IsKindOf(basePanelClassInfo))
		{
			EditorPanel* panel = (EditorPanel*)classInfo->CreateObject();
			panel->Create(this);
			if (!panel->MakeControls())
				delete panel;
			else
			{
				wxAuiPaneInfo paneInfo;
				panel->MakePaneInfo(paneInfo);
				paneInfo.CloseButton(false);
				this->auiManager->AddPane(panel, paneInfo);
			}
		}

		classInfo = classInfo->GetNext();
	}

	return true;
}

EditorPanel* EditorFrame::FindPanel(wxClassInfo* classInfo)
{
	wxAuiPaneInfoArray& paneInfoArray = this->auiManager->GetAllPanes();
	for (int i = 0; i < (signed)paneInfoArray.GetCount(); i++)
	{
		wxWindow* window = paneInfoArray[i].window;
		if (window->IsKindOf(classInfo))
			return wxDynamicCast(window, EditorPanel);
	}

	return nullptr;
}

void EditorFrame::OnTimer(wxTimerEvent& event)
{
	wxAuiPaneInfoArray& paneInfoArray = auiManager->GetAllPanes();
	for (int i = 0; i < (signed)paneInfoArray.GetCount(); i++)
	{
		EditorPanel* panel = wxDynamicCast(paneInfoArray[i].window, EditorPanel);
		if (panel)
			panel->DoIdleProcessing();
	}
}

void EditorFrame::OnImport(wxCommandEvent& event)
{
	wxFileDialog fileOpenDlg(this, "Import Meshes", wxEmptyString, wxEmptyString, "OBJ File (*.OBJ)|*.OBJ", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	if (fileOpenDlg.ShowModal() == wxID_OK)
	{
		wxBusyCursor busyCursor;

		wxArrayString errorArray;
		wxArrayString filePathArray;
		fileOpenDlg.GetPaths(filePathArray);
		for (int i = 0; i < (int)filePathArray.size(); i++)
		{
			wxString filePath = filePathArray[i];
			FileFormat* fileFormat = this->FindFileFormat(filePath);
			if (!fileFormat)
				errorArray.push_back("File format not yet supported: " + filePath);
			else
			{
				std::vector<FileObject*> fileObjectArray;
				if (!fileFormat->Load((const char*)filePath.c_str(), fileObjectArray))
					errorArray.push_back("Failed to load file: " + filePath);
				else
				{
					for (FileObject* fileObject : fileObjectArray)
					{
						if (!EditorApp::Get()->scene->AddFileObject(fileObject))
						{
							errorArray.push_back(wxString::Format("Could not handle loaded object (%s) from file: ", (const char*)fileObject->name->c_str()) + filePath);
							delete fileObject;
						}
					}
				}
			}
		}

		this->ShowErrorMessage(errorArray);
		this->Refresh();
	}
}

void EditorFrame::OnExport(wxCommandEvent& event)
{
	wxFileDialog fileSaveDlg(this, "Export Meshes", wxEmptyString, wxEmptyString, "OBJ File (*.OBJ)|*.OBJ", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (fileSaveDlg.ShowModal() == wxID_OK)
	{
		wxBusyCursor busyCursor;

		wxArrayString errorArray;
		wxString filePath = fileSaveDlg.GetPath();
		FileFormat* fileFormat = this->FindFileFormat(filePath);
		if (!fileFormat)
			errorArray.push_back("File format not yet supported: " + filePath);
		else
		{
			std::vector<FileObject*> fileObjectArray;
			EditorApp::Get()->scene->GetAllFileObjects(fileObjectArray);
			if (fileObjectArray.size() == 0)
				errorArray.push_back("Nothing was found in the scene to export.");
			else if (!fileFormat->Save((const char*)filePath.c_str(), fileObjectArray))
				errorArray.push_back("Failed to save file: " + filePath);
		}

		this->ShowErrorMessage(errorArray);
		this->Refresh();
	}
}

void EditorFrame::ShowErrorMessage(const wxArrayString& errorArray)
{
	if (errorArray.size() > 0)
	{
		wxString errorMsg = wxString::Format("Encountered %d error(s)...\n\n", errorArray.size());
		for (int i = 0; i < (int)errorArray.size(); i++)
			errorMsg += errorArray[i] + "\n";

		wxMessageBox(errorMsg, "Error!", wxICON_ERROR, this);
	}
}

FileFormat* EditorFrame::FindFileFormat(const wxString& filePath)
{
	wxFileName fileName(filePath);
	wxString ext = fileName.GetExt().Lower();

	for (FileFormat* fileFormat : this->fileFormatArray)
	{
		wxString supportedExt = wxString(fileFormat->SupportedExtension().c_str()).Lower();
		if (supportedExt == ext)
			return fileFormat;
	}

	return nullptr;
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