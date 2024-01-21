#pragma once

#include <utility>
#include "assert.h"

namespace Foundation
{
	template <typename T>
	class TOptional final
	{
	public:
		TOptional()
			: m_IsValid(false)
		{
		}

		TOptional(const T& InValue)
			: m_IsValid(true)
			, m_Value(InValue)
		{
		}

		TOptional(T&& InValue)
			: m_IsValid(true)
			, m_Value(std::move(InValue))
		{
		}

		TOptional(const TOptional& InOther)
			: m_IsValid(InOther.m_IsValid)
		{
			if(m_IsValid)
				new (&m_Value) T(InOther.m_Value);
		}

		TOptional(TOptional&& InOther)
			: m_IsValid(InOther.m_IsValid)
		{
			if (m_IsValid)
				new (&m_Value) T(std::move(InOther.m_Value));
		}

		TOptional& operator=(const TOptional& InOther)
		{
			if (m_IsValid)
				m_Value.~T();

			m_IsValid = InOther.m_IsValid;
			if (m_IsValid)
				new (&m_Value) T(InOther.m_Value);

			return *this;
		}

		TOptional& operator=(TOptional&& InOther)
		{
			m_IsValid = InOther.m_IsValid;
			InOther.IsValid = false;

			if (m_IsValid)
				new (&m_Value) T(std::move(InOther.m_Value));

			return *this;
		}

		~TOptional()
		{
			if (m_IsValid)
				m_Value.~T();
		}

		bool IsValid() const
		{
			return m_IsValid;
		}

		const T& Get() const
		{
			FOUNDATION_ASSERT(m_IsValid);
			return m_Value;
		}

		T& Get()
		{
			FOUNDATION_ASSERT(m_IsValid);
			return m_Value;
		}

	private:
		bool m_IsValid;
		T m_Value;
	};
}