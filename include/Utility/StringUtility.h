#pragma once

#include <algorithm>
#include <filesystem>

class StringUtility
{
public:
	template <typename T>
	static std::string ConvertValueToHexString(const T value)
	{
		std::stringstream stringstream;

		stringstream << "0x" << std::hex << std::uppercase << value;

		return stringstream.str();
	}

	static std::string ToUpper(std::string string);
	static std::string ToLower(std::string string);
};
