#include "Utility/StringUtility.h"

std::string StringUtility::ToUpper(std::string string)
{
	for (size_t i = 0; i < string.length(); ++i)
	{
		string[i] = std::toupper(string[i]);
	}

	return string;
}

std::string StringUtility::ToLower(std::string string)
{
	for (size_t i = 0; i < string.length(); ++i)
	{
		string[i] = std::tolower(string[i]);
	}

	return string;
}
