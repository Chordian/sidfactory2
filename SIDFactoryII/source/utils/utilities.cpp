#include "utilities.h"
#include "c64file.h"
#include <cctype>
#include <algorithm>

namespace Utility
{
	void MakeBinaryResourceIncludeFile(const std::string& inReadFileName, const std::string& inWriteFileName, const std::string& inDataName, const std::string& inNamespace)
	{
		// Open the file
		FILE* file_read = fopen(inReadFileName.c_str(), "rb");

		if (file_read == nullptr)
			return;

		// Get the file size
		fseek(file_read, 0, SEEK_END);
		long read_file_size = ftell(file_read);
		fseek(file_read, 0, SEEK_SET);

		// Allocate a buffer for the file data
		char* buffer = new char[read_file_size];

		// Read the data from the file
		size_t bytes_read = fread(buffer, 1, read_file_size, file_read);

		// Close the file
		fclose(file_read);

		if (bytes_read == static_cast<size_t>(read_file_size))
		{
			// Open the output file
			FILE* write_file = fopen(inWriteFileName.c_str(), "w");
			fprintf(write_file, "#pragma once\n\n");
			fprintf(write_file, "namespace %s\n{\n", inNamespace.c_str());
			fprintf(write_file, "\tconst unsigned char %s[0x%08x] =\n\t{", inDataName.c_str(), static_cast<unsigned int>(read_file_size));

			for (int data_counter = 0; data_counter < read_file_size; ++data_counter)
			{
				unsigned char byte = buffer[data_counter];

				if (data_counter > 0)
					fprintf(write_file, ", ");
				if ((data_counter & 0x0f) == 0)
					fprintf(write_file, "\n\t\t");

				fprintf(write_file, "0x%02x", byte);
			}

			fprintf(write_file, "\n\t};\n}");

			fclose(write_file);
			delete[] buffer;
		}
	}


	bool ReadFile(const std::string& inFileName, int inMaxFileSize, void** outData, long& outDataSize)
	{
		// Open the file
		FILE* file_read = fopen(inFileName.c_str(), "rb");

		if (file_read == nullptr)
		{
			outDataSize = 0;
			return false;
		}

		// Get the file size
		fseek(file_read, 0, SEEK_END);
		long read_file_size = ftell(file_read);
		fseek(file_read, 0, SEEK_SET);

		if (inMaxFileSize > 0 && read_file_size > static_cast<long>(inMaxFileSize))
			return false;

		// Allocate a buffer for the file data
		char* buffer = new char[read_file_size];

		// Read the data from the file
		size_t bytes_read = fread(buffer, 1, read_file_size, file_read);

		// Close the file
		fclose(file_read);

		if (bytes_read == static_cast<size_t>(read_file_size))
		{
			outDataSize = read_file_size;
			*outData = buffer;

			return true;
		}

		delete[] buffer;
		return false;
	}


	bool WriteFile(const std::string& inFileName, const void* inData, long inDataSize)
	{
		FILE* file_write = fopen(inFileName.c_str(), "wb");

		if (file_write == nullptr)
            return false;

		fwrite(inData, 1, inDataSize, file_write);
		fclose(file_write);

		return true;
	}


	bool WriteFile(const std::string& inFileName, std::shared_ptr<Utility::C64File> inFile)
	{
		unsigned char* data = inFile->GetDataCopyAsPRG();
		const unsigned int data_size = inFile->GetPRGDataSize();

		const bool return_value = WriteFile(inFileName, static_cast<const void*>(data), data_size);

		delete[] data;

		return return_value;
	}


	void TrimStringInPlace(std::string& inString)
	{
		inString.erase(inString.begin(), std::find_if(inString.begin(), inString.end(), [](int character)
			{
				return !std::isspace(character);
			}));
		inString.erase(std::find_if(inString.rbegin(), inString.rend(), [](int character)
			{
				return !std::isspace(character);
			}).base(), inString.end());
	}

	std::string TrimString(const std::string& inString)
	{
		std::string result = inString;

		TrimStringInPlace(result);
		return result;
	}

	void StringToLowerCaseInPlace(std::string& ioString)
	{
		std::transform(ioString.begin(), ioString.end(), ioString.begin(),
			[](unsigned char c) { return std::tolower(c); });
	}

	std::string StringToLowerCase(const std::string& inString)
	{
		std::string result = inString;
		StringToLowerCaseInPlace(result);

		return result;
	}
}



