#pragma once

#include <string>
#include <stdio.h>
#include <memory>

namespace Utility
{
	class C64File;

	void MakeBinaryResourceIncludeFile(const std::string& inReadFileName, const std::string& inWriteFileName, const std::string& inDataName, const std::string& inNamespace);
	bool ReadFile(const std::string& inFileName, int inMaxFileSize, void** outData, long& outDataSize);
	bool WriteFile(const std::string& inFileName, const void* inData, long inDataSize);
	bool WriteFile(const std::string& inFileName, std::shared_ptr<Utility::C64File> inFile);

	void TrimStringInPlace(std::string& inString);
	std::string TrimString(const std::string& inString);
	void StringToLowerCaseInPlace(std::string& inString);
	std::string StringToLowerCase(const std::string& inString);
}



