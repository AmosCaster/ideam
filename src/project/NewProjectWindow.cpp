/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "NewProjectWindow.h"

#include <Catalog.h>
#include <Button.h>
#include <ListItem.h>
#include <OutlineListView.h>
#include <ScrollView.h>
#include <Architecture.h>

#include <Alignment.h>
#include <AppFileInfo.h>
#include <Application.h>
#include <Bitmap.h>
#include <Box.h>
#include <Button.h>
#include <Catalog.h>
#include <ControlLook.h>
#include <DateTime.h>
#include <Directory.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <Locale.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Mime.h>
#include <NodeInfo.h>
#include <OutlineListView.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <SeparatorView.h>
#include <SpaceLayoutItem.h>
#include <SplitView.h>
#include <TextControl.h>
#include <OptionPopUp.h>

#include "IdeamNamespace.h"
#include "TPreferences.h"

#include <iostream>
#include <fstream>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NewProject Window"

ProjectTypeMap projectTypeMap;

enum
{
	MSG_PROJECT_CANCEL				= 'prca',
	MSG_PROJECT_CREATE				= 'prcr',
	MSG_PROJECT_CHOSEN				= 'prch',
	MSG_PROJECT_NAME_EDITED			= 'pned',
	MSG_PROJECT_TARGET				= 'prta',
	MSG_PROJECT_DIRECTORY			= 'prdi',
	MSG_RUN_IN_TERMINAL				= 'rite',
	MSG_ADD_FILE_NAME				= 'afna',
	MSG_ADD_HEADER_TOGGLED			= 'ahto',
	MSG_ADD_SECOND_FILE_NAME		= 'asfn',
	MSG_ADD_SECOND_HEADER_TOGGLED	= 'asht',
	MSG_PROJECT_TYPE_CHANGED		= 'ptch',
	MSG_HAIKU_APP_EDITED			= 'haed',
	MSG_BROWSE_HAIKU_APP_CLICKED	= 'bhac',
	MSG_HAIKU_APP_REFS_RECEIVED		= 'harr',
	MSG_LOCAL_APP_EDITED			= 'laed',
	MSG_BROWSE_LOCAL_APP_CLICKED	= 'blac',
	MSG_LOCAL_APP_REFS_RECEIVED		= 'larr',
};


NewProjectWindow::NewProjectWindow()
	:
	BWindow(BRect(0, 0, 799, 599), "New Project", B_TITLED_WINDOW, //B_MODAL_WINDOW,
													B_ASYNCHRONOUS_CONTROLS | 
													B_NOT_ZOOMABLE |
													B_NOT_RESIZABLE |
													B_AVOID_FRONT |
													B_AUTO_UPDATE_SIZE_LIMITS |
													B_CLOSE_ON_ESCAPE)
{
	// Project types OutlineListView
	fTypeListView = new BOutlineListView("typeview", B_SINGLE_SELECTION_LIST);
	fTypeListView->SetSelectionMessage(new BMessage(MSG_PROJECT_CHOSEN));

	typeListScrollView = new BScrollView("typescrollview",
		fTypeListView, B_FRAME_EVENTS | B_WILL_DRAW, false, true, B_FANCY_BORDER);
	
	// Items
	haikuItem = new TitleItem("Haiku");
	haikuItem->SetEnabled(false);
	haikuItem->SetExpanded(false);
	fTypeListView->AddItem(haikuItem);

	appItem = new BStringItem(B_TRANSLATE("Application"), 1, true);
	appMenuItem = new BStringItem(B_TRANSLATE("Application with menu"), 1, true);
/*
	appLayoutItem = new BStringItem(B_TRANSLATE("Application with layout"), 1, true);
	sharedItem = new BStringItem(B_TRANSLATE("Shared Library"), 1, true);
	staticItem = new BStringItem(B_TRANSLATE("Static Library"), 1, true);
	driverItem = new BStringItem(B_TRANSLATE("Driver"), 1, true);
	trackerItem = new BStringItem(B_TRANSLATE("Tracker add-on"), 1, true);
*/
	fTypeListView->AddItem(appItem);
	fTypeListView->AddItem(appMenuItem);
/*
	fTypeListView->AddItem(appLayoutItem);
	fTypeListView->AddItem(sharedItem);
	fTypeListView->AddItem(staticItem);
	fTypeListView->AddItem(driverItem);
	fTypeListView->AddItem(trackerItem);
*/
	genericItem = new TitleItem("Generic");
	genericItem->SetEnabled(false);
	genericItem->SetExpanded(false);
	fTypeListView->AddItem(genericItem);
	helloCplusItem = new BStringItem(B_TRANSLATE("C++ Hello World!"), 1, true);
	helloCItem = new BStringItem(B_TRANSLATE("C Hello World!"), 1, true);
	principlesItem = new BStringItem(B_TRANSLATE("Principles and Practice (2nd)"), 1, true);
	emptyItem = new BStringItem(B_TRANSLATE("Empty Project"), 1, true);
	fTypeListView->AddItem(helloCplusItem);
	fTypeListView->AddItem(helloCItem);
	fTypeListView->AddItem(principlesItem);
	fTypeListView->AddItem(emptyItem);

	importItem = new TitleItem(B_TRANSLATE("Import"));
	importItem->SetEnabled(false);
	importItem->SetExpanded(false);
	fTypeListView->AddItem(importItem);
	sourcesItem = new BStringItem(B_TRANSLATE("App from Haiku sources"), 1, true);
	existingItem = new BStringItem(B_TRANSLATE("C/C++ Project with Makefile"), 1, true);
	fTypeListView->AddItem(sourcesItem);
	fTypeListView->AddItem(existingItem);

	rustItem = new TitleItem(B_TRANSLATE("Rust"));
	rustItem->SetEnabled(false);
	rustItem->SetExpanded(false);
	cargoItem = new BStringItem(B_TRANSLATE("Cargo project"), 1, true);
	fTypeListView->AddItem(rustItem);
	fTypeListView->AddItem(cargoItem);

	// Map Items description
	_MapItems();

	// Project Description TextView
	fProjectDescription = new BTextView("projecttext");
	fProjectDescription->SetInsets(4.0f, 4.0f, 4.0f, 4.0f);

	fScrollText = new BScrollView("scrolltext",
		fProjectDescription, B_WILL_DRAW | B_FRAME_EVENTS, false,
		true, B_FANCY_BORDER);

	// Buttons
	fCancelButton = new BButton("cancel",
		B_TRANSLATE("Cancel"), new BMessage(MSG_PROJECT_CANCEL));

	fCreateButton = new BButton("create", B_TRANSLATE("Create"),
		new BMessage(MSG_PROJECT_CREATE) );
	fCreateButton->SetEnabled(false);

	// "Project" Box
	fProjectBox = new BBox("projectBox");
	BString boxLabel = B_TRANSLATE("Project (host architecture: ");
	boxLabel << get_primary_architecture() << ")";
	fProjectBox->SetLabel(B_TRANSLATE(boxLabel));

	fProjectNameText = new BTextControl("nameTC", B_TRANSLATE("Project name:"), "",
			nullptr); //new BMessage(MSG_PROJECT_NAME));
	fProjectNameText->SetModificationMessage(new BMessage(MSG_PROJECT_NAME_EDITED));
	fProjectNameText->SetEnabled(false);

	fProjectTargetTC = new BTextControl("targetTC", B_TRANSLATE("Project target:"), "",
			new BMessage(MSG_PROJECT_TARGET));	
	fProjectTargetTC->SetEnabled(false);

	fRunInTeminal = new BCheckBox("RunInTeminal",
		B_TRANSLATE("Run in\nTerminal"), new BMessage(MSG_RUN_IN_TERMINAL));
	fRunInTeminal->SetEnabled(false);
	fRunInTeminal->SetValue(B_CONTROL_OFF);

	fProjectsDirectoryText = new BTextControl("directoryTC", B_TRANSLATE("Projects dir:"), "",
			new BMessage(MSG_PROJECT_DIRECTORY));
	fProjectsDirectoryText->SetEnabled(false);

	// Peep settings
	TPreferences* prefs = new TPreferences(IdeamNames::kSettingsFileName,
								IdeamNames::kApplicationName, 'IDSE');
	fProjectsDirectoryText->SetText(prefs->GetString("projects_directory"));
	delete prefs;

	fAddFileTC = new BTextControl("addfileTC", B_TRANSLATE("Add file:"), "",
			new BMessage(MSG_ADD_FILE_NAME));
	fAddFileTC->SetEnabled(false);

	fAddHeader = new BCheckBox("AddHeader",
		B_TRANSLATE("Add header"), new BMessage(MSG_ADD_HEADER_TOGGLED));
	fAddHeader->SetEnabled(false);
	fAddHeader->SetValue(B_CONTROL_OFF);

	fAddSecondFileTC = new BTextControl("addSecondfileTC", B_TRANSLATE("Add file:"), "",
			new BMessage(MSG_ADD_SECOND_FILE_NAME));
	fAddSecondFileTC->SetEnabled(false);


	fAddSecondHeader = new BCheckBox("AddSecondHeader",
		B_TRANSLATE("Add header"), new BMessage(MSG_ADD_SECOND_HEADER_TOGGLED));
	fAddSecondHeader->SetEnabled(false);
	fAddSecondHeader->SetValue(B_CONTROL_OFF);

//	BOptionPopUp* fProjectTypeOPU = new BOptionPopUp("ProjectTypeOPU",
//		B_TRANSLATE("Project type:"), new BMessage(MSG_PROJECT_TYPE_CHANGED));

	// Haiku sources app
	fHaikuAppDirTC = new BTextControl("haikuappdirTC", B_TRANSLATE("Haiku app dir:"),
																			"", NULL);
	fHaikuAppDirTC->SetModificationMessage(new BMessage(MSG_HAIKU_APP_EDITED));
	fHaikuAppDirTC->SetEnabled(false);

	fBrowseHaikuAppButton = new BButton(B_TRANSLATE("Browse" B_UTF8_ELLIPSIS),
								new BMessage(MSG_BROWSE_HAIKU_APP_CLICKED));
	fBrowseHaikuAppButton->SetEnabled(false);

	// Local sources app
	fLocalAppDirTC = new BTextControl("localappdirTC", B_TRANSLATE("Local app dir:"),
																			"", NULL);
	fLocalAppDirTC->SetModificationMessage(new BMessage(MSG_LOCAL_APP_EDITED));
	fLocalAppDirTC->SetEnabled(false);

	fBrowseLocalAppButton = new BButton(B_TRANSLATE("Browse" B_UTF8_ELLIPSIS),
								new BMessage(MSG_BROWSE_LOCAL_APP_CLICKED));
	fBrowseLocalAppButton->SetEnabled(false);

	// Cargo app
	fCargoPathText = new BTextControl("CargoPathText", B_TRANSLATE("cargo path:"),
																			"", NULL);
	fCargoPathText->SetText("/bin/cargo");
	fCargoPathText->SetEnabled(false);

	fCargoBin = new BCheckBox("CargoBin", "--bin", nullptr);
	fCargoBin->SetEnabled(false);
	fCargoBin->SetValue(B_CONTROL_ON);
	fCargoBin->SetToolTip(B_TRANSLATE("Library target when unchecked"));

	fCargoVcs = new BCheckBox("CargoVcs", "--vcs none", nullptr);
	fCargoVcs->SetEnabled(false);
	fCargoVcs->SetValue(B_CONTROL_OFF);
	fCargoVcs->SetToolTip(B_TRANSLATE("Disables vcs management when checked"));

	// Open panel
	// TODO read settings file for sources dir
	fOpenPanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL,
							B_DIRECTORY_NODE, false, NULL);

	BLayoutBuilder::Grid<>(fProjectBox)
		.SetInsets(20, 20, 10, 10)
		.Add(fProjectNameText->CreateLabelLayoutItem(), 0, 1)
		.Add(fProjectNameText->CreateTextViewLayoutItem(), 1, 1, 2)
		.Add(fProjectTargetTC->CreateLabelLayoutItem(), 0, 2)
		.Add(fProjectTargetTC->CreateTextViewLayoutItem(), 1, 2, 2)
		.Add(fRunInTeminal, 3, 2)
		.Add(fProjectsDirectoryText->CreateLabelLayoutItem(), 0, 3)
		.Add(fProjectsDirectoryText->CreateTextViewLayoutItem(), 1, 3, 2)
		.Add(fAddFileTC->CreateLabelLayoutItem(), 0, 4)
		.Add(fAddFileTC->CreateTextViewLayoutItem(), 1, 4, 2)
		.Add(fAddHeader, 3, 4)
		.Add(fAddSecondFileTC->CreateLabelLayoutItem(), 0, 5)
		.Add(fAddSecondFileTC->CreateTextViewLayoutItem(), 1, 5, 2)
		.Add(fAddSecondHeader, 3, 5)
