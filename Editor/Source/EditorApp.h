#pragma once

#include <wx/app.h>

namespace MeshWarrior
{
	class EditorFrame;

	class EditorApp : public wxApp
	{
	public:
		EditorApp();
		virtual ~EditorApp();

		virtual bool OnInit(void) override;
		virtual int OnExit(void) override;

		EditorFrame* frame;
	};

	wxDECLARE_APP(EditorApp);
}