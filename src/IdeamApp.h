/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef IDEAMAPP_H
#define IDEAMAPP_H

#include <Application.h>

#include "IdeamWindow.h"
#include "TPreferences.h"

class IdeamApp : public BApplication {
public:
								IdeamApp();
	virtual						~IdeamApp();

	virtual	void				AboutRequested();
	virtual	bool				QuitRequested();
private:
		IdeamWindow*			fIdeamWindow;
		TPreferences*			fUISettingsFile;
};

#endif //IDEAMAPP_H
