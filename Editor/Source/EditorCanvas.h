#pragma once

#include <wx/glcanvas.h>
#include "Transform.h"

#define MW_CAMERA_MOVE_FLAG_UP		0x0001
#define MW_CAMERA_MOVE_FLAG_DOWN	0x0002
#define MW_CAMERA_MOVE_FLAG_LEFT	0x0004
#define MW_CAMERA_MOVE_FLAG_RIGHT	0x0008

namespace MeshWarrior
{
	class EditorCanvas : public wxGLCanvas
	{
	public:
		EditorCanvas(wxWindow* parent);
		virtual ~EditorCanvas();

		void OnPaint(wxPaintEvent& event);
		void OnSize(wxSizeEvent& event);
		void OnMouseMotion(wxMouseEvent& event);
		void OnRightMouseButtonDown(wxMouseEvent& event);
		void OnRightMouseButtonUp(wxMouseEvent& event);
		void OnCaptureLost(wxMouseCaptureLostEvent& event);
		void OnKeyDown(wxKeyEvent& event);
		void OnKeyUp(wxKeyEvent& event);

		void BindContext();
		void Render(GLenum renderMode);
		void Tick(double deltaTime);

		void CalcCameraTransform(Transform& cameraTransform) const;

		static void MakeOpenGLMatrix(const Transform& transform, GLdouble* matrix);

		wxGLContext* context;
		static int attributeList[];
		Vector cameraPosition;
		double cameraPitch;
		double cameraYaw;
		double cameraTranslationRate;
		double cameraRotationSensativity;
		short cameraMoveFlags;
		bool adjustingCameraLookDirection;
		wxPoint lastMouseDragPos;
	};
}