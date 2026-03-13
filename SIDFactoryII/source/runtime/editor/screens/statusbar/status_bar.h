#pragma once

#include "foundation/graphics/color.h"
#include "foundation/input/mouse.h"
#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace Foundation
{
	class TextField;
}

namespace Editor
{
	class StatusBar
	{
	public:
		StatusBar(Foundation::TextField* inTextField);
		~StatusBar();

		void Clear();
		void SetDirty();

		void SetColors(const Foundation::Color& inTextColor, const Foundation::Color& inBackgroundColor, const Foundation::Color& inBackgroundMouseOverColor);
		void SetText(const std::string& inText);
		void SetText(const std::string& inText, int inDuration, bool inCanBeOverwritten = true);
		bool IsDisplayingTimedText() const;
		void Refresh();

		void Update(int inDeltaTick);

		bool ConsumeInput(const Foundation::Mouse& inMouse, int inKeyboardModifiers);

	protected:
		virtual void UpdateInternal(int inDeltaTick, bool inNeedUpdate);
		virtual void ClearContents();
		virtual void DrawText();
		virtual void DrawTextSectionList();

		class TextSection
		{
		public:
			TextSection(int inWidth);
			TextSection(int inWidth, const std::function<void(Foundation::Mouse::Button, int)>& inMouseButtonCallback);

			void SetText(const std::string& inString);
			const std::string& GetText() const;

			int GetWidth() const;

			bool CaresAboutMouseInput() const;
			void MouseButtonPressed(Foundation::Mouse::Button inMouseButton, int) const;

		private:
			int m_Width;

			std::string m_Text;
			std::function<void(Foundation::Mouse::Button, int)> m_MouseButtonCallback;
		};

		bool m_CanBeOverwritten;
		bool m_NeedRefresh;
		bool m_NeedUpdate;

		std::string m_Text;
		int m_TextClearTimer;

		std::vector<std::shared_ptr<TextSection>> m_TextSectionList;
		int m_MouseOverTextSectionIndex;

		Foundation::TextField* m_TextField;
		Foundation::Color m_TextColor;
		Foundation::Color m_BackgroundColor;
		Foundation::Color m_BackgroundMouseOverColor;
	};
}