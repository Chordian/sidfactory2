#include "visualizer_component_base.h"

#include "foundation/graphics/drawfield.h"
#include "foundation/input/mouse.h"

#include <assert.h>

namespace Editor
{
	VisualizerComponentBase::VisualizerComponentBase(int inID, Foundation::DrawField* inDrawField, int inX, int inY, int inWidth, int inHeight)
		: m_ComponentID(inID)
		, m_DrawField(inDrawField)
		, m_Position({ inX, inY })
		, m_Dimensions({ inWidth, inHeight })
		, m_Rect({ m_Position, m_Dimensions })
		, m_Enabled(false)
	{
	}


	const int VisualizerComponentBase::GetComponentID() const
	{
		return m_ComponentID;
	}


	const Foundation::Point& VisualizerComponentBase::GetPosition() const
	{
		return m_Position;
	}


	const Foundation::Extent& VisualizerComponentBase::GetDimensions() const
	{
		return m_Dimensions;
	}


	const Foundation::Rect& VisualizerComponentBase::GetRect() const
	{
		return m_Rect;
	}


	void VisualizerComponentBase::SetEnabled(bool inEnabled)
	{
		m_Enabled = inEnabled;
	}


	const bool VisualizerComponentBase::IsEnabled() const
	{
		return m_Enabled;
	}


	void VisualizerComponentBase::ForceRefresh()
	{
		m_RequireRefresh = true;
	}


	bool VisualizerComponentBase::ContainsPosition(const Foundation::Point& inPixelPosition) const
	{
		return m_Rect.Contains(inPixelPosition);
	}


	const Foundation::Point VisualizerComponentBase::ToLocalDrawFieldPosition(const Foundation::Point& inPosition) const
	{
		return inPosition - m_DrawField->GetPosition();
	}

}