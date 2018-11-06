/*
 * Copyright 2018 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef IDEAM_COMMON_H
#define IDEAM_COMMON_H

#include <algorithm>
#include <string>


namespace Ideam
{
	std::string const	Copyright();
	std::string const	HeaderGuard(const std::string&  fileName);

	bool				file_exists(const std::string& filename);
	std::string const	file_type(const std::string& filename);
	int					get_year();


	template<class Element, class Container>
	bool _in_container(const Element & element, const Container & container)
	{
		return std::find(std::begin(container), std::end(container), element)
				!= std::end(container);
	}



}


#endif // IDEAM_COMMON_H
