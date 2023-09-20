#pragma once

#include "datasource_tlist.h"
#include "libraries/ghc/fs_std.h"
#include <vector>
#include <string>

namespace Utility
{
	class ConfigFile;
}

namespace Foundation
{
    class IPlatform;
}

namespace Editor
{
	struct DirectoryEntry
	{
		enum Type : int
		{
			Drive,
			Back,
			Folder,
			File
		};

		Type m_Type;
        fs::path m_Path;
		std::string m_DisplayName;
	};

	class DataSourceDirectory : public DataSourceTList<DirectoryEntry>
	{
	public:
		enum SelectResult : unsigned int
		{
			Succeeded = 0,
			SelectFileSucceeded,
			SelectFolderSucceeded,
			Failed = 0x8000000,
			SelectFileFailed,
			SelectFolderFailed
		};

		DataSourceDirectory(Foundation::IPlatform* inPlatform, const Utility::ConfigFile& inConfigFile);

		SelectResult Select(int inIndex);
		bool Back();

		bool HasFileSelection() const;
		void ClearFileSelection();
		const DirectoryEntry& GetFileSelection() const;
		const int GetFileSelectIndex() const;

		void GenerateData();
	private:

		struct Drive
		{
			std::string m_Path;
			std::string m_Alias;
		};

        Foundation::IPlatform* m_Platform;
		const Utility::ConfigFile& m_ConfigFile;

		std::vector<Drive> m_Drives;
		std::vector<std::string> m_ExtensionFilter;

		bool m_HasFileSelection;
		int m_SelectedFileIndex;
	};
}
