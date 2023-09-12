#pragma once

#include <memory>

namespace Utility
{
	class C64FileWriter;
	class C64FileReader;
}

namespace Editor
{
	class AuxilaryDataPlayMarkers;
	class AuxilaryDataHardwarePreferences;
	class AuxilaryDataEditingPreferences;
	class AuxilaryDataTableText;
	class AuxilaryDataSongs;

	class AuxilaryDataCollection
	{
	public:
		AuxilaryDataCollection();
		~AuxilaryDataCollection();

		void operator=(const AuxilaryDataCollection& inRHS);

		AuxilaryDataPlayMarkers& GetPlayMarkers();
		const AuxilaryDataPlayMarkers& GetPlayMarkers() const;
		AuxilaryDataHardwarePreferences& GetHardwarePreferences();
		const AuxilaryDataHardwarePreferences& GetHardwarePreferences() const;
		AuxilaryDataEditingPreferences& GetEditingPreferences();
		const AuxilaryDataEditingPreferences& GetEditingPreferences() const;
		AuxilaryDataTableText& GetTableText();
		const AuxilaryDataTableText& GetTableText() const;
		AuxilaryDataSongs& GetSongs();
		const AuxilaryDataSongs& GetSongs() const;

		void Reset();

		bool Save(Utility::C64FileWriter& inWriter) const;
		bool Load(Utility::C64FileReader& inReader) const;

	private:
		std::unique_ptr<AuxilaryDataHardwarePreferences> m_HardwarePreferences;
		std::unique_ptr<AuxilaryDataPlayMarkers> m_PlayMarkers;
		std::unique_ptr<AuxilaryDataEditingPreferences> m_EditingPreferences;
		std::unique_ptr<AuxilaryDataTableText> m_TableText;
		std::unique_ptr<AuxilaryDataSongs> m_Songs;
	};
}