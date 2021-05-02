#include "json_export.h"
#include <iostream>
#include "runtime/editor/utilities/editor_utils.h"

using namespace Utility;

namespace Editor
{
	namespace EditorUtils
	{
		void SaveAsJson(std::shared_ptr<DataSourceTable> table)
		{

			const int rows = (*table).GetRowCount();
			const int columns = (*table).GetColumnCount();

			json j;
			j["instruments"] = json::array();

			for (int i = 0; i < rows; i++)
			{
				json instrument;

				instrument["id"] = i;
				instrument["data"] = json::array();
				for (int j = 0; j < columns; j++)
				{
          std::string value = EditorUtils::ConvertToHexValue((*table)[i * columns + j], false);
					instrument["data"].push_back(value);
				}
				j["instruments"].push_back(instrument);
			}

			std::cout << j.dump() << "\n";
		}
	}
}