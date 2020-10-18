#include "datasource_directory.h"
#include "foundation/platform/iplatform.h"
#include "utils/configfile.h"
#include "utils/config/configtypes.h"
#include <cctype>

using namespace fs;
using namespace Utility;
using namespace Utility::Config;

namespace Editor
{
	DataSourceDirectory::DataSourceDirectory(Foundation::IPlatform* inPlatform, const ConfigFile& inConfigFile)
		: DataSourceTList<DirectoryEntry>()
		, m_Platform(inPlatform)
		, m_ConfigFile(inConfigFile)
		, m_HasFileSelection(false)
		, m_SelectedFileIndex(0)
	{
		const unsigned int drives_count = inPlatform->Storage_GetLogicalDrivesCount();
		for (unsigned int i = 0; i < drives_count; ++i)
			m_Drives.push_back({ inPlatform->Storage_GetLogicalDriveName(i), "" });

		auto user_folders = GetConfigurationValues<ConfigValueString>(inConfigFile, "Disk.UserFolders", {});
		auto user_folders_alias = GetConfigurationValues<ConfigValueString>(inConfigFile, "Disk.UserFolders.Aliases", {});

		const size_t user_folder_count = user_folders.size();
		const bool has_aliases = user_folders_alias.size() == user_folder_count;

		for (size_t i = 0; i<user_folder_count; ++i)
		{
			const std::string& user_folder = inPlatform->OS_ParsePath(user_folders[i]);
			path user_folder_path = path(user_folder);

			if (is_directory(user_folder_path))
				m_Drives.push_back({ user_folder, has_aliases ? user_folders_alias[i] : "" });
		}
        
		GenerateData();
	}

	//----------------------------------------------------------------------------------------------------------------

	DataSourceDirectory::SelectResult DataSourceDirectory::Select(int inIndex)
	{
		assert(inIndex >= 0);
		assert(inIndex < static_cast<int>(m_List.size()));

		const DirectoryEntry& entry = (*this)[inIndex];
        std::error_code error_code;

		switch (entry.m_Type)
		{
		case DirectoryEntry::Back:
			Back();
			return SelectResult::SelectFolderSucceeded;

		case DirectoryEntry::Drive:
		case DirectoryEntry::Folder:
            if(m_Platform->Storage_SetCurrentPath(entry.m_Path.string()))
            {
                GenerateData();
                return SelectResult::SelectFolderSucceeded;
            }

			return SelectResult::SelectFolderFailed;
		case DirectoryEntry::File:
			m_SelectedFileIndex = inIndex;
			m_HasFileSelection = true;

			return SelectResult::SelectFileSucceeded;
		default:
			assert(false);
			break;
		}

		return SelectResult::Failed;
	}

	bool DataSourceDirectory::Back()
	{
        std::error_code error_code;
        
		std::string current_path_string = current_path().string();
		if (m_Platform->Storage_SetCurrentPath(current_path().parent_path().string()))
 		{
			GenerateData();

			for (size_t i = 0; i < m_List.size(); ++i)
			{
				if (m_List[i].m_Type == DirectoryEntry::Folder && m_List[i].m_Path == current_path_string)
				{
					m_HasFileSelection = true;
					m_SelectedFileIndex = static_cast<int>(i);
					return true;
				}
			}

			m_SelectedFileIndex = -1;
			return true;
		}

		return false;
	}

	//----------------------------------------------------------------------------------------------------------------

	bool DataSourceDirectory::HasFileSelection() const
	{
		return m_HasFileSelection;
	}

	void DataSourceDirectory::ClearFileSelection()
	{
		m_HasFileSelection = false;
		m_SelectedFileIndex = -1;
	}

	const DirectoryEntry& DataSourceDirectory::GetFileSelection() const
	{
		assert(m_HasFileSelection);
		assert(m_SelectedFileIndex >= 0 && m_SelectedFileIndex < static_cast<int>(m_List.size()));

		return m_List[m_SelectedFileIndex];
	}

	const int DataSourceDirectory::GetFileSelectIndex() const
	{
		return m_SelectedFileIndex;
	}


	//----------------------------------------------------------------------------------------------------------------

	void DataSourceDirectory::GenerateData()
	{
		// Clear the list (data)
		m_List.clear();

		// Add drives to the list
		for (const Drive& drive_name : m_Drives)
			m_List.push_back({ DirectoryEntry::Drive, path(drive_name.m_Path), drive_name.m_Alias });

		// Add back
		fs::path current_path = fs::current_path();

		const bool is_root = current_path.parent_path() == current_path;
		if(!is_root)
			m_List.push_back({ DirectoryEntry::Back, ".." });

		// Iterate entries in current directory and add folder to the list and cache files for adding afterward
		directory_iterator directory_iterator(current_path);
		std::vector<path> files;
		for (auto& path : directory_iterator)
		{
            if(!m_Platform->Storage_IsSystemFile(path.path().string()))
            {
                std::error_code error_code;
                if (is_directory(path, error_code))
                    m_List.push_back({ DirectoryEntry::Folder, path });
				else if (is_regular_file(path, error_code))
					files.push_back(path);
			}
		}

		for (auto& file : files)
			m_List.push_back({ DirectoryEntry::File, file });

		// Sort the list
		std::sort(m_List.begin(), m_List.end(), [](const DirectoryEntry& inEntry1, const DirectoryEntry& inEntry2)
			{
				// If the types are the same, lets check the filenames against eachother (and ignore case .. which means a transformation per comparasin, not fast.. but who cares! This is disk operation stuff)
				if (inEntry1.m_Type == inEntry2.m_Type)
				{
					std::string name1 = inEntry1.m_Path.string();
					std::string name2 = inEntry2.m_Path.string();

					std::transform(name1.begin(), name1.end(), name1.begin(),
						[](char c) { return std::tolower(c); });
					std::transform(name2.begin(), name2.end(), name2.begin(),
						[](char c) { return std::tolower(c); });

					return name1 < name2;
				}

				// Otherwise just prefer one type over the other
				return inEntry1.m_Type < inEntry2.m_Type;
			});
	}
}
