#include "status_bar.h"

#include "foundation/graphics/textfield.h"
#include "foundation/input/mouse.h"
#include "foundation/graphics/color.h"
#include "utils/usercolors.h"
#include <assert.h>

using namespace Utility;
using namespace Foundation;

namespace Editor
{
	StatusBar::StatusBar(Foundation::TextField* inTextField) 
		: m_TextField(inTextField)
		, m_NeedRefresh(true)
		, m_TextColor(ToColor(UserColor::StatusBarText))
		, m_BackgroundColor(ToColor(UserColor::StatusBarBackgroundStopped))
		, m_BackgroundMouseOverColor(Foundation::Color::LightBlue)
		, m_TextClearTimer(0)
		, m_MouseOverTextSectionIndex(-1)
		, m_NeedUpdate(true)
	{

	}


	StatusBar::~StatusBar()
	{

	}


	void StatusBar::Clear()
	{
		ClearContents();
		m_NeedRefresh = true;
	}


	void StatusBar::SetDirty()
	{
		m_NeedRefresh = true;
	}


	void StatusBar::SetColors(const Foundation::Color& inTextColor, const Foundation::Color& inBackgroundColor, const Foundation::Color& inBackgroundMouseOverColor)
	{
		m_TextColor = inTextColor;
		m_BackgroundColor = inBackgroundColor;
		m_BackgroundMouseOverColor = inBackgroundMouseOverColor;
		m_NeedRefresh = true;
	}


	void StatusBar::SetText(const std::string& inText)
	{
		SetText(inText, 0);
	}


	void StatusBar::SetText(const std::string& inText, int inDuration)
	{
		m_Text = inText;
		m_TextClearTimer = inDuration;
		m_NeedRefresh = true;
	}

	bool StatusBar::IsDisplayingTimedText() const
	{
		return m_TextClearTimer > 0;
	}

	void StatusBar::Refresh()
	{
		if (m_NeedRefresh)
		{
			const int width = m_TextField->GetDimensions().m_Width;
			const Foundation::Rect rect = { { 0, 0 }, { width, 1} };

			m_TextField->Clear(rect);
			m_TextField->ColorAreaBackground(m_BackgroundColor, rect);

			DrawText();
			DrawTextSectionList();

			m_NeedRefresh = false;
		}
	}


	bool StatusBar::ConsumeInput(const Foundation::Mouse& inMouse, int inKeyboardModifiers)
	{
		Foundation::Point cell_position = m_TextField->GetCellPositionFromPixelPosition(inMouse.GetPosition());

		int mouse_over_index = -1;

		if (cell_position.m_Y == 0)
		{
			int left = m_TextField->GetDimensions().m_Width;

			for (int i = 0; i < static_cast<int>(m_TextSectionList.size()); ++i)
			{
				auto& text_section = m_TextSectionList[i];

				int right = left;
				left -= text_section->GetWidth();

				if (cell_position.m_X < right && cell_position.m_X >= left)
				{
					mouse_over_index = i;
					break;
				}
			}
		}

		if (mouse_over_index != m_MouseOverTextSectionIndex)
		{
			m_MouseOverTextSectionIndex = mouse_over_index;
			m_NeedRefresh = true;
		}


		if (m_MouseOverTextSectionIndex >= 0 && m_TextSectionList[m_MouseOverTextSectionIndex]->CaresAboutMouseInput())
		{
			if (inMouse.IsButtonPressed(Foundation::Mouse::Button::Left))
			{
				m_TextSectionList[m_MouseOverTextSectionIndex]->MouseButtonPressed(Foundation::Mouse::Button::Left, inKeyboardModifiers);
				return true;
			}
			if (inMouse.IsButtonPressed(Foundation::Mouse::Button::Right))
			{
				m_TextSectionList[m_MouseOverTextSectionIndex]->MouseButtonPressed(Foundation::Mouse::Button::Right, inKeyboardModifiers);
				return true;
			}
		}

		return false;
	}


	void StatusBar::Update(int inDeltaTick)
	{
		if (m_TextClearTimer > 0)
		{
			m_TextClearTimer -= inDeltaTick;

			if (m_TextClearTimer <= 0)
			{
				m_Text = "";
				m_NeedRefresh = true;
			}
		}

		UpdateInternal(inDeltaTick, m_NeedUpdate);

		m_NeedUpdate = false;
	}


	void StatusBar::UpdateInternal(int inDeltaTick, bool inNeedUpdate)
	{

	}


	void StatusBar::ClearContents()
	{
		m_Text = "";
	}


	void StatusBar::DrawText()
	{
		m_TextField->Print(0, 0, m_TextColor, m_Text);
	}


	void StatusBar::DrawTextSectionList()
	{
		int left = m_TextField->GetDimensions().m_Width;

		for (int i = 0; i < static_cast<int>(m_TextSectionList.size()); ++i)
		{
			auto& text_section = m_TextSectionList[i];

			int right = left;
			left -= text_section->GetWidth();

			if (i == m_MouseOverTextSectionIndex)
				m_TextField->ColorAreaBackground(m_BackgroundMouseOverColor, left, 0, right - left, 1);

			m_TextField->Print(left + 1, 0, m_TextColor, text_section->GetText());
		}
	}


	//-----------------------------------------------------------------------------------------------------------

	StatusBar::TextSection::TextSection(int inWidth)
		: m_Width(inWidth)
	{
	}


	StatusBar::TextSection::TextSection(int inWidth, const std::function<void(Foundation::Mouse::Button, int)>& inMouseButtonCallback)
		: m_Width(inWidth)
		, m_MouseButtonCallback(inMouseButtonCallback)
	{
	}


	void StatusBar::TextSection::SetText(const std::string& inText)
	{
		assert(static_cast<int>(inText.length()) < m_Width);
		m_Text = inText;
	}


	const std::string& StatusBar::TextSection::GetText() const
	{
		return m_Text;
	}


	int StatusBar::TextSection::GetWidth() const
	{
		return m_Width;
	}


	bool StatusBar::TextSection::CaresAboutMouseInput() const
	{
		if (m_MouseButtonCallback)
			return true;

		return false;
	}


	void StatusBar::TextSection::MouseButtonPressed(Foundation::Mouse::Button inMouseButton, int inKeyboardModifiers) const
	{
		assert(m_MouseButtonCallback);
		m_MouseButtonCallback(inMouseButton, inKeyboardModifiers);
	}
}