#pragma once

#include "../EditorPanel.h"

namespace MeshWarrior
{
	class EditorCanvas;

	class CanvasPanel : public EditorPanel
	{
	public:
		wxDECLARE_DYNAMIC_CLASS(CanvasPanel);

		CanvasPanel();
		virtual ~CanvasPanel();

		virtual bool MakePaneInfo(wxAuiPaneInfo& paneInfo) override;
		virtual bool MakeControls(void) override;
		virtual void DoIdleProcessing(void) override;

		EditorCanvas* canvas;
	};
}