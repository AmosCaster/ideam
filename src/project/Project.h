/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef PROJECT_H
#define PROJECT_H

#include <String.h>
#include <vector>

#include "ProjectTitleItem.h"
#include "TPreferences.h"

class Project {
public:
								Project(BString const& name);
								~Project();

			void				Activate();
			BString				BasePath() const { return fProjectDirectory; }
			BString				BuildCommand() const { return fBuildCommand; }
			BString				CleanCommand() const { return fCleanCommand; }
			void				Close();
			void				Deactivate();
	std::vector<BString>		FilesList() const { return fFilesList; };
			bool				IsActive() { return isActive; }
			BString	const		Name() const { return fName; }
			status_t			Open(bool activate);
			bool				RunInTerminal() { return fRunInTerminal; }
	std::vector<BString>		SourcesList() const { return fSourcesList; };
			BString 			Target() const { return fTarget; }
			ProjectTitleItem*	Title() const { return fProjectTitle; }
			BString				Type() const { return fType; }

private:

private:
			BString	const		fName;
			BString				fBuildCommand;
			BString				fCleanCommand;
			BString				fProjectDirectory;
			BString				fTarget;
			BString				fScm;
			BString				fType;
			bool				fRunInTerminal;
		std::vector<BString>	fFilesList;
		std::vector<BString>	fSourcesList;
			bool				isActive;
			ProjectTitleItem*	fProjectTitle;
			TPreferences*		fIdmproFile;
};


#endif // PROJECT_H

