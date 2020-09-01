#pragma once

#include "foundation/graphics/types.h"
#include <functional>

namespace Foundation
{
	class Mouse;
	class DrawField;
}

namespace Editor
{
	class DisplayState;

	class VisualizerComponentBase
	{
	protected:
		VisualizerComponentBase() = delete;
		VisualizerComponentBase(int inID, Foundation::DrawField* inDrawField, int inX, int inY, int inWidth, int inHeight);

	public:
		const int GetComponentID() const;
		
		const Foundation::Point& GetPosition() const;
		const Foundation::Extent& GetDimensions() const;
		const Foundation::Rect& GetRect() const;

		void SetEnabled(bool inEnabled);
		const bool IsEnabled() const;

		virtual void ForceRefresh();

		virtual void ConsumeNonExclusiveInput(const Foundation::Mouse& inMouse) = 0;
		virtual void Refresh(const DisplayState& inDisplayState) = 0;

		bool ContainsPosition(const Foundation::Point& inPixelPosition) const;

	protected:
		const Foundation::Point ToLocalDrawFieldPosition(const Foundation::Point& inPosition) const;

		const int m_ComponentID;

		bool m_Enabled;

		Foundation::Point m_Position;
		Foundation::Extent m_Dimensions;
		Foundation::Rect m_Rect;

		Foundation::DrawField* m_DrawField;

		bool m_RequireRefresh;
	};
}