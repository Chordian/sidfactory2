#pragma once

#include "datacopy_orderlist.h"
#include "datacopy_sequence.h"
#include "foundation/base/assert.h"
#include <memory>
#include <type_traits>

namespace Editor
{
	class CopyPaste
	{
	public:
		static CopyPaste& Instance();

		void Flush();

		template<typename T>
		bool HasData() const;
		
		template<typename T>
		const T* GetData() const;

		template<typename T>
		void SetData(T* inData);

	private:
		CopyPaste();

		std::unique_ptr<DataCopySequence> m_SequenceData;
		std::unique_ptr<DataCopyOrderList> m_OrderListData;

		template<typename T>
		struct FalseT : std::false_type
		{
		};
	};


	template<typename T>
	bool CopyPaste::HasData() const
	{
		static_assert(FalseT<T>::value, "Implement data type specialization");
		return false;
	}


	template<typename T>
	const T* CopyPaste::GetData() const
	{
		static_assert(FalseT<T>::value, "Implement data type specialization");
		return nullptr;
	}

	template<typename T>
	void CopyPaste::SetData(T* inData)
	{
		static_assert(FalseT<T>::value, "Implement data type specialization");
	}


	// Template specializations
	template<>
	inline bool CopyPaste::HasData<DataCopySequence>() const
	{
		return m_SequenceData != nullptr;
	}

	template<>
	inline bool CopyPaste::HasData<DataCopyOrderList>() const
	{
		return m_OrderListData != nullptr;
	}


	template<>
	inline const DataCopySequence* CopyPaste::GetData<DataCopySequence>() const
	{
		FOUNDATION_ASSERT(m_SequenceData != nullptr);
		return m_SequenceData.get();
	}

	template<>
	inline const DataCopyOrderList* CopyPaste::GetData<DataCopyOrderList>() const
	{
		FOUNDATION_ASSERT(m_OrderListData != nullptr);
		return m_OrderListData.get();
	}


	template<>
	inline void CopyPaste::SetData<DataCopySequence>(DataCopySequence* inData)
	{
		Flush();

		m_SequenceData = std::unique_ptr<DataCopySequence>(inData);
	}

	template<>
	inline void CopyPaste::SetData<DataCopyOrderList>(DataCopyOrderList* inData)
	{
		Flush();

		m_OrderListData = std::unique_ptr<DataCopyOrderList>(inData);
	}
}