//		.Add(fProjectTypeOPU, 0, 6, 2)
		.Add(new BSeparatorView(B_HORIZONTAL), 0, 6, 4)
		.Add(fHaikuAppDirTC->CreateLabelLayoutItem(), 0, 7)
		.Add(fHaikuAppDirTC->CreateTextViewLayoutItem(), 1, 7, 2)
		.Add(fBrowseHaikuAppButton, 3, 7)
		.Add(fLocalAppDirTC->CreateLabelLayoutItem(), 0, 8)
		.Add(fLocalAppDirTC->CreateTextViewLayoutItem(), 1, 8, 2)
		.Add(fBrowseLocalAppButton, 3, 8)
		.Add(new BSeparatorView(B_HORIZONTAL), 0, 9, 4)
		.Add(fCargoPathText->CreateLabelLayoutItem(), 0, 10)
		.Add(fCargoPathText->CreateTextViewLayoutItem(), 1, 10)
		.Add(fCargoBin, 2, 10)
		.Add(fCargoVcs, 3, 10)
		.AddGlue(0, 12)
		;

	// Window layout
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(2)

		.AddGroup(B_HORIZONTAL, 0, 5)
				.Add(typeListScrollView, 2)
					.Add(fProjectBox, 5)
		.End()	
		.AddGroup(B_VERTICAL, 0, 3)	
			.Add(fScrollText)
					.AddGroup(B_HORIZONTAL)
						.Add(fCancelButton)
						.Add(fCreateButton)
						.AddGlue()
					.End()
		.End()
	;

	CenterOnScreen();			
}

NewProjectWindow::~NewProjectWindow()
{

}

bool
NewProjectWindow::QuitRequested()
{
	delete fOpenPanel;

	delete haikuItem;
	delete appItem;
	delete appMenuItem;
	delete genericItem;
	delete helloCplusItem;
	delete helloCItem;
	delete principlesItem;
	delete emptyItem;
	delete importItem;
	delete sourcesItem;
	delete existingItem;
	delete rustItem;
	delete cargoItem;

	BWindow::Quit();
		
	return true;
}

void
NewProjectWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what) {

		case MSG_PROJECT_CANCEL:
			PostMessage(B_QUIT_REQUESTED);
			break;
		case MSG_PROJECT_CREATE:
		{
			_CreateProject();
			break;
		}
		case MSG_PROJECT_CHOSEN:
		{
			_UpdateControlsState(fTypeListView->CurrentSelection());
			break;	
		}
		case MSG_PROJECT_NAME_EDITED:
		{
			_UpdateControlsData(fTypeListView->CurrentSelection());
			break;	
		}
		case MSG_HAIKU_APP_EDITED:
		{
			_OnEditingHaikuAppText();
			break;
		}
		case MSG_BROWSE_HAIKU_APP_CLICKED:
		{
			fOpenPanel->SetMessage(new BMessage(MSG_HAIKU_APP_REFS_RECEIVED));
			fOpenPanel->Show();
			break;
		}
		case MSG_HAIKU_APP_REFS_RECEIVED:
		{
			entry_ref ref;
			if ((msg->FindRef("refs", &ref)) == B_OK) {
				BPath path(&ref);
				fHaikuAppDirTC->SetText(path.Path());
			}
			break;
		}
		case MSG_LOCAL_APP_EDITED:
		{
			_OnEditingLocalAppText();
			break;
		}
		case MSG_BROWSE_LOCAL_APP_CLICKED:
		{
			fOpenPanel->SetMessage(new BMessage(MSG_LOCAL_APP_REFS_RECEIVED));
			fOpenPanel->Show();
			break;
		}
		case MSG_LOCAL_APP_REFS_RECEIVED:
		{
			entry_ref ref;
			if ((msg->FindRef("refs", &ref)) == B_OK) {
				BPath path(&ref);
				fLocalAppDirTC->SetText(path.Path());
			}
			break;
		}

		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}	
}

/**
 * When you click 'Create' button you are driven here.
 * After some validation the _Create... function for the chosen project is called.
 * Some projects may use common functions (e.g. _CreateSkeleton) 
 * while others are "stand alone" (e.g. _CreateCargoProject)
 */
