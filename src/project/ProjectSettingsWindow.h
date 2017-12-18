/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef PROJECT_SETTINGS_WINDOW_H
#define PROJECT_SETTINGS_WINDOW_H

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <TextControl.h>
#include <Window.h>

#include "Project.h"
#include "TPreferences.h"


class ProjectSettingsWindow : public BWindow
{
public:
								ProjectSettingsWindow(BString name);
	virtual						~ProjectSettingsWindow();

	virtual void				MessageReceived(BMessage* message);
	virtual	bool				QuitRequested();
private:
			void				_CloseProject();
			int32				_GetProjects();
			void				_LoadProject(BString name);
			void				_SaveChanges();

			BString				fName;
			int32				fProjectsCount;
			BBox* 				fProjectBox;
			BString		 		fProjectBoxLabel;
			BString		 		fProjectBoxProjectLabel;

			BMenuField*			fProjectMenuField;
			BTextControl* 		fProjectTargetText;
			BTextControl* 		fBuildCommandText;
			BTextControl* 		fCleanCommandText;
			BTextControl* 		fRunArgsText;
			BTextControl* 		fProjectScmText;
			BString				fTargetString;
			BString				fBuildString;
			BString				fCleanString;
			BString				fRunArgsString;
			BString				fProjectScmString;

			TPreferences*		fIdmproFile;
};


#endif // PROJECT_SETTINGS_WINDOW_H
