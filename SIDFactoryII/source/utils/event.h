#pragma once

#include "delegate.h"

#include <map>
#include <functional>
#include <assert.h>

namespace Utility
{
	template<typename FUNCTION_DESCRIPTION>
	class TEvent
	{
	public:
		TEvent() { }

		void Clear()
		{
			m_Delegates.clear();
		}

		void Add(void* inContext, TDelegate<FUNCTION_DESCRIPTION>&& inDelegate)
		{
			assert(m_Delegates.find(inContext) == m_Delegates.end());
			m_Delegates[inContext] = inDelegate;
		}

		void Remove(void* inContext)
		{
			auto it = m_Delegates.find(inContext);
			assert(it != m_Delegates.end());

			m_Delegates.erase(it);
		}

		template<typename ...ARGS>
		void Execute(ARGS... input)
		{
			for (auto& delegate : m_Delegates)
				delegate.second.Execute(input...);
		}

	private:
		std::map<void*, TDelegate<FUNCTION_DESCRIPTION>> m_Delegates;
	};
}