status_t
NewProjectWindow::_CreateProject()
{
	status_t status;
	int32 selection = fTypeListView->CurrentSelection();

	fCurrentItem = dynamic_cast<BStringItem*>(fTypeListView->ItemAt(selection));


	// Little validation for controls
	// Warn if project name is empty
	if ((strcmp(fProjectNameText->Text(), "") == 0)) {
		fProjectDescription->SetText(B_TRANSLATE("Please fill \"Project name\" field"));
		return B_ERROR;
	}

	// Warn if project directory exists
	BPath dirPath(fProjectsDirectoryText->Text());
	dirPath.Append(fProjectNameText->Text());
	BEntry entry(dirPath.Path());
	if (entry.Exists() && fCurrentItem != existingItem) {
		fProjectDescription->SetText(B_TRANSLATE("ERROR: Project directory exists!"));
		return B_ERROR;
	}

	// Warn if project exists
	BPath projectPath;
	BString projectFile(fProjectNameText->Text());
	projectFile.Append(IdeamNames::kProjectExtension); // ".idmpro"

	status = find_directory(B_USER_SETTINGS_DIRECTORY, &projectPath);
	projectPath.Append(IdeamNames::kApplicationName);
	projectPath.Append(projectFile);
	entry.SetTo(projectPath.Path());
	if (entry.Exists()) {
		BString warn;
		warn << B_TRANSLATE("ERROR: Project ") << projectFile
			<< B_TRANSLATE(" exists!");
		fProjectDescription->SetText(warn.String());
		return B_ERROR;
	}

	// Warn if project target is empty if enabled
	if ((!strcmp(fProjectTargetTC->Text(), "") && fProjectTargetTC->IsEnabled())) {
		fProjectDescription->SetText(B_TRANSLATE("Please fill \"Project target\" field"));
		return B_ERROR;
	}

	// Warn if add file is empty if enabled
	if ((!strcmp(fAddFileTC->Text(), "") && fAddFileTC->IsEnabled())) {
		fProjectDescription->SetText(B_TRANSLATE("Please fill \"Add file\" field"));
		return B_ERROR;
	}

	// Create projects directory if not existing
	BPath projectsPath(fProjectsDirectoryText->Text());
	BEntry projectsEntry(projectsPath.Path());
	if (!projectsEntry.Exists()) {
		status = create_directory(projectsPath.Path(), 0755);
		if (status != B_OK)
			return status;
	}


	// Get year
	time_t time = ::time(NULL);
	struct tm* tm = localtime(&time);
	fYear = tm->tm_year + 1900;

	if (fCurrentItem == appItem)
		status = _CreateAppProject();
	else if (fCurrentItem == appMenuItem) {
		// App with menu item, warn on empty add second file
		if (strcmp(fAddSecondFileTC->Text(), "") == 0) {
			fProjectDescription->SetText(B_TRANSLATE("Please fill \"Add file\" field"));
			return B_ERROR;
		}
		status = _CreateAppMenuProject();
	}
	else if (fCurrentItem == helloCplusItem)
		status = _CreateHelloCplusProject();
	else if (fCurrentItem == helloCItem)
		status = _CreateHelloCProject();
	else if (fCurrentItem == principlesItem)
		status = _CreatePrinciplesProject();
	else if (fCurrentItem == emptyItem)
		status = _CreateEmptyProject();
	else if (fCurrentItem == sourcesItem) {
		// Haiku sources item, warn on empty app dir
		if (strcmp(fHaikuAppDirTC->Text(), "") == 0) {
			fProjectDescription->SetText(B_TRANSLATE("Please fill \"Haiku app dir\" field"));
			return B_ERROR;
		}
		status = _CreateHaikuSourcesProject();
	}
	else if (fCurrentItem == existingItem) {
		// Local sources item, warn on empty app dir
		if (strcmp(fLocalAppDirTC->Text(), "") == 0) {
			fProjectDescription->SetText(B_TRANSLATE("Please fill \"Local app dir\" field"));
			return B_ERROR;
		}
		status = _CreateLocalSourcesProject();
	}
	else if (fCurrentItem == cargoItem)
		status = _CreateCargoProject();

	if (status == B_OK) {
		// Post a message
		BMessage message(NEWPROJECTWINDOW_PROJECT_OPEN_NEW);
		message.AddString("project_extensioned_name", fProjectExtensionedName);
		be_app->WindowAt(0)->PostMessage(&message);
		// Quit
		PostMessage(B_QUIT_REQUESTED);
	} else {
		// Delete stale project files if any
		fProjectDescription->Insert(B_TRANSLATE("\n\nERROR: Project creation failed\n\n"));

		_RemoveStaleEntries(dirPath.Path());

		entry.SetTo(dirPath.Path());
		if (entry.Exists()) {
			entry.Remove();
			BString text;
			text << B_TRANSLATE("\nRemoving stale project dir: ") << entry.Name();
			fProjectDescription->Insert(text);
		}

		entry.SetTo(projectPath.Path());
		if (entry.Exists()) {
			entry.Remove();
			BString text;
			text << B_TRANSLATE("\nRemoving stale project file: ") << entry.Name();
			fProjectDescription->Insert(text);
		}
	}

	return status;
}

status_t
NewProjectWindow::_CreateSkeleton()
{
	status_t status;

	BPath path(fProjectsDirectoryText->Text());
	path.Append(fProjectNameText->Text());
	BDirectory projectDirectory;

	status = projectDirectory.CreateDirectory(path.Path(), NULL);
	if (status != B_OK)
		return status;

	// Create "src" and "app" directories
	BPath srcPath(path.Path());
	srcPath.Append("src");
	BDirectory srcDirectory;
	status = srcDirectory.CreateDirectory(srcPath.Path(), NULL);
	if (status != B_OK)
		return status;

	BPath appPath(path.Path());
	appPath.Append("app");
	BDirectory appDirectory;
	status = appDirectory.CreateDirectory(appPath.Path(), NULL);
	if (status != B_OK)
		return status;

	// Project file
	fProjectExtensionedName = fProjectNameText->Text();
	fProjectExtensionedName.Append(IdeamNames::kProjectExtension); // ".idmpro"
	fProjectFile =  new TPreferences(fProjectExtensionedName, IdeamNames::kApplicationName, 'PRSE');

	fProjectFile->SetBString("project_extensioned_name", fProjectExtensionedName);
	fProjectFile->SetBString("project_name", fProjectNameText->Text());
	fProjectFile->SetBString("project_directory", path.Path());
	fProjectFile->SetBString("project_build_command", "make");
	fProjectFile->SetBString("project_clean_command", "make clean rmapp");
	fProjectFile->SetBool("run_in_terminal", fRunInTeminal->Value());

	// Set project target
	BString target(appPath.Path());
	target << "/" << fProjectTargetTC->Text();
	fProjectFile->SetBString("project_target", target);

	return B_OK;
}

status_t
NewProjectWindow::_CreateAppProject()
{
	status_t status;

	if ((status = _CreateSkeleton()) != B_OK)
		return status;

	if ((status = _WriteMakefile()) != B_OK)
		return status;

	if ((status = _WriteAppfiles()) != B_OK)
		return status;

	delete fProjectFile;

	return B_OK;
}

status_t
NewProjectWindow::_CreateAppMenuProject()
{
	status_t status;

	if ((status = _CreateSkeleton()) != B_OK)
		return status;

	fProjectFile->SetBString("project_build_command",
		"make && make bindcatalogs");

	if ((status = _WriteMakefile()) != B_OK)
		return status;

	if ((status = _WriteAppMenufiles()) != B_OK)
		return status;

	delete fProjectFile;

	return B_OK;
}

status_t
NewProjectWindow::_CreateCargoProject()
{
	BPath path(fProjectsDirectoryText->Text());

	// Test cargo presence
	BEntry entry(fCargoPathText->Text(), true);
	if (!entry.Exists()) {
		fProjectDescription->SetText(B_TRANSLATE("cargo binary not found!"));
		return B_ERROR;
	}

	BString args, srcFilename("src/lib.rs");
	args << fProjectNameText->Text();
	if (fCargoBin->Value() == B_CONTROL_ON) {
		args << " --bin";
		srcFilename = "src/main.rs";
	}
	if (fCargoVcs->Value() == B_CONTROL_ON)
		args << " --vcs none";

	// Post a message
	BMessage message(NEWPROJECTWINDOW_PROJECT_CARGO_NEW);
	message.AddString("cargo_new_string", args);
	be_app->WindowAt(0)->PostMessage(&message);

	// Project file
	fProjectExtensionedName = fProjectNameText->Text();
	fProjectExtensionedName.Append(IdeamNames::kProjectExtension); // ".idmpro"
	TPreferences projectFile(fProjectExtensionedName, IdeamNames::kApplicationName, 'PRSE');

	projectFile.SetBString("project_extensioned_name", fProjectExtensionedName);
	projectFile.SetString("project_name", fProjectNameText->Text());
	path.Append(fProjectNameText->Text());
	projectFile.SetString("project_directory", path.Path());
	// Set project target: cargo is able to build & run in one pass, setting
	// target to project directory will enable doing the same in
	// IdeamWindow::_RunTarget()
	projectFile.SetBString("project_target", path.Path());
	projectFile.SetBString("project_build_command", "cargo build");
	projectFile.SetBString("project_clean_command", "cargo clean");
	projectFile.SetBool("run_in_terminal", fRunInTeminal->Value());

	// Cargo specific
	projectFile.SetBString("project_type", "cargo");

	BPath srcPath(path);
	path.Append("Cargo.toml");
	projectFile.AddString("project_file", path.Path());
	srcPath.Append(srcFilename);
	projectFile.AddString("project_source", srcPath.Path());

	return B_OK;
}

