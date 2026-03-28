#include "wrapped_string.h"

namespace Foundation
{
	WrappedString::WrappedString(const std::string& inSourceString, int inMaxWidth)
	{
		const size_t source_length = inSourceString.length();

		if (source_length == 0)
			return;

		const size_t max_width = inMaxWidth > 0 ? static_cast<size_t>(inMaxWidth) : 1;

		size_t from = 0;
		while (from < source_length)
		{
			size_t next_newline = inSourceString.find("\n", from);
			size_t segment_end = (next_newline == std::string::npos) ? source_length : next_newline;

			if (segment_end == from)
			{
				m_Lines.push_back(Line("", 0));
			}
			else
			{
				size_t segment_from = from;
				while (segment_from < segment_end)
				{
					size_t remaining = segment_end - segment_from;

					if (remaining <= max_width)
					{
						m_Lines.push_back(Line(inSourceString.substr(segment_from, remaining), static_cast<int>(remaining)));
						segment_from = segment_end;
					}
					else
					{
						size_t search_limit = segment_from + max_width;
						size_t last_space = inSourceString.find_last_of(" ", search_limit);

						if (last_space == std::string::npos || last_space < segment_from)
						{
							m_Lines.push_back(Line(inSourceString.substr(segment_from, max_width), static_cast<int>(max_width)));
							segment_from += max_width;
						}
						else
						{
							size_t line_length = last_space - segment_from;
							m_Lines.push_back(Line(inSourceString.substr(segment_from, line_length), static_cast<int>(line_length)));
							segment_from = last_space + 1;
						}
					}
				}
			}

			if (next_newline == std::string::npos)
				break;

			from = next_newline + 1;
			if (from == source_length)
				m_Lines.push_back(Line("", 0));
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
