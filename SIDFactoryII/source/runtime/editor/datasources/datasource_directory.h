#pragma once

#include "datasource_tlist.h"
#include "libraries/ghc/fs_std.h"
#include <vector>
#include <string>

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

		DataSourceDirectory(Foundation::IPlatform* inPlatform);

		SelectResult Select(int inIndex);
		bool Back();

		bool HasFileSelection() const;
		void ClearFileSelection();
		const DirectoryEntry& GetFileSelection() const;
		const int GetFileSelectIndex() const;

		void GenerateData();
	private:

        Foundation::IPlatform* m_Platform;
		std::vector<std::string> m_Drives;

		bool m_HasFileSelection;
		int m_SelectedFileIndex;
	};
}