status_t
NewProjectWindow::_CreateHelloCplusProject()
{
	status_t status;

	if ((status = _CreateSkeleton()) != B_OK)
		return status;

	if ((status = _WriteMakefile()) != B_OK)
		return status;

	if ((status = _WriteHelloCplusfile()) != B_OK)
		return status;

	delete fProjectFile;

	return B_OK;
}

status_t
NewProjectWindow::_CreateHelloCProject()
{
	status_t status;

	if ((status = _CreateSkeleton()) != B_OK)
		return status;

	if ((status = _WriteHelloCMakefile()) != B_OK)
		return status;

	if ((status = _WriteHelloCfile()) != B_OK)
		return status;

	delete fProjectFile;

	return B_OK;
}

status_t
NewProjectWindow::_CreatePrinciplesProject()
{
	status_t status;

	if ((status = _CreateSkeleton()) != B_OK)
		return status;

	if ((status = _WriteMakefile()) != B_OK)
		return status;

	if ((status = _WritePrinciplesfile()) != B_OK)
		return status;

	delete fProjectFile;

		return B_OK;
}

status_t
NewProjectWindow::_CreateEmptyProject()
{
	status_t status;

	BPath path(fProjectsDirectoryText->Text());
	path.Append(fProjectNameText->Text());
	BDirectory projectDirectory;

	// TODO manage existing
	status = projectDirectory.CreateDirectory(path.Path(), NULL);
	if (status != B_OK)
		return status;

	// Project file
	fProjectExtensionedName = fProjectNameText->Text();
	fProjectExtensionedName.Append(IdeamNames::kProjectExtension); // ".idmpro"
	fProjectFile =  new TPreferences(fProjectExtensionedName, IdeamNames::kApplicationName, 'PRSE');

	fProjectFile->SetBString("project_extensioned_name", fProjectExtensionedName);
	fProjectFile->SetString("project_name", fProjectNameText->Text());
	fProjectFile->SetString("project_directory", path.Path());
	fProjectFile->SetBool("run_in_terminal", fRunInTeminal->Value());

	delete fProjectFile;

	return B_OK;
}

status_t
NewProjectWindow::_CreateHaikuSourcesProject()
{
	status_t status    = B_OK;

	// Project file
	fProjectExtensionedName = fProjectNameText->Text();
	fProjectExtensionedName.Append(IdeamNames::kProjectExtension); // ".idmpro"
	fProjectFile =  new TPreferences(fProjectExtensionedName, IdeamNames::kApplicationName, 'PRSE');

	fProjectFile->SetBString("project_extensioned_name", fProjectExtensionedName);
	fProjectFile->SetString("project_name", fProjectNameText->Text());
	fProjectFile->SetString("project_target", fProjectTargetTC->Text());
	fProjectFile->SetString("project_directory", fHaikuAppDirTC->Text());
	fProjectFile->SetBString("project_build_command", "jam -q");
	fProjectFile->SetBString("project_clean_command", "jam clean");
	fProjectFile->SetBool("run_in_terminal", fRunInTeminal->Value());

	// Scan dir for files
	_GetSourcesFiles(fHaikuAppDirTC->Text());

	delete fProjectFile;

	return status;
}

status_t
NewProjectWindow::_CreateLocalSourcesProject()
{
	status_t status    = B_OK;

	// Project file
	fProjectExtensionedName = fProjectNameText->Text();
	fProjectExtensionedName.Append(IdeamNames::kProjectExtension); // ".idmpro"
	fProjectFile =  new TPreferences(fProjectExtensionedName, IdeamNames::kApplicationName, 'PRSE');

	fProjectFile->SetBString("project_extensioned_name", fProjectExtensionedName);
	fProjectFile->SetString("project_name", fProjectNameText->Text());
	fProjectFile->SetString("project_target", fProjectTargetTC->Text());
	fProjectFile->SetString("project_directory", fLocalAppDirTC->Text());
	fProjectFile->SetBString("project_build_command", "make");
	fProjectFile->SetBString("project_clean_command", "make clean");
	fProjectFile->SetBool("run_in_terminal", fRunInTeminal->Value());

	// Scan dir for files
	_GetSourcesFiles(fLocalAppDirTC->Text());

	delete fProjectFile;

	return status;
}

status_t
NewProjectWindow::_GetSourcesFiles(const char* dirpath)
{
	status_t status;

	BDirectory dir(dirpath);
//	int32 entries = dir.CountEntries();
	BEntry entry;
	entry_ref ref;
	char sname[B_FILE_NAME_LENGTH];

	while ((status = dir.GetNextEntry(&entry)) == B_OK) {

		entry.GetName(sname);
		BString name(sname);

		// Manage directories
		if (entry.IsDirectory()) {

			// Exclude scm dirs TODO: more scms
			if (name == ".git") {
				fProjectFile->SetString("project_scm", "Git");
			} else if (name == ".hg") {
				fProjectFile->SetString("project_scm", "Mercurial");
			} else if (name == ".bzr") {
				fProjectFile->SetString("project_scm", "Bazaar");
			}
			// Exclude objects dir ?
			else if (name.StartsWith("objects.")) {
				;
			}
			// Exclude app dir if any
			else if (name == "app") {
				;
			}
			else {
				BString newPath(dirpath);
				newPath << "/" << entry.Name();
				_GetSourcesFiles(newPath.String());
			}

		} else {
			// Entry is a file, get it
			if ((status = entry.GetRef(&ref)) == B_OK) {
				BPath path(&entry);
				// TODO Exclude target

/*				if (name == "Jamfile"
						|| name == "Makefile"
						|| name == "makefile")
						// TODO Exclude makefile.xxx ?
					fProjectFile->AddString("project_make", path.Path());

				else if (name.EndsWith(".rdef"))
					fProjectFile->AddString("project_rdef", path.Path());

				else */ if (name.EndsWith(".cpp")
						|| name.EndsWith(".c")
						|| name.EndsWith(".h")
						|| name.EndsWith(".S")
						|| name.EndsWith(".awk")
						|| name.EndsWith(".inc"))
					fProjectFile->AddString("project_source", path.Path());
				else if (name.EndsWith(".txt")
						|| name == "LICENSE"
						|| name == "TODO"
						|| name.StartsWith("README")
						||	name.EndsWith(".html"))
					fProjectFile->AddString("project_file", path.Path());
//					fProjectFile->AddString("project_source", path.Path());
				else if (name.EndsWith(".bin")
						|| name == "usbdevs"
						|| name.EndsWith(".hk3d")
						|| name.EndsWith(".png"))
					fProjectFile->AddString("project_file", path.Path());
//					fProjectFile->AddString("project_source", path.Path());
				// Files to exclude, do nothing
				else if (name.EndsWith(".d")
						|| name.EndsWith(".o"))
					;
				else
					fProjectFile->AddString("project_file", path.Path());
//					fProjectFile->AddString("project_source", path.Path());
			}
			else
				return status;
		}
	}
	return B_OK;
}

