/*
 * Copyright 2017  A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "Project.h"

#include <stdexcept>

#include "IdeamNamespace.h"

Project::Project(BString const& name)
	:
	fName(name)
{
}

Project::~Project()
{
	delete fProjectTitle;
}

void
Project::Activate()
{
	isActive = true;
	fProjectTitle->Activate();
}

void
Project::Close()
{
}

void
Project::Deactivate()
{
	isActive = false;
	fProjectTitle->Deactivate();
}

status_t
Project::Open(bool activate)
{
	if (fName.IsEmpty())
		throw std::logic_error("Empty name");

	fIdmproFile = new TPreferences(fName, IdeamNames::kApplicationName, 'PRSE');
/*
	BString filename;
	if (fIdmproFile->FindString("project_filename", &filename) != B_OK) {
		delete fIdmproFile;
		return B_ERROR;
	}
*/
	isActive = activate;

	fProjectTitle = new ProjectTitleItem(fName.String(), activate);

	// Update project data
	if (fIdmproFile->FindString("project_build_command", &fBuildCommand) != B_OK)
		fBuildCommand = "";

	if (fIdmproFile->FindString("project_clean_command", &fCleanCommand) != B_OK)
		fCleanCommand = "";

	if (fIdmproFile->FindString("project_directory", &fProjectDirectory) != B_OK)
		fProjectDirectory = "";

	if (fIdmproFile->FindString("project_target", &fTarget) != B_OK)
		fTarget = "";

	if (fIdmproFile->FindString("project_scm", &fScm) != B_OK)
		fScm = "";

	// Source files
	int32 refsCount = 0;
	BString sources;

	while (fIdmproFile->FindString("project_source", refsCount, &sources) == B_OK) {

		fSourcesList.push_back(sources);
		refsCount++;
	}
	// Other files
	BString files;
	refsCount = 0;

	while (fIdmproFile->FindString("project_file", refsCount, &files) == B_OK) {

		fFilesList.push_back(files);
		refsCount++;
	}

	delete fIdmproFile;	

	return B_OK;
}
