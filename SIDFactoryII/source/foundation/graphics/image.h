#pragma once

#include <string>
#include "SDL.h"

#include "foundation/graphics/color.h"
#include "foundation/graphics/imanaged.h"
#include "foundation/base/types.h"

namespace Foundation
{
	class Viewport;

	class Image final : public IManaged
	{
		friend class Viewport;

	private:
		Image(const Viewport& inViewport, SDL_Renderer* inRenderer, SDL_Surface* inSurface);
		~Image();

	public:
		void Begin() override;
		void End() override;

		Foundation::Extent GetDimensions() const;
		Foundation::Point GetPosition() const;

		void SetPosition(const Foundation::Point& inPosition);

	private:
		const Viewport& m_Viewport;

		Point m_Position;

		int m_Width;
		int m_Height;

		SDL_Renderer* m_Renderer;
		SDL_Texture* m_Image;
	};
}