void
NewProjectWindow::_MapItems()
{
	projectTypeMap.insert(ProjectTypePair(appItem,
		B_TRANSLATE("A simple GUI \"Hello World!\" application")));

	projectTypeMap.insert(ProjectTypePair(appMenuItem,
		B_TRANSLATE("A GUI application with menu bar and localization")));

	BString sourcesItemString;
	sourcesItemString
		<< B_TRANSLATE("An application from Haiku sources repo.\n\n")
		<< B_TRANSLATE("Steps (read Haiku guides if unsure):\n\n")
		<< B_TRANSLATE(") Clone Haiku repo and configure it. \n")
		<< B_TRANSLATE(" e.g. repodir>  git clone https://git.haiku-os.org/haiku\n")
		<< B_TRANSLATE("      repodir/haiku> ./configure --use-xattr-ref --use-gcc-pipe\n\n")
//		") Issue a build to save time.\n"
//		" e.g. repodir/haiku>  jam -q -j2 haiku-image\n\n" // @nightly-raw
		<< B_TRANSLATE("Once done get back here and:\n\n")
		<< B_TRANSLATE(") Select your Haiku app dir (from src/apps) or write full path in text control\n")
		<< B_TRANSLATE(" e.g. /boot/home/repodir/haiku/src/apps/clock\n\n")
		<< B_TRANSLATE(") Accord \"/generated/\" target dir (in \"Project target:\") if needed\n\n")
		<< B_TRANSLATE(") Click \"Create\" button\n\n\n")
		<< B_TRANSLATE("NOTE: sources not copied or moved");
	projectTypeMap.insert(ProjectTypePair(sourcesItem, sourcesItemString));

	projectTypeMap.insert(ProjectTypePair(helloCplusItem,
		B_TRANSLATE("A C++ command line \"Hello World!\" application")));

	projectTypeMap.insert(ProjectTypePair(helloCItem,
		B_TRANSLATE("A C command line \"Hello World!\" application")));

	BString principlesItemString;
	principlesItemString
		<< B_TRANSLATE("An application template for programs"
			" contained in Bjarne Stroustrup' s book:\n\n")
		<< B_TRANSLATE("Programming: Principles and Practice using C++ (2nd edition)\n\n\n")
		<< B_TRANSLATE("Put \"std_lib_facilities.h\" in:\n")
		<< "/boot/home/config/non-packaged/develop/headers\n"
		<< B_TRANSLATE("or edit Makefile variable (SYSTEM_INCLUDE_PATHS) accordingly");
	projectTypeMap.insert(ProjectTypePair(principlesItem, principlesItemString));

	projectTypeMap.insert(ProjectTypePair(emptyItem,
		B_TRANSLATE("A project file with an empty project directory.\n\n"
		"It may be filled in Project->Settings menu")));

	projectTypeMap.insert(ProjectTypePair(existingItem,
		B_TRANSLATE("Existing project with Makefile.\n\n"
		"Accord \"Project target:\" if needed\n\n\n"
//		"NOTE: clean sources before importing\n"
		"NOTE: sources not copied or moved")));

	projectTypeMap.insert(ProjectTypePair(cargoItem,
		B_TRANSLATE("A rust cargo project.\n\n"
		"You should make sure that the rust package (read cargo binary) is installed and working!")));

};

void
NewProjectWindow::_OnEditingHaikuAppText()
{
	BString appdir(fHaikuAppDirTC->Text());

	if (appdir == "")
		return;

	appdir.Replace("/src/", "/generated/objects/haiku/x86_64/release/", 1,
					appdir.FindLast("/src/"));

	BString appname;
	appdir.CopyInto(appname, appdir.FindLast('/') + 1, appdir.Length());
	appname.Capitalize();

	BString target;
	target << appdir << "/" << appname;

	fProjectNameText->SetText(appname);
	fProjectTargetTC->SetText(target);
}

void
NewProjectWindow::_OnEditingLocalAppText()
{
	BString appdir(fLocalAppDirTC->Text());
	if (appdir == "")
		return;

	BString appname;
	appdir.CopyInto(appname, appdir.FindLast('/') + 1, appdir.Length());
	appname.Capitalize();

	BString target, targetPath;
	bool foundTarget = _FindMakefile(target);

	if (foundTarget == true)
		// Target found, set that
		targetPath << appdir << "/" << target;
	else
		// Target not found, make assumptions
		targetPath << appdir << "/" << appname;

	fProjectNameText->SetText(appname);
	fProjectTargetTC->SetText(targetPath);
}

void
NewProjectWindow::_RemoveStaleEntries(const char* dirpath)
{
	BDirectory dir(dirpath);
	BEntry entry;

	while (dir.GetNextEntry(&entry) == B_OK) {
		BString text;

		if (entry.IsDirectory()) {
			BDirectory newdir(&entry);

			if (newdir.CountEntries() > 0) {
				BString newPath(dirpath);
				newPath << "/" << entry.Name();

				_RemoveStaleEntries(newPath.String());
				entry.Remove();
				text << B_TRANSLATE("\nRemoving stale dir: ") << entry.Name();
				fProjectDescription->Insert(text);
			}
			else {
				entry.Remove();
				text << B_TRANSLATE("\nRemoving stale dir: ") << entry.Name();
				fProjectDescription->Insert(text);
			}
		}
		else {
			entry.Remove();
			text << B_TRANSLATE("\nRemoving stale file: ") << entry.Name();
			fProjectDescription->Insert(text);
		}
	}
}

bool
NewProjectWindow::_FindMakefile(BString& target)
{
	BDirectory projectDir(fLocalAppDirTC->Text());
	BEntry entry;
	entry_ref ref;
	char sname[B_FILE_NAME_LENGTH];

	while (projectDir.GetNextEntry(&entry) == B_OK) {
		if (entry.IsFile()) {
				entry.GetName(sname);
				BString name(sname);
				// Standard name first in case there are many
				if (name == "Makefile")
					return _ParseMakefile(target, &entry);
				else if (name == "makefile")
					return _ParseMakefile(target, &entry);
				else if (name.IFindFirst("makefile") != B_ERROR)
					return _ParseMakefile(target, &entry);
		}
	}
	// No target found
	return false;
}

bool
NewProjectWindow::_ParseMakefile(BString& target, const BEntry* entry)
{
	BPath path;
	entry->GetPath(&path);
	std::ifstream file(path.Path());
	std::string str;
	BString targetName, targetString;
	bool nameFound = false;

	if (file.is_open()) {
		while (getline(file, str)) {
			BString line(str.c_str());
			line.Trim();

			// Avoid comments
			if (line.StartsWith("#") == false) {
				// Haiku Makefile (hopefully)
				if (line.StartsWith("NAME")) {
					// Target found
					nameFound = true;
					line.CopyInto(targetName, line.FindLast('=') + 1, line.Length());

					// Empty NAME, trouble
					if (targetName.Trim() == "")
						fProjectDescription->SetText(B_TRANSLATE("ERROR: empty \"NAME\" in Makefile"));

					// Do not break right now, it could be a standard makefile
					// with a NAME variable unset
					continue;
				}
				if (line.StartsWith("TARGET_DIR")) {
					// Target dir found
					line.CopyInto(targetString, line.FindLast('=') + 1, line.Length());

					targetString.Trim();
					// Assuming Haiku makefile: should have NAME set, then TARGET_DIR
					// maybe set. So break and assign target string if:
					// both are found in correct order and NAME not empty
					if (nameFound == true && targetName != "") {
						if (targetString != "" || targetString != ".")
							target << targetString << "/" << targetName;
						else
							target << targetName;

						break;
					}
					continue;

				} else if (line.IFindFirst("TARGET") != B_ERROR) {
					// Traditional Makefile
					line.CopyInto(targetString, line.FindLast('=') + 1, line.Length());

					if (targetString.Trim() != "") {
						target << targetString;
						nameFound = true;
					} else
						fProjectDescription->SetText(B_TRANSLATE("ERROR: empty \"TARGET\" in Makefile"));

					break;
				}
			}
		}
		file.close();
	}

	return nameFound;
}


/**
 * Upon changing project type only relevant controls are enabled.
 *
 */
