/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef IDEAMAPP_H
#define IDEAMAPP_H

#include <Application.h>

class IdeamApp : public BApplication {
public:
								IdeamApp();
	virtual						~IdeamApp();

	virtual	void				AboutRequested();
private:

};

#endif //IDEAMAPP_H
