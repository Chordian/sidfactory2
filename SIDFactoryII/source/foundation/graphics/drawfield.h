#pragma once

#include "SDL.h"

#include "foundation/graphics/color.h"
#include "foundation/graphics/imanaged.h"
#include "foundation/base/types.h"
#include "utils/bit_array.h"

namespace Foundation
{
	class Viewport;

	class DrawField final : public IManaged
	{
	public:
		DrawField(const Viewport& inViewport, SDL_Renderer* inRenderer, int inWidth, int inHeight, int inX, int inY);
		~DrawField();

		void SetEnable(bool inEnable);
		bool IsEnabled() const;

		void Begin() override;
		void End() override;

		const Extent& GetDimensions() const;
		const Point& GetPosition() const;
		void SetPosition(const Point& inPosition);

		void Clear(const Color& inColor);

		void DrawDot(const Color& inColor, int inX, int inY);
		void DrawLine(const Color& inColor, int inX1, int inY1, int inX2, int inY2);
		void DrawBox(const Color& inColor, int inTopLeftX1, int inTopLeftY1, int inWidth, int inHeight);

		void DrawVerticalLine(const Color& inColor, int inX, int inY1, int inY2);
		void DrawHorizontalLine(const Color& inColor, int inX1, int inX2, int inY);

	private:
		bool m_Enabled;

		const Viewport& m_Viewport;

		Point m_Position;
		Extent m_Dimensions;
		Rect m_LocalRect;

		SDL_Renderer* m_Renderer;
		SDL_Surface* m_Surface;
		SDL_Texture* m_Texture;
	};
}