void
NewProjectWindow::_UpdateControlsState(int32 selection)
{
	if (selection < 0 ) {
		fCreateButton->SetEnabled(false);
		fProjectDescription->SetText(B_TRANSLATE("Invalid selection"));
		return;
	}

	BStringItem *item = dynamic_cast<BStringItem*>(fTypeListView->ItemAt(selection));

	// Find item description in map and display it
	ProjectTypeIterator iter = projectTypeMap.find(item);
	fProjectDescription->SetText(iter->second);

	// Clean controls
	fProjectNameText->SetText("");
	fProjectTargetTC->SetText("");
	fRunInTeminal->SetValue(B_CONTROL_OFF);
	fAddFileTC->SetText("");
	fAddHeader->SetValue(B_CONTROL_OFF);
	fAddSecondFileTC->SetText("");
	fAddSecondHeader->SetValue(B_CONTROL_OFF);
	fHaikuAppDirTC->SetText("");
	fLocalAppDirTC->SetText("");

	// Set controls
	fProjectNameText->SetEnabled(true);
	fProjectTargetTC->SetEnabled(true);
	fRunInTeminal->SetEnabled(false);
	fAddFileTC->SetEnabled(true);
	fAddHeader->SetEnabled(false);
	fAddSecondFileTC->SetEnabled(false);
	fAddSecondHeader->SetEnabled(false);
	fHaikuAppDirTC->SetEnabled(false);
	fBrowseHaikuAppButton->SetEnabled(false);
	fLocalAppDirTC->SetEnabled(false);
	fBrowseLocalAppButton->SetEnabled(false);
	fCargoPathText->SetEnabled(false);
	fCargoBin->SetEnabled(false);
	fCargoVcs->SetEnabled(false);

	if (item == appItem) {

//	fAddHeader->SetEnabled(true);
	fAddHeader->SetValue(B_CONTROL_ON);

	} else if (item == appMenuItem) {

	fAddSecondFileTC->SetEnabled(true);
//	fAddHeader->SetEnabled(true);
//	fAddSecondHeader->SetEnabled(true);
	fAddHeader->SetValue(B_CONTROL_ON);
	fAddSecondHeader->SetValue(B_CONTROL_ON);

	} else if (item == helloCplusItem) {
		//fRunInTeminal->SetEnabled(true);
		fRunInTeminal->SetValue(B_CONTROL_ON);

	} else if (item == helloCItem) {
		//fRunInTeminal->SetEnabled(true);
		fRunInTeminal->SetValue(B_CONTROL_ON);

	} else if (item == principlesItem) {
		fRunInTeminal->SetEnabled(true);
		fRunInTeminal->SetValue(B_CONTROL_ON);

	} else if (item == emptyItem) {

		fRunInTeminal->SetEnabled(true);
		fAddFileTC->SetEnabled(false);

	} else if (item == sourcesItem) {
		fRunInTeminal->SetEnabled(true);
		fAddFileTC->SetEnabled(false);
		fHaikuAppDirTC->SetEnabled(true);
		fBrowseHaikuAppButton->SetEnabled(true);

	} else if (item == existingItem) {

//		fProjectTargetTC->SetEnabled(false);
		fRunInTeminal->SetEnabled(true);
		fAddFileTC->SetEnabled(false);
		fLocalAppDirTC->SetEnabled(true);
		fBrowseLocalAppButton->SetEnabled(true);

	} else if (item == cargoItem) {

		fProjectTargetTC->SetEnabled(false);
		fRunInTeminal->SetEnabled(true);
		fRunInTeminal->SetValue(B_CONTROL_ON);
		fAddFileTC->SetEnabled(false);
		fCargoPathText->SetEnabled(true);
		fCargoBin->SetEnabled(true);
		fCargoVcs->SetEnabled(true);
//	} else if (item == gitItem) {
	}

	fCreateButton->SetEnabled(true);
}

void
NewProjectWindow::_UpdateControlsData(int32 selection)
{
	// No project selected, exit
	if (selection < 0 ) {
		fProjectDescription->SetText(B_TRANSLATE("Please choose a project first"));
		return;
	}

	BStringItem *item = dynamic_cast<BStringItem*>(fTypeListView->ItemAt(selection));

	// Import and Cargo items are managed elsewhere
	if (item == sourcesItem || item == existingItem || item == cargoItem)
		return;

	BString name = fProjectNameText->Text();
	fProjectTargetTC->SetText(name);

	// Get rid of garbage on changing project
	if (name == "")
		return;

	if (item == appItem) {

		name.Append(".cpp");
		fAddFileTC->SetText(name);

	} else if (item == appMenuItem) {

		fProjectTargetTC->SetText(name);
		BString name2(name);
		name.Append("App.cpp");
		fAddFileTC->SetText(name);
		name2.Append("Window.cpp");
		fAddSecondFileTC->SetText(name2);

	} else if (item == helloCplusItem ) {

		name.Append(".cpp");
		fAddFileTC->SetText(name);

	} else if (item == helloCItem ) {

		name.Append(".c");
		fAddFileTC->SetText(name);

	} else if (item == principlesItem ) {

		name.Append(".cpp");
		fAddFileTC->SetText(name);
	}
}

status_t
NewProjectWindow::_WriteMakefile()
{
	status_t status;
	BFile file;
	BPath path(fProjectsDirectoryText->Text());
	path.Append(fProjectNameText->Text());

	path.Append("Makefile");
	status = file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS);
	if (status != B_OK)
		return status;

	BString makefile;
	makefile << "#######################################################"
		"#########################\n"
		<< "## Ideam generated makefile\n"
		<< "##\n\n"
		<< "NAME := " << fProjectTargetTC->Text() << "\n\n"
		<< "TARGET_DIR := app\n\n"
		<< "TYPE := APP\n\n"// TODO get
		<< "APP_MIME_SIG := \"application/x-vnd.Ideam-"
		<< fProjectTargetTC->Text() <<"\"\n\n";

	if (fCurrentItem == appMenuItem)
		makefile << "SRCS :=  src/" << fAddFileTC->Text() << "\n"
					"SRCS +=  src/" << fAddSecondFileTC->Text() << "\n\n";
	else
		makefile << "SRCS :=  src/" << fAddFileTC->Text() << "\n\n";

		makefile << "RDEFS :=  \n\n";

	if (fCurrentItem == appItem)
		makefile << "LIBS = be $(STDCPPLIBS)\n\n";
	else if (fCurrentItem == appMenuItem)
		makefile << "LIBS = be localestub $(STDCPPLIBS)\n\n";
	else
		makefile << "LIBS = $(STDCPPLIBS)\n\n";
		

	makefile << "LIBPATHS =\n\n";

	if (fCurrentItem == principlesItem) {
		makefile << "SYSTEM_INCLUDE_PATHS += /boot/home/config/non-packaged/develop/headers/\n\n";
	}

	makefile << "OPTIMIZE := FULL\n\n";

	if (fCurrentItem == appMenuItem)
		makefile << "CFLAGS := -Wall -Werror\n\n";
	else
		makefile << "CFLAGS := -Wall\n\n";

	makefile << "CXXFLAGS := -std=c++11\n\n"
		<< "LOCALES :=\n\n"
		<< "DEBUGGER := TRUE\n\n"
		<< "## Include the Makefile-Engine\n"
		<< "DEVEL_DIRECTORY := \\\n"
		<< "\t$(shell findpaths -r \"makefile_engine\" B_FIND_PATH_DEVELOP_DIRECTORY)\n"
		<< " include $(DEVEL_DIRECTORY)/etc/makefile-engine\n\n"
		// TODO hope to be merged upstream
		<< "$(OBJ_DIR)/%.o : %.cpp\n"
		<< "\t$(C++) -c $< $(INCLUDES) $(CFLAGS) $(CXXFLAGS) -o \"$@\"\n";

	ssize_t bytes = file.Write(makefile.String(), makefile.Length());
	if (bytes != makefile.Length())
		return B_ERROR; // TODO tune
	file.Flush();

//	fProjectFile->AddString("project_make", path.Path());
	fProjectFile->AddString("project_file", path.Path());
	return B_OK;
}

status_t
NewProjectWindow::_WriteAppfiles()
{
	status_t status;
	BFile file;
	BPath cppPath(fProjectsDirectoryText->Text());
	cppPath.Append(fProjectNameText->Text());
	cppPath.Append("src");

	BPath hPath(cppPath.Path());

	BString fileName(fAddFileTC->Text());
	fileName.Remove(fileName.FindLast('.'), fileName.Length());
	BString hFileName(fileName);
	hFileName.Append(".h");

	BString headComment;
	headComment << "/*\n"
				<< " * Copyright " << fYear << " Your Name <your@email.address>\n"
				<< " * All rights reserved. Distributed under the terms of the MIT license.\n"
				<< " */\n\n";

	if (fAddHeader->Value() == true) {
		// TODO add _ on upper letter
		BString upperFileName(fileName);
		upperFileName.ToUpper();

		hPath.Append(hFileName);
		status = file.SetTo(hPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS);
		if (status != B_OK)
			return status;

		BString hFileContent;
		hFileContent << headComment
			<< "#ifndef " << upperFileName << "_H\n"
			<< "#define " << upperFileName << "_H\n\n"
			<< "#include <Application.h>\n\n"
			<< "class " << fileName << " : public BApplication {\n"
			<< "public:\n"
			<< "\t\t\t\t\t\t\t\t" << fileName << "();\n"
			<< "\tvirtual\t\t\t\t\t\t~" << fileName << "();\n\n"
			<< "private:\n\n"
			<< "};\n\n"
			<< "#endif //" << upperFileName << "_H\n";

		ssize_t bytes = file.Write(hFileContent.String(), hFileContent.Length());
		if (bytes != hFileContent.Length())
			return B_ERROR; // TODO tune
		file.Flush();

		fProjectFile->AddString("project_source", hPath.Path());
	}

	cppPath.Append(fAddFileTC->Text());
	status = file.SetTo(cppPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS);
	if (status != B_OK)
		return status;

	BString cppFileContent;
	cppFileContent << headComment
		<< "#include \"" << hFileName <<  "\"\n\n"
		<< "#include <String.h>\n"
		<< "#include <StringView.h>\n"
		<< "#include <Window.h>\n\n"
		<< "BString kApplicationSignature(\"application/x-vnd.Ideam-"
		<< fileName << "\");\n\n"
		<< fileName << "::" << fileName << "()\n"
		<< "\t:\n"
		<< "\tBApplication(kApplicationSignature)\n"
		<< "{\n"
		<< "\tBWindow* window = new BWindow(BRect(100, 100, 599, 399), "
		<< "\"" << fileName << "\",\n"
		<< "\t\tB_TITLED_WINDOW, B_QUIT_ON_WINDOW_CLOSE);\n\n"
		<< "\tBStringView* stringView = new BStringView("
		<< "\"StringView\", \"Hello World!\");\n"
		<< "\tstringView->ResizeToPreferred();\n"
		<< "\twindow->AddChild(stringView);\n\n"
		<< "\twindow->Show();\n"
		<< "}\n\n"
		<< fileName << "::~" << fileName << "()\n"
		<< "{\n"
		<< "}\n\n"
		<< "int\n"
		<< "main(int argc, char* argv[])\n"
		<< "{\n"
		<< "\t" << fileName << " app;\n"
		<< "\tapp.Run();\n\n"
		<< "\treturn 0;\n"
		<< "}\n";

	ssize_t bytes = file.Write(cppFileContent.String(), cppFileContent.Length());
	if (bytes != cppFileContent.Length())
		return B_ERROR; // TODO tune
	file.Flush();

	fProjectFile->AddString("project_source", cppPath.Path());

	return B_OK;
}

