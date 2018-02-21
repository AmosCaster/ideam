/*
 * Copyright 2018 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef IDEAM_COMMON_H
#define IDEAM_COMMON_H

#include <String.h>

#include <string>


namespace Ideam
{
	bool file_exists(const std::string& filename);
	int get_year();

	BString const Copyright();
	BString const HeaderGuard(const BString&  fileName);





}


#endif // IDEAM_COMMON_H
