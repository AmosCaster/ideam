/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IdeamApp.h"

#include <Alert.h>
#include <AboutWindow.h>
#include <Catalog.h>
#include <String.h>
#include <iostream>

#include "IdeamNamespace.h"
#include "SettingsWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "IdeamApp"


IdeamApp::IdeamApp()
	:
	BApplication(IdeamNames::kApplicationSignature)
{
	BRect frame;

	// Load UI settings
	fUISettingsFile = new TPreferences(IdeamNames::kUISettingsFileName,
										IdeamNames::kApplicationName, 'UISE');

	// Load frame from settings if present or use default
	if (fUISettingsFile->FindRect("ui_bounds", &frame) != B_OK)
		frame.Set(40, 40, 839, 639);

	fIdeamWindow = new IdeamWindow(frame);

	fIdeamWindow->Show();
}

IdeamApp::~IdeamApp()
{
}

void
IdeamApp::AboutRequested()
{
	BAboutWindow* window = new BAboutWindow(IdeamNames::kApplicationName,
											IdeamNames::kApplicationSignature);
	
	// create the about window
	const char* authors[] = {
		"A. Mosca",
		NULL
	}; 

	window->AddCopyright(2017, "A. Mosca");
	window->AddAuthors(authors);

	BString extraInfo;
	extraInfo << B_TRANSLATE("available under the MIT license.");
	extraInfo << "\n\n";
	extraInfo << IdeamNames::kApplicationName << " " << B_TRANSLATE("uses:");
	extraInfo << "\nScintilla lib";
	extraInfo << "\nCopyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>";
	extraInfo << "\n\nScintilla for Haiku";
	extraInfo << "\nCopyright 2011 by Andrea Anzani <andrea.anzani@gmail.com>";
	extraInfo << "\nCopyright 2014-2015 by Kacper Kasper <kacperkasper@gmail.com>\n\n";
	extraInfo << B_TRANSLATE("See Credits for a complete list.");

	window->AddExtraInfo(extraInfo);

	window->Show();
}

void
IdeamApp::ArgvReceived(int32 agrc, char** argv)
{
}

void
IdeamApp::MessageReceived(BMessage* message)
{
	switch (message->what) {

		default:
			BApplication::MessageReceived(message);
			break;
	}
	
}

bool
IdeamApp::QuitRequested()
{	
	// Manage settings counter
	int32 count;
	if (fUISettingsFile->FindInt32("use_count", &count) != B_OK)
		count = 0;

	// Check if window position was modified
	BRect actualFrame, savedFrame;
	fUISettingsFile->FindRect("ui_bounds", &savedFrame);
	actualFrame = fIdeamWindow->ConvertToScreen(fIdeamWindow->Bounds());
	
	// Automatically save window position via TPreferences
	// only if modified
	if (actualFrame != savedFrame) {
		// Check if settings are available and apply
		if (fUISettingsFile->InitCheck() == B_OK) {
			fUISettingsFile->SetRect("ui_bounds", actualFrame);
			fUISettingsFile->SetInt64("last_used", real_time_clock());
			fUISettingsFile->SetInt32("use_count", ++count);
		}
	}
	
	delete fUISettingsFile;
	
	be_app->PostMessage(B_QUIT_REQUESTED);
	
	return true;
}

void
IdeamApp::RefsReceived(BMessage* message)
{
	fIdeamWindow->PostMessage(message);
}

void
IdeamApp::ReadyToRun()
{
	// Window Settings file needs updating?
	_CheckSettingsVersion();

	std::cerr << IdeamNames::GetSignature() << std::endl;
}

void
IdeamApp::_CheckSettingsVersion()
{
	BString fileVersion("");
	int32 result;

	TPreferences* settings = new TPreferences(IdeamNames::kSettingsFileName,
												IdeamNames::kApplicationName, 'IDSE');
	settings->FindString("app_version", &fileVersion);
	delete settings;

	// Settings file missing or corrupted
	if (fileVersion.IsEmpty() || fileVersion == "0.0.0.0") {

		BString text;
		text << B_TRANSLATE("Settings file is corrupted or deleted,")
			 << "\n"
			 << B_TRANSLATE("do You want to ignore, review or load to defaults?");

		BAlert* alert = new BAlert("SettingsDeletedDialog", text,
			B_TRANSLATE("Ignore"), B_TRANSLATE("Review"), B_TRANSLATE("Load"),
			B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);

		alert->SetShortcut(0, B_ESCAPE);

		int32 choice = alert->Go();
	 
		if (choice == 0)
			;
		else if (choice == 1) {
			// Fill file with defaults
			IdeamNames::UpdateSettingsFile();
			SettingsWindow* window = new SettingsWindow();
			window->Show();
		}
		else if (choice == 2) {
			IdeamNames::UpdateSettingsFile();
			IdeamNames::LoadSettingsVars();
		}
	}
	else {

		result = IdeamNames::CompareVersion(IdeamNames::GetVersionInfo(), fileVersion);
		// App version > file version
		if (result > 0) {

			BString text;
			text << B_TRANSLATE("Settings file for a previous version detected,")
				 << "\n"
				 << B_TRANSLATE("do You want to ignore, review or load to defaults?");

			BAlert* alert = new BAlert("SettingsUpdateDialog", text,
				B_TRANSLATE("Ignore"), B_TRANSLATE("Review"), B_TRANSLATE("Load"),
				B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);

			alert->SetShortcut(0, B_ESCAPE);

			int32 choice = alert->Go();
		 
			if (choice == 0)
				;
			else if (choice == 1) {
				SettingsWindow* window = new SettingsWindow();
				window->Show();
			}
			else if (choice == 2) {
				IdeamNames::UpdateSettingsFile();
				IdeamNames::LoadSettingsVars();
			}
		}
	}
}

int
main(int argc, char* argv[])
{
	try {
		IdeamApp *app = new IdeamApp();

		app->Run();

		delete app;
	} catch (std::out_of_range) {
		std::cerr << "OUT OF RANGE" << std::endl;

	} catch (...) {

		debugger("Exception caught.");

	}

	return 0;
}