status_t
NewProjectWindow::_WriteAppMenufiles()
{
	// Files path
	status_t status;
	BFile file;
	BPath cppPath(fProjectsDirectoryText->Text());
	cppPath.Append(fProjectNameText->Text());
	cppPath.Append("src");
	BPath cpp2Path(cppPath);

	// Headers path
	BPath hPath(cppPath.Path());
	BPath hSecondPath(cppPath.Path());

	BString fileName(fAddFileTC->Text());
	BString fileSecondName(fAddSecondFileTC->Text());
	BString fileNameStripped, fileSecondNameStripped;

	// Manage extension
	fileName.CopyInto(fileNameStripped, 0, fileName.FindLast("."));
	fileSecondName.CopyInto(fileSecondNameStripped, 0, fileSecondName.FindLast("."));
	BString hFileName(fileNameStripped);
	BString hSecondFileName(fileSecondNameStripped);
	hFileName.Append(".h");
	hSecondFileName.Append(".h");

	BString headComment;
	headComment << "/*\n"
				<< " * Copyright " << fYear << " Your Name <your@email.address>\n"
				<< " * All rights reserved. Distributed under the terms of the MIT license.\n"
				<< " */\n\n";

	// App header
	if (fAddHeader->Value() == true) {
		// TODO add _ on upper letter
		BString upperFileName(fileNameStripped);
		upperFileName.ToUpper();

		hPath.Append(hFileName);
		status = file.SetTo(hPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS);
		if (status != B_OK)
			return status;

		BString hFileContent;
		hFileContent << headComment
			<< "#ifndef " << upperFileName << "_H\n"
			<< "#define " << upperFileName << "_H\n\n"
			<< "#include <Application.h>\n\n"
			<< "class " << fileNameStripped << " : public BApplication {\n"
			<< "public:\n"
			<< "\t\t\t\t\t\t\t\t" << fileNameStripped << "();\n"
			<< "\tvirtual\t\t\t\t\t\t~" << fileNameStripped << "();\n\n"
			<< "private:\n\n"
			<< "};\n\n"
			<< "#endif //" << upperFileName << "_H\n";

		ssize_t bytes = file.Write(hFileContent.String(), hFileContent.Length());
		if (bytes != hFileContent.Length())
			return B_ERROR; // TODO tune
		file.Flush();

		fProjectFile->AddString("project_source", hPath.Path());
	}

	// Window header
	if (fAddSecondHeader->Value() == true) {
		// TODO add _ on upper letter
		BString upperFileName(fileSecondNameStripped);
		upperFileName.ToUpper();

		hSecondPath.Append(hSecondFileName);
		status = file.SetTo(hSecondPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS);
		if (status != B_OK)
			return status;

		// TODO Camel case ?
		BString h2FileContent;
		h2FileContent << headComment
			<< "#ifndef " << upperFileName << "_H\n"
			<< "#define " << upperFileName << "_H\n\n"
			<< "#include <GroupLayout.h>\n"
			<< "#include <Window.h>\n\n"
			<< "class " << fileSecondNameStripped << " : public BWindow\n"
			<< "{\n"
			<< "public:\n"
			<< "\t\t\t\t\t\t\t\t" << fileSecondNameStripped << "();\n"
			<< "\tvirtual\t\t\t\t\t\t~" << fileSecondNameStripped << "();\n\n"
			<< "\tvirtual void\t\t\t\tMessageReceived(BMessage* message);\n\n"
			<< "private:\n"
			<< "\t\t\tBGroupLayout*\t\tfRootLayout;\n"
			<< "};\n\n"
			<< "#endif //" << upperFileName << "_H\n";

		ssize_t bytes = file.Write(h2FileContent.String(), h2FileContent.Length());
		if (bytes != h2FileContent.Length())
			return B_ERROR; // TODO tune
		file.Flush();

		fProjectFile->AddString("project_source", hSecondPath.Path());
	}

	// App file
	cppPath.Append(fAddFileTC->Text());
	status = file.SetTo(cppPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS);
	if (status != B_OK)
		return status;

	BString cppFileContent;
	cppFileContent << headComment
		<< "#include \"" << hFileName <<  "\"\n\n"
		<< "#include <String.h>\n\n"
		<< "#include \"" << hSecondFileName <<  "\"\n\n"

		<< "BString kApplicationSignature(\"application/x-vnd.Ideam-"
		<< fileNameStripped << "\");\n\n"
		<< fileNameStripped << "::" << fileNameStripped << "()\n"
		<< "\t:\n"
		<< "\tBApplication(kApplicationSignature)\n"
		<< "{\n"
		<< "\t" << fileSecondNameStripped << "* window = new "
		<< fileSecondNameStripped << "();\n"
		<< "\twindow->Show();\n"
		<< "}\n\n"

		<< fileNameStripped << "::~" << fileNameStripped << "()\n"
		<< "{\n"
		<< "}\n\n"

		<< "int\n"
		<< "main(int argc, char* argv[])\n"
		<< "{\n"
		<< "\ttry {\n"
		<< "\t\tnew " << fileNameStripped << "();\n\n"

		<< "\t\tbe_app->Run();\n\n"

		<< "\t\tdelete be_app;\n\n"

		<< "\t} catch (...) {\n\n"

		<< "\t\tdebugger(\"Exception caught.\");\n"
		<< "\t}\n\n"
		<< "\treturn 0;\n"
		<< "}\n";

	ssize_t bytes = file.Write(cppFileContent.String(), cppFileContent.Length());
	if (bytes != cppFileContent.Length())
		return B_ERROR; // TODO tune
	file.Flush();

	fProjectFile->AddString("project_source", cppPath.Path());

	// Window file
	cpp2Path.Append(fAddSecondFileTC->Text());
	status = file.SetTo(cpp2Path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS);
	if (status != B_OK)
		return status;

	BString cpp2FileContent;
	cpp2FileContent << headComment
		<< "#include \"" << hSecondFileName <<  "\"\n\n"
		<< "#include <Catalog.h>\n"
		<< "#include <LayoutBuilder.h>\n"
		<< "#include <MenuBar.h>\n\n"

		<< "#undef B_TRANSLATION_CONTEXT\n"
		<< "#define B_TRANSLATION_CONTEXT \"" << fileSecondNameStripped << "\"\n\n"

		<< fileSecondNameStripped << "::" << fileSecondNameStripped << "()\n"
		<< "\t:\n"
		<< "\tBWindow(BRect(100, 100, 499, 399), \""
		<< fProjectNameText->Text() << "\", B_TITLED_WINDOW,\n"
		<< "\t\t\t\t\t\t\t\t\t\t\t\tB_QUIT_ON_WINDOW_CLOSE)\n"
		<< "{\n"
		<< "\t// Menu\n"
		<< "\tBMenuBar *menuBar = new BMenuBar(\"menubar\");\n"
		<< "\tBLayoutBuilder::Menu<>(menuBar)\n"
		<< "\t\t.AddMenu(B_TRANSLATE(\"File\"))\n"
		<< "\t\t.AddItem(B_TRANSLATE(\"Quit\"), B_QUIT_REQUESTED, 'Q')\n"
		<< "\t\t.End();\n\n"

		<< "\t// Layout\n"
		<< "\tfRootLayout = BLayoutBuilder::Group<>(this, B_VERTICAL)\n"
		<< "\t\t.SetInsets(0, 0, 0, 0)\n"
		<< "\t\t.Add(menuBar, 0)\n"
		<< "\t\t.AddGlue();\n"
		<< "}\n\n"

		<< fileSecondNameStripped << "::~" << fileSecondNameStripped << "()\n"
		<< "{\n"
		<< "}\n\n"

		<< "void\n"
		<< fileSecondNameStripped << "::MessageReceived(BMessage* message)\n"
		<< "{\n"
		<< "\tswitch (message->what) {\n\n"

		<< "\t\tdefault:\n"
		<< "\t\t\tBWindow::MessageReceived(message);\n"
		<< "\t\t\tbreak;\n"
		<< "\t}\n"
		<< "}\n";

	bytes = file.Write(cpp2FileContent.String(), cpp2FileContent.Length());
	if (bytes != cpp2FileContent.Length())
		return B_ERROR; // TODO tune
	file.Flush();

	fProjectFile->AddString("project_source", cpp2Path.Path());

	return B_OK;
}

