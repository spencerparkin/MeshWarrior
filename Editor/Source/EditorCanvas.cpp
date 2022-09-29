#include "EditorCanvas.h"
#include "EditorApp.h"
#include "EditorScene.h"
#include <gl/GLU.h>

using namespace MeshWarrior;

int EditorCanvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

EditorCanvas::EditorCanvas(wxWindow* parent) : wxGLCanvas(parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize)
{
	this->context = nullptr;

	this->Bind(wxEVT_PAINT, &EditorCanvas::OnPaint, this);
	this->Bind(wxEVT_SIZE, &EditorCanvas::OnSize, this);
	this->Bind(wxEVT_MOTION, &EditorCanvas::OnMouseMotion, this);
	this->Bind(wxEVT_RIGHT_DOWN, &EditorCanvas::OnRightMouseButtonDown, this);
	this->Bind(wxEVT_RIGHT_UP, &EditorCanvas::OnRightMouseButtonUp, this);
	this->Bind(wxEVT_MOUSE_CAPTURE_LOST, &EditorCanvas::OnCaptureLost, this);
	this->Bind(wxEVT_KEY_DOWN, &EditorCanvas::OnKeyDown, this);
	this->Bind(wxEVT_KEY_UP, &EditorCanvas::OnKeyUp, this);

	this->cameraPosition = Vector(0.0, 0.0, 100.0);
	this->cameraPitch = 0.0;
	this->cameraYaw = 0.0;
	this->cameraTranslationRate = 20.0f;
	this->cameraRotationSensativity = 0.01f;
	this->cameraMoveFlags = 0;
	this->adjustingCameraLookDirection = false;
}

/*virtual*/ EditorCanvas::~EditorCanvas()
{
	delete this->context;
}

void EditorCanvas::BindContext()
{
	if (!this->context)
		this->context = new wxGLContext(this);

	this->SetCurrent(*this->context);
}

void EditorCanvas::OnPaint(wxPaintEvent& event)
{
	this->BindContext();
	this->Render(GL_RENDER);
	this->SwapBuffers();
}

void EditorCanvas::CalcCameraTransform(Transform& cameraTransform) const
{
	cameraTransform.matrix.SetIdentity();
	cameraTransform.translation = this->cameraPosition;

	Matrix3x3 yawMatrix, pitchMatrix;

	yawMatrix.SetFromAxisAngle(Vector(0.0, 1.0, 0.0), this->cameraYaw);
	pitchMatrix.SetFromAxisAngle(Vector(1.0, 0.0, 0.0), this->cameraPitch);

	cameraTransform.matrix.SetProduct(cameraTransform.matrix, yawMatrix);
	cameraTransform.matrix.SetProduct(cameraTransform.matrix, pitchMatrix);
}

/*static*/ void EditorCanvas::MakeOpenGLMatrix(const Transform& transform, GLdouble* matrix)
{
	matrix[0] = transform.matrix.ele[0][0];
	matrix[1] = transform.matrix.ele[1][0];
	matrix[2] = transform.matrix.ele[2][0];
	matrix[3] = 0.0;
	matrix[4] = transform.matrix.ele[0][1];
	matrix[5] = transform.matrix.ele[1][1];
	matrix[6] = transform.matrix.ele[2][1];
	matrix[7] = 0.0;
	matrix[8] = transform.matrix.ele[0][2];
	matrix[9] = transform.matrix.ele[1][2];
	matrix[10] = transform.matrix.ele[2][2];
	matrix[11] = 0.0;
	matrix[12] = transform.translation.x;
	matrix[13] = transform.translation.y;
	matrix[14] = transform.translation.z;
	matrix[15] = 1.0;
}

void EditorCanvas::Render(GLenum renderMode)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	double aspectRatio = double(viewport[2]) / double(viewport[3]);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, aspectRatio, 0.5, 1000.0);

	Transform cameraTransform;
	this->CalcCameraTransform(cameraTransform);

	Transform cameraTransformInv;
	cameraTransform.GetInverse(cameraTransformInv);

	GLdouble viewMat[16];
	this->MakeOpenGLMatrix(cameraTransformInv, viewMat);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(viewMat);

	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glLineWidth(2.0f);
	glBegin(GL_LINES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(10.0f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 10.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 10.0f);

	glEnd();

	EditorScene* scene = EditorApp::Get()->scene;
	if (scene)
		scene->Render(renderMode);

	glFlush();
}

