#include "image.h"
#include "viewport.h"
#include "resources/data_char.h"

#include "SDL.h"
#include "foundation/base/assert.h"

namespace Foundation
{
	Image::Image(const Viewport& inViewport, SDL_Renderer* inRenderer, SDL_Surface* inSurface)
		: m_Viewport(inViewport)
		, m_Renderer(inRenderer)
		, m_Position({ 0, 0 })
	{
		FOUNDATION_ASSERT(inSurface != nullptr);
		m_Image = SDL_CreateTextureFromSurface(inRenderer, inSurface);
		
		m_Width = inSurface->w;
		m_Height = inSurface->h;

		SDL_FreeSurface(inSurface);
	}

	Image::~Image()
	{
		SDL_DestroyTexture(m_Image);
	}


	Foundation::Extent Image::GetDimensions() const
	{
		return { m_Width, m_Height };
	}


	Foundation::Point Image::GetPosition() const
	{
		return m_Position;
	}

	void Image::SetPosition(const Foundation::Point& inPosition)
	{
		m_Position = inPosition;
	}


	void Image::Begin()
	{
		// Nothing to do
	}


	void Image::End()
	{
		if (m_Image != nullptr)
		{
			SDL_Rect rect;

			rect.x = m_Position.m_X;
			rect.y = m_Position.m_Y;
			rect.w = m_Width;
			rect.h = m_Height;

			SDL_RenderCopy(m_Renderer, m_Image, nullptr, &rect);
		}
	}
}