status_t
NewProjectWindow::_WriteHelloCplusfile()
{
	status_t status;
	BFile file;
	BPath cppPath(fProjectsDirectoryText->Text());
	cppPath.Append(fProjectNameText->Text());
	cppPath.Append("src");
	cppPath.Append(fAddFileTC->Text());
	status = file.SetTo(cppPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS);
	if (status != B_OK)
		return status;

	BString cppFileContent;
	cppFileContent
		<< "#include <iostream>\n"
		<< "#include <string>\n\n"

		<< "int main(int argc, char* argv[])\n"
		<< "{\n"
		<< "\tstd::string helloString(\"Hello world!\");\n\n"
			
		<< "\tstd::cout << helloString << std::endl;\n\n"
			
		<< "\treturn 0;\n"
		<< "}\n";

	ssize_t bytes = file.Write(cppFileContent.String(), cppFileContent.Length());
	if (bytes != cppFileContent.Length())
		return B_ERROR; // TODO tune
	file.Flush();

	fProjectFile->AddString("project_source", cppPath.Path());

	return B_OK;
}

status_t
NewProjectWindow::_WriteHelloCfile()
{
	status_t status;
	BFile file;
	BPath cPath(fProjectsDirectoryText->Text());
	cPath.Append(fProjectNameText->Text());
	cPath.Append("src");
	cPath.Append(fAddFileTC->Text());
	status = file.SetTo(cPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS);
	if (status != B_OK)
		return status;

	BString cFileContent;
	cFileContent
			<< "#include <stdio.h>\n\n"

			<< "void print(const char* message)\n"
			<< "{\n"
			<< "\tfprintf(stdout, \"\%s\\n\", message);\n"
			<< "}\n\n"

			<< "int\n"
			<< "main(int argc, char* argv[])\n"
			<< "{\n"
			<< "\tprint(\"Hello World!\");\n\n"
			<< "\treturn 0;\n"
			<< "}\n";

	ssize_t bytes = file.Write(cFileContent.String(), cFileContent.Length());
	if (bytes != cFileContent.Length())
		return B_ERROR; // TODO tune
	file.Flush();

	fProjectFile->AddString("project_source", cPath.Path());

	return B_OK;
}

status_t
NewProjectWindow::_WritePrinciplesfile()
{
	status_t status;
	BFile file;
	BPath cppPath(fProjectsDirectoryText->Text());
	cppPath.Append(fProjectNameText->Text());
	cppPath.Append("src");
	cppPath.Append(fAddFileTC->Text());
	status = file.SetTo(cppPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS);
	if (status != B_OK)
		return status;

	BString cppFileContent;
	cppFileContent
		<< "#include \"std_lib_facilities.h\"\n\n"

		<< "int main()\n"
		<< "{\n"
		<< "\tcout << \"Hello, World!\\n\";\n"
		<< "\treturn 0;\n"
		<< "}\n";

	ssize_t bytes = file.Write(cppFileContent.String(), cppFileContent.Length());
	if (bytes != cppFileContent.Length())
		return B_ERROR; // TODO tune
	file.Flush();

	fProjectFile->AddString("project_source", cppPath.Path());

	return B_OK;
}

status_t
NewProjectWindow::_WriteHelloCMakefile()
{
	status_t status;
	BFile file;
	BPath path(fProjectsDirectoryText->Text());
	path.Append(fProjectNameText->Text());

	path.Append("Makefile");
	status = file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_FAIL_IF_EXISTS);
	if (status != B_OK)
		return status;

	BString makefile;
	makefile << "#######################################################"
		<< "#########################\n"
		<< "CC		:= gcc\n"
		<< "C++		:= g++\n"
		<< "LD		:= gcc\n\n"

		<< "# Determine the CPU type\n"
		<< "CPU = $(shell uname -m)\n"
		<< "# Get the compiler version.\n"
		<< "CC_VER = $(word 1, $(subst -, , $(subst ., , $(shell $(CC) -dumpversion))))\n\n"

		<< "DEBUG    := -g -ggdb\n"
		<< "CFLAGS   := -c -Wall ${DEBUG} -O2\n"
		<< "CXXFLAGS := -std=c++11\n"
		<< "ASFLAGS   =\n"
		<< "LDFLAGS   = -Xlinker -soname=_APP_\n"
		<< "LIBS	  = -lstdc++ -lsupc++ \n\n"

		<< "#######################################################"
		<< "#########################\n"
		<< "# Vars\n"
		<< "#\n\n"

		<< "# application name\n"
		<< "target	:= app/" << fProjectTargetTC->Text() << "\n\n"

		<< "# sources directories\n"
		<< "dirs	:= src\n\n"

		<< "# find sources\n"
		<< "sources := $(foreach dir, $(dirs), $(wildcard $(dir)/*.cpp $(dir)/*.c  $(dir)/*.S))\n\n"
//		<< "sources := $(foreach dir, $(dirs), $(wildcard $(dir)/*.[cpp,c,S]))\n\n"
		<< "# object files top directory\n"
		<< "objdir  := objects.$(CPU)-$(CC)$(CC_VER)-$(if $(DEBUG),debug,release)\n\n"

		<< "# ensue the object files\n"
		<< "objects :=	$(patsubst \%.S, $(objdir)/\%.o, $(filter \%.S,$(sources))) \\\n"
		<< "\t\t\t$(patsubst \%.c, $(objdir)/\%.o, $(filter \%.c,$(sources))) \\\n"
		<< "\t\t\t$(patsubst \%.cpp, $(objdir)/\%.o, $(filter \%.cpp,$(sources)))\n\n"

		<< "# object files subdirs\n"
		<< "objdirs := $(addprefix $(objdir)/, $(dirs))\n\n"

		<< "#######################################################"
		<< "#########################\n"

		<< "all: $(objdirs) $(target)\n\n"

		<< "$(objdir)/\%.o: \%.c\n"
		<< "\t${CC}  ${CFLAGS} $(includes) $< -o $@\n\n"

		<< "$(objdir)/\%.o: \%.cpp\n"
		<< "\t${C++}  ${CFLAGS} $(CXXFLAGS) $(includes) $< -o $@\n\n"
			
		<< "$(objdir)/\%.o: \%.S\n"
		<< "\t${CC} ${CFLAGS} ${ASFLAGS} $(includes) $< -o $@\n\n"

		<< "$(target): $(objects)\n"
		<< "\t${LD} $(DEBUG) ${LDFLAGS} ${LIBS} -o $@ $^\n\n"

		<< "# create needed dirs to store object files\n"
		<< "$(objdirs):\n"
		<< "\t@echo Creating dir: $@\n"
		<< "\t@mkdir -p  $@\n\n"

		<< "clean:\n"
		<< "\t@echo cleaning: $(objdir)\n"
		<< "\t@rm -f $(objects)\n\n"

		<< "rmapp:\n"
		<< "\t@echo cleaning: $(target)\n"
		<< "\t@rm -f $(target)\n\n"

		<< "list:\n"
		<< "\t@echo srcs: $(sources)\n"
		<< "\t@echo objs: $(objects)\n"
		<< "\t@echo dirs: $(objdirs)\n\n"

		<< "print-defines:\n"
		<< "\t@echo | ${CC} -dM -E -\n\n\n"


		<< ".PHONY:  clean rmapp list print-defines objdirs\n";


	ssize_t bytes = file.Write(makefile.String(), makefile.Length());
	if (bytes != makefile.Length())
		return B_ERROR; // TODO tune
	file.Flush();

//	fProjectFile->AddString("project_make", path.Path());
	fProjectFile->AddString("project_file", path.Path());

	return B_OK;
}
