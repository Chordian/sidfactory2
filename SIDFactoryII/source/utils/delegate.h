#pragma once

#include <functional>

namespace Utility
{
	template <typename FUNCTION_DESCRIPTION>
	class TDelegate
	{
		template<typename T>
		friend class TEvent;

	public:
		TDelegate()
		{

		}

		TDelegate(std::function<FUNCTION_DESCRIPTION>&& inFunction)
			: m_Function(inFunction)
		{
		}

	private:
		template<typename ...ARGS>
		void Execute(ARGS... input)
		{
			m_Function(input...);
		}

		std::function<FUNCTION_DESCRIPTION> m_Function;
	};
}
