#pragma once

namespace Editor
{
	class UndoDataSource
	{
	public:
		enum class Type
		{
			CPUMemory,
			AuxilaryDataTableText
		};

		virtual Type GetType() const = 0;
	};
}