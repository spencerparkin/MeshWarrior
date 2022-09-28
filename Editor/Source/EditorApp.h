#pragma once

#include <wx/app.h>

namespace MeshWarrior
{
	class EditorFrame;
	class EditorScene;

	class EditorApp : public wxApp
	{
	public:
		EditorApp();
		virtual ~EditorApp();

		virtual bool OnInit(void) override;
		virtual int OnExit(void) override;

		static EditorApp* Get()
		{
			return wxDynamicCast(wxApp::GetInstance(), EditorApp);
		}

		EditorFrame* frame;
		EditorScene* scene;
	};

	wxDECLARE_APP(EditorApp);
}