#pragma once

#include <SDL.h>
#include <string>
#include <vector>

#include "foundation/base/types.h"
#include "foundation/graphics/color.h"

namespace Foundation
{
	// Forward declarations
	class IManaged;
	class TextField;
	class DrawField;
	class Image;

	// Class
	class Viewport final
	{
	public:
		Viewport(int inResolutionX, int inResolutionY, float inScaling, const std::string& inCaption);
		~Viewport();

		int GetClientWidth() const;
		int GetClientHeight() const;

		void SetClientPositionInWindow(const Point& inClientPosition);
		Rect GetClientRectInWindow() const;

		Point GetWindowPosition() const;
		void SetWindowPosition(const Point& inPosition);

		Extent GetWindowSize() const;
		void SetWindowSize(const Extent& inSize);

		void SetFadeValue(float inFadeValue);
		void SetAdditionTitleInfo(const std::string& inAdditionTitleInfo);

		void ShowOverlay(bool inShowOverlay);
		void SetOverlayPNG(int inIndex, void* inData, const Rect& inImageRect);
		void SetWindowFullScreen(int flags);

		void Begin();
		void End();

		TextField* CreateTextField(unsigned inWidth, unsigned int inHeight, int inX, int inY);
		DrawField* CreateDrawField(unsigned inWidth, unsigned int inHeight, int inX, int inY);
		Image* CreateImageFromFile(const std::string& inFileName);
		Image* CreateImageFromARGBData(void* inData, unsigned int inWidth, unsigned int inHeight, bool inIncludeAlphaChannel);
		
		SDL_Renderer* GetRenderer();

		void Destroy(IManaged* inManagedResource);

		void SetUserColor(unsigned char inUserColorIndex, unsigned int inARGB);
		const Palette& GetPalette() const;


	private:
		struct Overlay
		{
			Overlay()
				: m_Texture(nullptr)
			{
			}

			SDL_Texture* m_Texture;
			Rect m_Rect;
		};

		const int m_ClientResolutionX;
		const int m_ClientResolutionY;
		const float m_Scaling;

		int m_ClientX;
		int m_ClientY;

		bool m_ShowOverlay;
		float m_FadeValue;

		Palette m_Palette;

		SDL_Window* m_Window;
		SDL_Renderer* m_Renderer;
		SDL_Texture* m_RenderTarget;
		std::vector<Overlay> m_OverlayList;

		std::string m_Caption;

		std::vector<IManaged*> m_ManagedResources;
	};
}
