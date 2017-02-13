/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IdeamWindow.h"

#include <Application.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <MenuBar.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "IdeamWindow"

IdeamWindow::IdeamWindow(BRect frame)
	:
	BWindow(frame, "Ideam", B_TITLED_WINDOW,
												B_QUIT_ON_WINDOW_CLOSE)
{
	// Menu
	BMenuBar *menuBar = new BMenuBar("menubar");
	BLayoutBuilder::Menu<>(menuBar)
		.AddMenu(B_TRANSLATE("Project"))
			.AddItem(B_TRANSLATE("Quit"), B_QUIT_REQUESTED, 'Q')
		.End()
		.AddMenu(B_TRANSLATE("Help"))
			.AddItem(B_TRANSLATE("About" B_UTF8_ELLIPSIS), B_ABOUT_REQUESTED)
		.End()
	.End();

	// Layout
	fRootLayout = BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(0, 0, 0, 0)
		.Add(menuBar, 0)
		.AddGlue()
	;
}

IdeamWindow::~IdeamWindow()
{
}

void
IdeamWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_ABOUT_REQUESTED:
			be_app->PostMessage(B_ABOUT_REQUESTED);
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