void EditorCanvas::OnSize(wxSizeEvent& event)
{
	this->BindContext();

	wxSize size = event.GetSize();
	glViewport(0, 0, size.GetWidth(), size.GetHeight());

	this->Refresh();
}

void EditorCanvas::Tick(double deltaTime)
{
	Vector cameraDelta(0.0, 0.0, 0.0);

	if ((this->cameraMoveFlags & MW_CAMERA_MOVE_FLAG_LEFT) != 0)
		cameraDelta += Vector(-1.0, 0.0, 0.0);
	if ((this->cameraMoveFlags & MW_CAMERA_MOVE_FLAG_RIGHT) != 0)
		cameraDelta += Vector(1.0, 0.0, 0.0);
	if ((this->cameraMoveFlags & MW_CAMERA_MOVE_FLAG_UP) != 0)
		cameraDelta += Vector(0.0, 0.0, -1.0);
	if ((this->cameraMoveFlags & MW_CAMERA_MOVE_FLAG_DOWN) != 0)
		cameraDelta += Vector(0.0, 0.0, 1.0);

	if (cameraDelta.Length() > 0.0)
	{
		Transform cameraTransform;
		this->CalcCameraTransform(cameraTransform);
		cameraDelta =  cameraTransform.TransformVector(cameraDelta);
		this->cameraPosition += cameraDelta * this->cameraTranslationRate * deltaTime;
		this->Refresh();
	}
}

void EditorCanvas::OnMouseMotion(wxMouseEvent& event)
{
	if (this->adjustingCameraLookDirection)
	{
		wxPoint currentMouseDragPos = event.GetPosition();
		wxPoint mouseDragDelta = currentMouseDragPos - this->lastMouseDragPos;
		this->lastMouseDragPos = currentMouseDragPos;

		double yawAngleDelta = -double(mouseDragDelta.x) * this->cameraRotationSensativity;
		double pitchAngleDelta = -double(mouseDragDelta.y) * this->cameraRotationSensativity;

		this->cameraYaw += yawAngleDelta;
		this->cameraPitch += pitchAngleDelta;

		this->Refresh();
	}
}

void EditorCanvas::OnRightMouseButtonDown(wxMouseEvent& event)
{
	this->adjustingCameraLookDirection = true;
	this->lastMouseDragPos = event.GetPosition();
	this->CaptureMouse();
}

void EditorCanvas::OnRightMouseButtonUp(wxMouseEvent& event)
{
	this->adjustingCameraLookDirection = false;
	if (this->HasCapture())
		this->ReleaseMouse();
}

void EditorCanvas::OnCaptureLost(wxMouseCaptureLostEvent& event)
{
	this->adjustingCameraLookDirection = false;
}

void EditorCanvas::OnKeyDown(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
		case 'A':
		case 'a':
		{
			this->cameraMoveFlags |= MW_CAMERA_MOVE_FLAG_LEFT;
			break;
		}
		case 'D':
		case 'd':
		{
			this->cameraMoveFlags |= MW_CAMERA_MOVE_FLAG_RIGHT;
			break;
		}
		case 'W':
		case 'w':
		{
			this->cameraMoveFlags |= MW_CAMERA_MOVE_FLAG_UP;
			break;
		}
		case 'S':
		case 's':
		{
			this->cameraMoveFlags |= MW_CAMERA_MOVE_FLAG_DOWN;
			break;
		}
	}
}

void EditorCanvas::OnKeyUp(wxKeyEvent& event)
{
	switch (event.GetKeyCode())
	{
		case 'A':
		case 'a':
		{
			this->cameraMoveFlags &= ~MW_CAMERA_MOVE_FLAG_LEFT;
			break;
		}
		case 'D':
		case 'd':
		{
			this->cameraMoveFlags &= ~MW_CAMERA_MOVE_FLAG_RIGHT;
			break;
		}
		case 'W':
		case 'w':
		{
			this->cameraMoveFlags &= ~MW_CAMERA_MOVE_FLAG_UP;
			break;
		}
		case 'S':
		case 's':
		{
			this->cameraMoveFlags &= ~MW_CAMERA_MOVE_FLAG_DOWN;
			break;
		}
	}
}