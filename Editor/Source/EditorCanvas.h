#pragma once

#include <wx/glcanvas.h>

namespace MeshWarrior
{
	class EditorCanvas : public wxGLCanvas
	{
	public:
		EditorCanvas(wxWindow* parent);
		virtual ~EditorCanvas();

		void OnPaint(wxPaintEvent& event);
		void OnSize(wxSizeEvent& event);

		void BindContext();

		wxGLContext* context;
		static int attributeList[];
	};
}