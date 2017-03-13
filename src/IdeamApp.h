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
	virtual	void				ArgvReceived(int32 agrc, char** argv);
	virtual	void				MessageReceived(BMessage* message);
	virtual	bool				QuitRequested();
	virtual	void				ReadyToRun();
	virtual	void				RefsReceived(BMessage* message);

private:
			void				_CheckSettingsVersion();
private:
		IdeamWindow*			fIdeamWindow;
		TPreferences*			fUISettingsFile;
};

#endif //IDEAMAPP_H
