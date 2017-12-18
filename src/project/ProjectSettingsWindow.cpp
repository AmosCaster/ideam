/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "ProjectSettingsWindow.h"

#include <Alignment.h>

#include <Catalog.h>
#include <Directory.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>
#include <string>

#include "IdeamNamespace.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ProjectSettingsWindow"

enum
{
	MSG_EXIT_CLICKED				= 'excl',
	MSG_PROJECT_SELECTED			= 'prse',
};

ProjectSettingsWindow::ProjectSettingsWindow(BString name)
	:
	BWindow(BRect(0, 0, 799, 599), "ProjectSettingsWindow", B_MODAL_WINDOW,
													B_ASYNCHRONOUS_CONTROLS | 
													B_NOT_ZOOMABLE |
													B_NOT_RESIZABLE |
													B_AVOID_FRONT |
													B_AUTO_UPDATE_SIZE_LIMITS |
													B_CLOSE_ON_ESCAPE)
	, fName(name)
	, fIdmproFile(nullptr)
{
	// Button
	BButton* exitButton = new BButton("exit",
		B_TRANSLATE("Exit"), new BMessage(MSG_EXIT_CLICKED));

	// "Project" Box
	fProjectBox = new BBox("projectBox");
	fProjectBoxLabel = B_TRANSLATE("Project:");
	fProjectBox->SetLabel(fProjectBoxLabel);

	// Projects Menu
	fProjectMenuField = new BMenuField("ProjectMenuField", nullptr,
										new BMenu(B_TRANSLATE("Choose Project:")));

	fProjectTargetText = new BTextControl(B_TRANSLATE("Project target:"), "", nullptr);

	fBuildCommandText = new BTextControl(B_TRANSLATE("Build comand:"), "", nullptr);

	fCleanCommandText = new BTextControl(B_TRANSLATE("Clean comand:"), "", nullptr);

	fRunArgsText = new BTextControl(B_TRANSLATE("Run args:"), "", nullptr);

	fProjectScmText = new BTextControl(B_TRANSLATE("Source control:"), "", nullptr);

	// Layout
	BLayoutBuilder::Grid<>(fProjectBox)
	.SetInsets(20, 40, 10, 10)
	.Add(fProjectMenuField, 0, 1, 4)
	.Add(fProjectTargetText->CreateLabelLayoutItem(), 0, 2, 1)
	.Add(fProjectTargetText->CreateTextViewLayoutItem(), 1, 2, 3)
	.Add(fBuildCommandText->CreateLabelLayoutItem(), 0, 3)
	.Add(fBuildCommandText->CreateTextViewLayoutItem(), 1, 3)
	.Add(fCleanCommandText->CreateLabelLayoutItem(), 2, 3)
	.Add(fCleanCommandText->CreateTextViewLayoutItem(), 3, 3)
	.Add(fRunArgsText->CreateLabelLayoutItem(), 0, 4)
	.Add(fRunArgsText->CreateTextViewLayoutItem(), 1, 4)
	.Add(fProjectScmText->CreateLabelLayoutItem(), 0, 5)
	.Add(fProjectScmText->CreateTextViewLayoutItem(), 1, 5)
	.Add(new BSeparatorView(B_HORIZONTAL), 0, 7, 4)
	.AddGlue(0, 10)
	;

	// Window layout
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.AddGroup(B_HORIZONTAL)
			.AddGroup(B_VERTICAL, 0, 3)
				.Add(fProjectBox)
				.AddGroup(B_HORIZONTAL)
					.AddGlue()
					.Add(exitButton)
					.AddGlue()
				.End()					
			.End()
		.End()
	;

	CenterOnScreen();

	fProjectsCount = _GetProjects();

	_LoadProject(fName);
}

ProjectSettingsWindow::~ProjectSettingsWindow()
{
}

bool
ProjectSettingsWindow::QuitRequested()
{
	_CloseProject();
	Quit();

	return true;
}

void
ProjectSettingsWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case MSG_EXIT_CLICKED: {
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case MSG_PROJECT_SELECTED: {
			int32 index;
			if (msg->FindInt32("index", &index) == B_OK) {
				BMenuItem* item = fProjectMenuField->Menu()->ItemAt(index);
				// Save old project if present
				_CloseProject();
				_LoadProject(item->Label());
			}
			break;
		}
		default: {
			BWindow::MessageReceived(msg);
			break;
		}
	}
}

void
ProjectSettingsWindow::_CloseProject()
{
	if (fIdmproFile != nullptr) {
		_SaveChanges();
		delete fIdmproFile;
	}
}

int32
ProjectSettingsWindow::_GetProjects()
{
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(IdeamNames::kApplicationName);
	BDirectory projectDir(path.Path());
	BEntry entry;
	char name[B_FILE_NAME_LENGTH];
	int32 projectsCount;

	for (projectsCount = 0; projectDir.GetNextEntry(&entry) == B_OK; projectsCount++) {
		entry.GetName(name);
		BString projectName(name);

		if (projectName.EndsWith(IdeamNames::kProjectExtension)) {
			BMenuItem* item = new BMenuItem(projectName, new BMessage(MSG_PROJECT_SELECTED));
			fProjectMenuField->Menu()->AddItem(item, projectsCount);
		}
	}
	return projectsCount;
}

void
ProjectSettingsWindow::_LoadProject(BString name)
{
	if (name == "")
		return;

	// Init controls
	fProjectBoxProjectLabel.SetTo("");
	fProjectTargetText->SetText("");
	fBuildCommandText->SetText("");
	fCleanCommandText->SetText("");
	fRunArgsText->SetText("");
	fProjectScmText->SetText("");

	fProjectBoxProjectLabel << fProjectBoxLabel << "\t\t" << name;
	fProjectBox->SetLabel(fProjectBoxProjectLabel);

	fIdmproFile = new TPreferences(name, IdeamNames::kApplicationName, 'PRSE');

	if (fIdmproFile->FindString("project_target", &fTargetString) == B_OK)
		fProjectTargetText->SetText(fTargetString);

	if (fIdmproFile->FindString("project_build_command", &fBuildString) == B_OK)
		fBuildCommandText->SetText(fBuildString);

	if (fIdmproFile->FindString("project_clean_command", &fCleanString) == B_OK)
		fCleanCommandText->SetText(fCleanString);

	if (fIdmproFile->FindString("project_run_args", &fRunArgsString) == B_OK)
		fRunArgsText->SetText(fRunArgsString);

	if (fIdmproFile->FindString("project_scm", &fProjectScmString) == B_OK)
		fProjectScmText->SetText(fProjectScmString);
}

void
ProjectSettingsWindow::_SaveChanges()
{
	BString target(fProjectTargetText->Text());
	if (target != fTargetString)
		fIdmproFile->SetBString("project_target", target);

	BString build(fBuildCommandText->Text());
	if (build != fBuildString)
		fIdmproFile->SetBString("project_build_command", build);

	BString clean(fCleanCommandText->Text());
	if (clean != fCleanString)
		fIdmproFile->SetBString("project_clean_command", clean);

	BString runargs(fRunArgsText->Text());
	if (runargs != fRunArgsString)
		fIdmproFile->SetBString("project_run_args", runargs);

	BString scm(fProjectScmText->Text());
	if (scm != fProjectScmString)
		fIdmproFile->SetBString("project_scm", scm);
}
