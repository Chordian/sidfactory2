#pragma once

#include <string>
#include <vector>

namespace Foundation
{
	class WrappedString
	{
	public:
		struct Line
		{
			const std::string m_Line;
			const int m_Width;

			Line(const std::string& inLine, int inWidth)
				: m_Line(inLine)
				, m_Width(inWidth)
			{
			}
		};

		WrappedString(const std::string& inSourceString, int inMaxWidth);
		~WrappedString();

		const std::vector<Line>& GetLines() const;

	private:
		std::vector<Line> m_Lines;
	};
}