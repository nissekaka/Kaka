#pragma once
#include <vector>
#include <string>


namespace Kaka
{
	std::vector<std::string> TokenizeQuoted(const std::string& aInput);
	std::wstring ToWide(const std::string& aNarrow);
	std::string ToNarrow(const std::wstring& aWide);
}
