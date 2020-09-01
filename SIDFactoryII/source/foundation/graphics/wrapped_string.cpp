#include "wrapped_string.h"

namespace Foundation
{
	WrappedString::WrappedString(const std::string& inSourceString, int inMaxWidth)
	{
		const size_t source_length = inSourceString.length();
		
		size_t from = 0;
		size_t pos = inSourceString.find(" ", from);

		while (true)
		{
			if (pos == std::string::npos)
			{
				if (m_Lines.empty() && source_length > 0)
					m_Lines.push_back(Line(inSourceString, static_cast<int>(source_length)));
				break;
			}

			if (pos - from > static_cast<size_t>(inMaxWidth))
			{
				m_Lines.push_back(Line(inSourceString.substr(from, pos - from), static_cast<int>(pos - from)));

				from = pos + 1;
				pos = inSourceString.find(" ", from);
			}
			else
			{
				size_t next_pos = inSourceString.find(" ", pos + 1);
				size_t line_break_pos = inSourceString.find("\n", pos + 1);

				if (line_break_pos != std::string::npos && line_break_pos < next_pos)
				{
					m_Lines.push_back(Line(inSourceString.substr(from, line_break_pos - from), static_cast<int>(line_break_pos - from)));
					from = line_break_pos + 1;
					pos = from;
				}

				if (next_pos == std::string::npos)
					next_pos = source_length;

				if (next_pos - from > static_cast<size_t>(inMaxWidth))
				{
					if (pos > from)
					{
						m_Lines.push_back(Line(inSourceString.substr(from, pos - from), static_cast<int>(pos - from)));
						from = pos + 1;
					}
					else
					{
						m_Lines.push_back(Line(inSourceString.substr(from, inMaxWidth), inMaxWidth));
						from += inMaxWidth;
						pos = from;
					}
				}

				if (next_pos == source_length)
				{
					if (next_pos > from)
						m_Lines.push_back(Line(inSourceString.substr(from, next_pos - from), static_cast<int>(next_pos - from)));

					break;
				}

				pos = next_pos;
			}
		}
	}

	WrappedString::~WrappedString()
	{
	}

	const std::vector<WrappedString::Line>& WrappedString::GetLines() const
	{
		return m_Lines;
	}
}
