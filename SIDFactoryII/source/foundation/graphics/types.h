#pragma once

namespace Foundation
{
	struct Point
	{
		Point()
			: m_X(0)
			, m_Y(0)
		{ 
		}

		Point(int inX, int inY)
			: m_X(inX)
			, m_Y(inY)
		{
		}

		Point operator+(const Point& inOtherPoint) const
		{
			return Point(m_X + inOtherPoint.m_X, m_Y + inOtherPoint.m_Y);
		}

		Point operator-(const Point& inOtherPoint) const
		{
			return Point(m_X - inOtherPoint.m_X, m_Y - inOtherPoint.m_Y);
		}

		Point& operator+=(const Point& inOtherPoint)
		{
			m_X += inOtherPoint.m_X;
			m_Y += inOtherPoint.m_Y;

			return *this;
		}

		Point& operator-=(const Point& inOtherPoint)
		{
			m_X -= inOtherPoint.m_X;
			m_Y -= inOtherPoint.m_Y;

			return *this;
		}

		int m_X;
		int m_Y;
	};

	struct Extent
	{
		const bool Contains(const Point& inPoint) const
		{
			return inPoint.m_X >= 0 
				&& inPoint.m_Y >= 0 
				&& inPoint.m_X < m_Width 
				&& inPoint.m_Y < m_Height;
		}

		int m_Width;
		int m_Height;
	};

	struct Rect
	{
		Rect()
		{
		}

		Rect(const Point& inPoint, const Extent& inDimensions)
			: m_Position(inPoint)
			, m_Dimensions(inDimensions)
		{
		}

		Rect(int inX, int inY, int inWidth, int inHeight)
			: m_Position({inX, inY})
			, m_Dimensions({inWidth, inHeight})
		{
		}

		const bool Contains(const Point& inPoint) const
		{
			return inPoint.m_X - m_Position.m_X >= 0 
				&& inPoint.m_Y - m_Position.m_Y >= 0 
				&& inPoint.m_X - m_Position.m_X < m_Dimensions.m_Width 
				&& inPoint.m_Y - m_Position.m_Y < m_Dimensions.m_Height;
		}

		Point m_Position;
		Extent m_Dimensions;
	};
}