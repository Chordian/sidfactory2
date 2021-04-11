#include "foundation/graphics/viewport.h"
#include "foundation/base/assert.h"
#include "foundation/base/types.h"
#include "foundation/graphics/drawfield.h"
#include "foundation/graphics/image.h"
#include "foundation/graphics/imanaged.h"
#include "foundation/graphics/textfield.h"
#include "resources/data_char.h"
#include <iostream>

namespace Foundation
{
	Viewport::Viewport(int inResolutionX, int inResolutionY, float inScaling, const std::string& inCaption)
		: m_ClientResolutionX(inResolutionX)
		, m_ClientResolutionY(inResolutionY)
		, m_Scaling(inScaling)
		, m_ClientX(0)
		, m_ClientY(0)
		, m_Window(nullptr)
		, m_Renderer(nullptr)
		, m_RenderTarget(nullptr)
		, m_ShowOverlay(false)
		, m_Caption(inCaption)
		, m_FadeValue(0.0f)
	{

		const int window_width = static_cast<int>(m_ClientResolutionX * m_Scaling);
		const int window_height = static_cast<int>(m_ClientResolutionY * m_Scaling);

		m_Window = SDL_CreateWindow(inCaption.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_SHOWN);
		FOUNDATION_ASSERT(m_Window != nullptr);

		m_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
		FOUNDATION_ASSERT(m_Renderer != nullptr);
		SDL_RenderSetLogicalSize(m_Renderer, m_ClientResolutionX, m_ClientResolutionY);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

		m_RenderTarget = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, m_ClientResolutionX, m_ClientResolutionY);
		FOUNDATION_ASSERT(m_RenderTarget != nullptr);
	}


	Viewport::~Viewport()
	{
		for (IManaged* text_field : m_ManagedResources)
			delete text_field;

		if (m_RenderTarget != nullptr)
			SDL_DestroyTexture(m_RenderTarget);
		for (auto overlay : m_OverlayList)
		{
			if (overlay.m_Texture != nullptr)
				SDL_DestroyTexture(overlay.m_Texture);
		}

		SDL_DestroyRenderer(m_Renderer);
		SDL_DestroyWindow(m_Window);
	}


	int Viewport::GetClientWidth() const
	{
		return m_ClientResolutionX;
	}


	int Viewport::GetClientHeight() const
	{
		return m_ClientResolutionY;
	}


	void Viewport::SetClientPositionInWindow(const Point& inClientPosition)
	{
		m_ClientX = inClientPosition.m_X;
		m_ClientY = inClientPosition.m_Y;
	}


	Rect Viewport::GetClientRectInWindow() const
	{
		return Rect(m_ClientX, m_ClientY, m_ClientResolutionX, m_ClientResolutionY);
	}


	Point Viewport::GetWindowPosition() const
	{
		Point position;

		SDL_GetWindowPosition(m_Window, &position.m_X, &position.m_Y);
		return position;
	}


	void Viewport::SetWindowPosition(const Point& inPosition)
	{
		SDL_SetWindowPosition(m_Window, inPosition.m_X, inPosition.m_Y);
	}


	Extent Viewport::GetWindowSize() const
	{
		Extent dimensions;

		SDL_GetWindowSize(m_Window, &dimensions.m_Width, &dimensions.m_Height);
		return dimensions;
	}

	void Viewport::SetWindowSize(const Extent& inSize)
	{
		const int window_width = static_cast<int>(inSize.m_Width * m_Scaling);
		const int window_height = static_cast<int>(inSize.m_Height * m_Scaling); 

		SDL_SetWindowSize(m_Window, window_width, window_height);
		SDL_RenderSetLogicalSize(m_Renderer, inSize.m_Width, inSize.m_Height);
	}


	void Viewport::SetFadeValue(float inFadeValue)
	{
		m_FadeValue = inFadeValue;
	}


	void Viewport::SetAdditionTitleInfo(const std::string& inAdditionTitleInfo)
	{
		if (inAdditionTitleInfo.empty())
			SDL_SetWindowTitle(m_Window, m_Caption.c_str());
		else
		{
			std::string title_with_addition_info = inAdditionTitleInfo + " - " + m_Caption;
			SDL_SetWindowTitle(m_Window, title_with_addition_info.c_str());
		}
	}


	void Viewport::ShowOverlay(bool inShowOverlay)
	{
		m_ShowOverlay = inShowOverlay;
	}


	void Viewport::SetOverlayPNG(int inIndex, void* inData, const Rect& inImageRect)
	{
		if (inIndex >= static_cast<int>(m_OverlayList.size()))
			m_OverlayList.resize(inIndex + 1);

		Overlay& overlay = m_OverlayList[inIndex];

		if (overlay.m_Texture != nullptr)
			SDL_DestroyTexture(overlay.m_Texture);

		unsigned int mask_r = 0x000000ff;
		unsigned int mask_g = 0x0000ff00;
		unsigned int mask_b = 0x00ff0000;
		unsigned int mask_a = 0xff000000;

		int depth = 32;
		int pitch = inImageRect.m_Dimensions.m_Width * 4;

		SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(inData, inImageRect.m_Dimensions.m_Width, inImageRect.m_Dimensions.m_Height, depth, pitch, mask_r, mask_g, mask_b, mask_a);

		overlay.m_Texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
		overlay.m_Rect = inImageRect;

		SDL_FreeSurface(surface);
	}


	void Viewport::Begin()
	{
		if (m_RenderTarget != nullptr)
			SDL_SetRenderTarget(m_Renderer, m_RenderTarget);

		SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
		SDL_RenderClear(m_Renderer);

		for (auto text_field : m_ManagedResources)
			text_field->Begin();
	}


	void Viewport::End()
	{
		for (auto text_field : m_ManagedResources)
			text_field->End();

		if (m_RenderTarget != nullptr)
		{
			SDL_SetRenderTarget(m_Renderer, nullptr);

			if (m_ShowOverlay)
			{
				for (const auto& overlay : m_OverlayList)
				{
					SDL_Rect overlay_destination_rect = {
						overlay.m_Rect.m_Position.m_X,
						overlay.m_Rect.m_Position.m_Y,
						overlay.m_Rect.m_Dimensions.m_Width,
						overlay.m_Rect.m_Dimensions.m_Height
					};

					SDL_RenderCopy(m_Renderer, overlay.m_Texture, nullptr, &overlay_destination_rect);
				}
			}

			SDL_Rect client_destination_rect = { m_ClientX, m_ClientY, m_ClientResolutionX, m_ClientResolutionY };
			SDL_RenderCopy(m_Renderer, m_RenderTarget, nullptr, &client_destination_rect);
		}

		if (m_FadeValue < 1.0f)
		{
			Extent dimensions;

			SDL_GetWindowSize(m_Window, &dimensions.m_Width, &dimensions.m_Height);

			SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, static_cast<unsigned char>(255.0f * (1.0f - m_FadeValue)));
			SDL_Rect rect = { 0, 0, dimensions.m_Width, dimensions.m_Height };
			SDL_SetRenderDrawBlendMode(m_Renderer, SDL_BLENDMODE_BLEND);
			SDL_RenderFillRect(m_Renderer, &rect);
		}

		SDL_RenderPresent(m_Renderer);
	}


	void Viewport::SetUserColor(unsigned char inUserColorIndex, unsigned int inARGB)
	{
		m_Palette.SetUserColor(inUserColorIndex, inARGB);
	}


	const Palette& Viewport::GetPalette() const
	{
		return m_Palette;
	}


	TextField* Viewport::CreateTextField(unsigned inWidth, unsigned int inHeight, int inX, int inY)
	{
		TextField* text_field = new TextField(*this, m_Renderer, inWidth, inHeight, inX, inY);

		m_ManagedResources.push_back(text_field);
		return text_field;
	}


	DrawField* Viewport::CreateDrawField(unsigned inWidth, unsigned int inHeight, int inX, int inY)
	{
		DrawField* draw_field = new DrawField(*this, m_Renderer, inWidth, inHeight, inX, inY);

		m_ManagedResources.push_back(draw_field);
		return draw_field;
	}


	Image* Viewport::CreateImageFromFile(const std::string& inFileName)
	{
		SDL_Surface* surface = SDL_LoadBMP(inFileName.c_str());

		if (surface != nullptr)
		{
			Image* image = new Image(*this, m_Renderer, surface);

			m_ManagedResources.push_back(image);
			return image;
		}

		return nullptr;
	}


	Image* Viewport::CreateImageFromARGBData(void* inData, unsigned int inWidth, unsigned int inHeight, bool inIncludeAlphaChannel)
	{
		unsigned int mask_r = 0x000000ff;
		unsigned int mask_g = 0x0000ff00;
		unsigned int mask_b = 0x00ff0000;
		unsigned int mask_a = 0xff000000;

		int depth = inIncludeAlphaChannel ? 32 : 24;
		int pitch = inWidth * (inIncludeAlphaChannel ? 4 : 3);

		SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(inData, inWidth, inHeight, depth, pitch, mask_r, mask_g, mask_b, mask_a);
		Image* image = new Image(*this, m_Renderer, surface);

		m_ManagedResources.push_back(image);
		return image;
	}


	void Viewport::Destroy(IManaged* inManaged)
	{
		auto it = m_ManagedResources.begin();

		while (it != m_ManagedResources.end())
		{
			if (*it == inManaged)
			{
				m_ManagedResources.erase(it);
				delete inManaged;

				return;
			}

			++it;
		}
	}
}
