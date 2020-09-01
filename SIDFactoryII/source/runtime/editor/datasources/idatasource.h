#pragma once

namespace Editor
{
	class IDataSource
	{
	protected:
		IDataSource() { }

	public:
		virtual ~IDataSource() { }

		virtual const int GetSize() const = 0;
		virtual bool PushDataToSource() = 0;
	};
}
