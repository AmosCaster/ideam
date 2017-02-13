/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IdeamApp.h"

#include <AboutWindow.h>
#include <Catalog.h>
#include <String.h>

#include "IdeamWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "IdeamApp"

const char* kApplicationSignature = "application/x-vnd.am-Ideam";
const char* kApplicationName = B_TRANSLATE_SYSTEM_NAME("Ideam");

IdeamApp::IdeamApp()
	:
	BApplication(kApplicationSignature)
{
	BRect frame;

	frame.Set(40, 40, 839, 639);

	IdeamWindow* window = new IdeamWindow(frame);

	window->Show();
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
