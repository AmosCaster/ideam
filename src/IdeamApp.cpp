/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IdeamApp.h"

#include <AboutWindow.h>
#include <Catalog.h>
#include <String.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "IdeamApp"

const char* kApplicationSignature = "application/x-vnd.am-Ideam";
const char* kApplicationName = B_TRANSLATE_SYSTEM_NAME("Ideam");
static const char * kSettingsFileName = "ui.settings";

IdeamApp::IdeamApp()
	:
	BApplication(kApplicationSignature)
{
	BRect frame;

	// Load UI settings
	fUISettingsFile = new TPreferences(kSettingsFileName, kApplicationName);

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
	BAboutWindow* window = new BAboutWindow(kApplicationName,
		kApplicationSignature);
	
	// create the about window
	const char* authors[] = {
		"A. Mosca",
		NULL
	}; 

	BString extraInfo;

	window->AddCopyright(2017, "A. Mosca");
	window->AddAuthors(authors);
	extraInfo << kApplicationName << " " << B_TRANSLATE("uses") << ":\n";
	extraInfo << "Scintilla lib";
	extraInfo << "\nCopyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>";

	extraInfo << "\n\nScintilla for Haiku";
	extraInfo << "\nCopyright 2011 by Andrea Anzani <andrea.anzani@gmail.com>";
	extraInfo << "\nCopyright 2014-2015 by Kacper Kasper <kacperkasper@gmail.com>";

	window->AddExtraInfo(extraInfo);

	window->Show();
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

int
main(int argc, char* argv[])
{
	try {
		new IdeamApp();

		be_app->Run();

		delete be_app;

	} catch (...) {

		debugger("Exception caught.");
	}

	return 0;
}
