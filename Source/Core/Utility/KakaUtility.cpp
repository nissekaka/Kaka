#include "stdafx.h"
#include "KakaUtility.h"
#include <sstream>
#include <iomanip>

namespace Kaka
{
	std::vector<std::string> TokenizeQuoted(const std::string& aInput)
	{
		std::istringstream stream;
		stream.str(aInput);
		std::vector<std::string> tokens;
		std::string token;

		while (stream >> std::quoted(token))
		{
			tokens.push_back(std::move(token));
		}
		return tokens;
	}

	std::wstring ToWide(const std::string& aNarrow)
	{
		wchar_t wide[512];
		mbstowcs_s(nullptr, wide, aNarrow.c_str(), _TRUNCATE);
		return wide;
	}

	std::string ToNarrow(const std::wstring& aWide)
	{
		char narrow[512];
		wcstombs_s(nullptr, narrow, aWide.c_str(), _TRUNCATE);
		return narrow;
	}
}
