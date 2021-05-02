#pragma once

#include <libraries/json.hpp>
#include "utils/logging.h"
#include "runtime/editor/datasources/datasource_table.h"

// for convenience
using json = nlohmann::json;

namespace Editor
{
	namespace EditorUtils
	{
    void SaveAsJson(std::shared_ptr<DataSourceTable> table);
  }
}