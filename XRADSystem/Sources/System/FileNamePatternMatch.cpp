// file FileNamePatternMatch.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "FileNamePatternMatch.h"
#include <regex>

XRAD_BEGIN

//--------------------------------------------------------------

FileNamePatternMatch::FileNamePatternMatch(const wstring &filter)
{
	// Распространенные частные случаи обрабатываем сразу:
	if (filter.empty() || filter == L"*.*" || filter == L"*")
		return;

	auto filters_v = split(filter, L';');
	wregex filter_all_re(L"^\\s*\\*(?:\\.\\*)?\\s*$"); // " * ", " *.* "
	wregex filter_ext_re(L"^\\s*\\*(\\.[^?*/\\\\]*?)\\s*$"); // " *.ext "
	for (auto &s: filters_v)
	{
		if (regex_match(s, filter_all_re))
		{
			filters.clear();
			return;
		}
		wsmatch match;
		if (regex_match(s, match, filter_ext_re) && match.size() == 2)
		{
			wstring ext = match.str(1);
			//printf("Debug: Filter format: \"%s\".\n", convert_to_string(ext).c_str());
			filters.insert(get_upper(ext));
		}
		else
		{
			fprintf(stderr, "Warning: Invalid file filter pattern: \"%s\".\n",
					EnsureType<const char*>(convert_to_string(s).c_str()));
		}
	}
}

//--------------------------------------------------------------

bool FileNamePatternMatch::operator()(const wstring &filename) const
{
	if (filters.empty())
		return true;
	wstring filename_uc = get_upper(filename);
	for (const auto &ext: filters)
	{
		if (ext.length() == 1)
		{
			// Особый случай: "*." — файлы без расширения.
			if (filename_uc.find(L'.') == filename_uc.npos && !filename_uc.empty())
			{
				//printf("Debug: Filter match: \"%s\" vs. \"%s\".\n",
				//		convert_to_string(filename_uc).c_str(),
				//		convert_to_string(ext).c_str());
				return true;
			}
		}
		else
		{
			if (filename_uc.length() > ext.length() &&
					!wcscmp(filename_uc.c_str() + (filename_uc.length() - ext.length()),
							ext.c_str()))
			{
				//printf("Debug: Filter match: \"%s\" vs. \"%s\".\n",
				//		convert_to_string(filename_uc).c_str(),
				//		convert_to_string(ext).c_str());
				return true;
			}
			//printf("Debug: Filter mismatch: \"%s\" vs. \"%s\".\n",
			//		convert_to_string(filename_uc).c_str(),
			//		convert_to_string(ext).c_str());
		}
	}
	return false;
}

//--------------------------------------------------------------

XRAD_END
