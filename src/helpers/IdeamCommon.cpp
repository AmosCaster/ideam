/*
 * Copyright 2018 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "IdeamCommon.h"

#include <cctype>
#include <ctime>
#include <fstream>
#include <string>

namespace Ideam
{

BString const Copyright()
{
	BString header;
	header
		<< "/*\n"
		<< " * Copyright " << get_year() << " Your_Name <your@email.address>\n"
		<< " * All rights reserved. Distributed under the terms of the MIT license.\n"
		<< " */\n";

	return header;
}

/*
 * Returns the string to be used in header guard
 * In: ALLUPPER  = Do not modify
 *     all_lower = ALL_LOWER
 *     CamelCase = CAMEL_CASE
 */
BString const HeaderGuard(const BString& fileName)
{
	BString upper(fileName);
	BString lower(fileName);

	// Filename is UPPER case, leave it
	if (fileName == upper.ToUpper())
		return fileName;
	// Filename is lower case, make it UPPER
	else if (fileName == lower.ToLower())
		return lower.ToUpper();

	std::string in(fileName);
	std::string out("");

	for (auto iter = in.begin(); iter != in.end(); iter++) {
		if (iter != in.begin() && std::isupper(static_cast<unsigned char>(*iter)))
			out += static_cast<unsigned char>('_');
		out += std::toupper(static_cast<unsigned char>(*iter));
	}
	return out.c_str();
}

bool file_exists(const std::string& filename)
{
	// Test existence
	std::ifstream file(filename);
	if (file.is_open()) {
		file.close();
		return true;
	}
	return false;
}

int get_year()
{
	// Get year
	time_t time = ::time(NULL);
	struct tm* tm = localtime(&time);
	return tm->tm_year + 1900;
}







} // end of namespace
