#pragma once

#include <vector>
#include <string>
#include "foundation/base/assert.h"

namespace Editor
{
	namespace AuxilaryDataUtils
	{
		inline void SaveDataPushBool(std::vector<unsigned char>& ioSaveData, const bool inBool)
		{
			ioSaveData.push_back(inBool != 0 ? 1 : 0);
		}

		inline void SaveDataPushByte(std::vector<unsigned char>& ioSaveData, const unsigned char inValue)
		{
			ioSaveData.push_back(inValue);
		}

		inline void SaveDataPushWord(std::vector<unsigned char>& ioSaveData, const unsigned short inWord)
		{
			ioSaveData.push_back(static_cast<unsigned char>(inWord & 0xff));
			ioSaveData.push_back(static_cast<unsigned char>((inWord >> 8) & 0xfF));
		}

		inline void SaveDataPushUInt(std::vector<unsigned char>& ioSaveData, const unsigned int inUInt)
		{
			ioSaveData.push_back(static_cast<unsigned char>((inUInt) & 0xff));
			ioSaveData.push_back(static_cast<unsigned char>((inUInt >> 8) & 0xff));
			ioSaveData.push_back(static_cast<unsigned char>((inUInt >> 16) & 0xff));
			ioSaveData.push_back(static_cast<unsigned char>((inUInt >> 24) & 0xff));
		}

		inline void SaveDataPushInt(std::vector<unsigned char>& ioSaveData, const int inInt)
		{
			return SaveDataPushUInt(ioSaveData, static_cast<unsigned int>(inInt));
		}

		inline void SaveDataPushStdString256(std::vector<unsigned char>& ioSaveData, const std::string& inString)
		{
			FOUNDATION_ASSERT(inString.size() < 256);
			
			const unsigned char string_length = static_cast<unsigned char>(inString.size());

			SaveDataPushByte(ioSaveData, string_length);

			const char* c_string = inString.c_str();

			for (int i = 0; i < string_length; ++i)
				SaveDataPushByte(ioSaveData, c_string[i]);
		}

		inline void SaveDataPushByteArray(std::vector<unsigned char>& ioSaveData, const std::vector<unsigned char>& inByteArray)
		{
			FOUNDATION_ASSERT(inByteArray.size() < 256);

			const unsigned char data_size = static_cast<unsigned char>(inByteArray.size());

			SaveDataPushByte(ioSaveData, data_size);

			for (int i = 0; i < data_size; ++i)
				SaveDataPushByte(ioSaveData, inByteArray[i]);
		}

		inline bool LoadDataPullBool(std::vector<unsigned char>::iterator& it)
		{
			unsigned char value = *it;
			it++;

			return value != 0;
		}

		inline unsigned char LoadDataPullByte(std::vector<unsigned char>::iterator& it)
		{
			unsigned char value = *it;
			it++;

			return value;
		}

		inline unsigned short LoadDataPullWord(std::vector<unsigned char>::iterator& it)
		{
			unsigned short value = static_cast<unsigned short>(*it);
			it++;
			value |= static_cast<unsigned short>(*it) << 8;
			it++;

			return value;
		}

		inline unsigned int LoadDataPullUInt(std::vector<unsigned char>::iterator& it)
		{
			unsigned int value = static_cast<unsigned int>(*it);
			it++;
			value |= static_cast<unsigned int>(*it) << 8;
			it++;
			value |= static_cast<unsigned int>(*it) << 16;
			it++;
			value |= static_cast<unsigned int>(*it) << 24;
			it++;

			return value;
		}

		inline std::string LoadDataPullStdString256(std::vector<unsigned char>::iterator& it)
		{
			const unsigned char string_length = LoadDataPullByte(it);

			std::string output;

			for(int i=0; i<string_length; ++i)
				output += static_cast<char>(LoadDataPullByte(it));

			return output;
		}

		inline std::vector<unsigned char> LoadDataPullByteArray(std::vector<unsigned char>::iterator& it)
		{
			const unsigned char data_size = LoadDataPullByte(it);

			std::vector<unsigned char> output;

			output.reserve(data_size);

			for (int i = 0; i < data_size; ++i)
				output.push_back(LoadDataPullByte(it));

			return output;
		}
	}
}
