#pragma once

#include <vector>
#include <string>
#include <assert.h>

namespace Editor
{
	namespace AuxilaryDataUtils
	{
		inline void SaveDataPushBool(std::vector<unsigned char>& ioSaveData, unsigned char inBool)
		{
			ioSaveData.push_back(inBool != 0 ? 1 : 0);
		}

		inline void SaveDataPushByte(std::vector<unsigned char>& ioSaveData, unsigned char inValue)
		{
			ioSaveData.push_back(inValue);
		}

		inline void SaveDataPushWord(std::vector<unsigned char>& ioSaveData, unsigned short inWord)
		{
			ioSaveData.push_back(static_cast<unsigned char>(inWord & 0xff));
			ioSaveData.push_back(static_cast<unsigned char>((inWord >> 8) & 0xfF));
		}

		inline void SaveDataPushUInt(std::vector<unsigned char>& ioSaveData, unsigned int inUInt)
		{
			ioSaveData.push_back(static_cast<unsigned char>((inUInt) & 0xff));
			ioSaveData.push_back(static_cast<unsigned char>((inUInt >> 8) & 0xff));
			ioSaveData.push_back(static_cast<unsigned char>((inUInt >> 16) & 0xff));
			ioSaveData.push_back(static_cast<unsigned char>((inUInt >> 24) & 0xff));
		}

		inline void SaveDataPushInt(std::vector<unsigned char>& ioSaveData, int inInt)
		{
			return SaveDataPushUInt(ioSaveData, static_cast<unsigned int>(inInt));
		}

		inline void SaveDataPushStdString256(std::vector<unsigned char>& ioSaveData, const std::string& inString)
		{
			assert(inString.size() < 256);
			
			const unsigned char string_length = static_cast<unsigned char>(inString.size());

			SaveDataPushByte(ioSaveData, string_length);

			const char* c_string = inString.c_str();

			for (int i = 0; i < string_length; ++i)
				SaveDataPushByte(ioSaveData, c_string[i]);
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
			unsigned short value = 0;
			
			value |= static_cast<unsigned short>(*it);
			it++;
			value |= static_cast<unsigned short>(*it) << 8;
			it++;

			return value;
		}

		inline unsigned int LoadDataPullUInt(std::vector<unsigned char>::iterator& it)
		{
			unsigned int value = 0;
			
			value |= static_cast<unsigned int>(*it);
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
	}
}
