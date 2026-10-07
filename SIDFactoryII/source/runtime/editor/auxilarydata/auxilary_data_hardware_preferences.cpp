#include "auxilary_data_hardware_preferences.h"
#include "auxilary_data_utils.h"
#include "foundation/base/assert.h"
#include "utils/c64file.h"
#include "utils/config/configtypes.h"
#include "utils/configfile.h"
#include "utils/global.h"

using namespace Utility;
using namespace Utility::Config;

namespace Editor
{
	AuxilaryDataHardwarePreferences::AuxilaryDataHardwarePreferences()
		: AuxilaryData(Type::HardwarePreferences)
	{
		Reset();
	}


	void AuxilaryDataHardwarePreferences::Reset()
	{

		ConfigFile& config = Global::instance().GetConfig();
		int default_sid_model = GetSingleConfigurationValue<ConfigValueInt>(config, "Sound.Emulation.Default.Model", 8580);
		std::string default_region = GetSingleConfigurationValue<ConfigValueString>(config, "Sound.Emulation.Default.Region", std::string("PAL"));

		if (default_sid_model == 6581)
		{
			m_SIDModel = SIDModel::MOS6581;
		}
		else
		{
			m_SIDModel = SIDModel::MOS8580;
		}

		if (default_region == "NTSC")
		{
			m_Region = Region::NTSC;
		}
		else
		{
			m_Region = Region::PAL;
		}

		ResetPanning();
	}


	void AuxilaryDataHardwarePreferences::ResetPanning()
	{
		// All bits cleared in the SID v5 header: L-R alternating for multi SID, a single SID centered
		m_PanLayout = SIDPanLayout::Standard;
		m_PanMode = SIDPanMode::Direct;
		m_SingleSIDPan = SIDPan::Center;
	}


	const AuxilaryDataHardwarePreferences::SIDModel AuxilaryDataHardwarePreferences::GetSIDModel() const
	{
		return m_SIDModel;
	}


	void AuxilaryDataHardwarePreferences::SetSIDModel(const SIDModel inSIDModel)
	{
		m_SIDModel = inSIDModel;
	}


	const AuxilaryDataHardwarePreferences::Region AuxilaryDataHardwarePreferences::GetRegion() const
	{
		return m_Region;
	}


	void AuxilaryDataHardwarePreferences::SetRegion(const Region inRegion)
	{
		m_Region = inRegion;
	}


	const SIDPanLayout AuxilaryDataHardwarePreferences::GetPanLayout() const
	{
		return m_PanLayout;
	}


	void AuxilaryDataHardwarePreferences::SetPanLayout(const SIDPanLayout inPanLayout)
	{
		m_PanLayout = inPanLayout;
	}


	const SIDPanMode AuxilaryDataHardwarePreferences::GetPanMode() const
	{
		return m_PanMode;
	}


	void AuxilaryDataHardwarePreferences::SetPanMode(const SIDPanMode inPanMode)
	{
		m_PanMode = inPanMode;
	}


	const SIDPan AuxilaryDataHardwarePreferences::GetSingleSIDPan() const
	{
		return m_SingleSIDPan;
	}


	void AuxilaryDataHardwarePreferences::SetSingleSIDPan(const SIDPan inPan)
	{
		m_SingleSIDPan = inPan;
	}


	void AuxilaryDataHardwarePreferences::GetPanning(unsigned int inSIDCount, SIDPan* outPan) const
	{
		if (inSIDCount == 1)
			outPan[0] = m_SingleSIDPan;
		else
			ComputeSIDPanning(m_PanLayout, m_PanMode, inSIDCount, outPan);
	}


	std::vector<unsigned char> AuxilaryDataHardwarePreferences::GenerateSaveData() const
	{
		std::vector<unsigned char> output;

		AuxilaryDataUtils::SaveDataPushByte(output, m_SIDModel);
		AuxilaryDataUtils::SaveDataPushByte(output, m_Region);

		// Version 2: older readers take the first two bytes and skip the rest of the block
		AuxilaryDataUtils::SaveDataPushByte(output, static_cast<unsigned char>(m_PanLayout));
		AuxilaryDataUtils::SaveDataPushByte(output, static_cast<unsigned char>(m_PanMode));
		AuxilaryDataUtils::SaveDataPushByte(output, static_cast<unsigned char>(m_SingleSIDPan));

		return output;
	}


	unsigned short AuxilaryDataHardwarePreferences::GetGeneratedFileVersion() const
	{
		return 2;
	}


	bool AuxilaryDataHardwarePreferences::RestoreFromSaveData(unsigned short inDataVersion, std::vector<unsigned char> inData)
	{
		auto it = inData.begin();

		m_SIDModel = static_cast<SIDModel>(AuxilaryDataUtils::LoadDataPullByte(it));
		m_Region = static_cast<Region>(AuxilaryDataUtils::LoadDataPullByte(it));

		ResetPanning();

		if (inDataVersion >= 2 && inData.size() >= 5)
		{
			m_PanLayout = static_cast<SIDPanLayout>(AuxilaryDataUtils::LoadDataPullByte(it) & 0x03);
			m_PanMode = static_cast<SIDPanMode>(AuxilaryDataUtils::LoadDataPullByte(it) & 0x03);

			const unsigned char single_sid_pan = AuxilaryDataUtils::LoadDataPullByte(it);
			m_SingleSIDPan = single_sid_pan <= static_cast<unsigned char>(SIDPan::Right) ? static_cast<SIDPan>(single_sid_pan) : SIDPan::Center;
		}

		return true;
	}
}
