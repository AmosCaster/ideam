/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IdeamWindow.h"

#include <Application.h>
#include <Catalog.h>
#include <IconUtils.h>
#include <LayoutBuilder.h>
#include <MenuBar.h>
#include <Resources.h>
#include <SeparatorView.h>

#include <assert.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "IdeamWindow"

//static const auto kToolBarSize = 24;
static float kProjectsWeight = 1.0f;
static float kEditorWeight  = 3.0f;
static float kOutputWeight  = 0.4f;


enum {
	// File menu
	MSG_FILE_NEW				= 'fnew',
	MSG_FILE_OPEN				= 'fope',

	MSG_SHOW_HIDE_PROJECTS		= 'shpr',
	MSG_SHOW_HIDE_OUTPUT		= 'shou',
};

IdeamWindow::IdeamWindow(BRect frame)
	:
	BWindow(frame, "Ideam", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS |
												B_QUIT_ON_WINDOW_CLOSE)
	,fCurrentEditorIndex(-1)
{
	// Menu
	BMenuBar* menuBar = new BMenuBar("menubar");
	BLayoutBuilder::Menu<>(menuBar)
		.AddMenu(B_TRANSLATE("Project"))
			.AddItem(B_TRANSLATE("Quit"), B_QUIT_REQUESTED, 'Q')
		.End()
		.AddMenu(B_TRANSLATE("File"))
			.AddItem(B_TRANSLATE("New"), MSG_FILE_NEW, 'N')
			.AddItem(B_TRANSLATE("Open"), MSG_FILE_OPEN, 'O')
		.End()
		.AddMenu(B_TRANSLATE("Help"))
			.AddItem(B_TRANSLATE("About" B_UTF8_ELLIPSIS), B_ABOUT_REQUESTED)
		.End()
	.End();

	// toolbar group
	fProjectsButton = _LoadIconButton("ProjectsButton", MSG_SHOW_HIDE_PROJECTS,
						111, true, B_TRANSLATE("Show/Hide Projects split"));
	fOutputButton = _LoadIconButton("OutputButton", MSG_SHOW_HIDE_OUTPUT,
						115, true, B_TRANSLATE("Show/Hide Output split"));

	BGroupLayout* toolBar = BLayoutBuilder::Group<>(B_VERTICAL, 0.1)
		.Add(BLayoutBuilder::Group<>(B_HORIZONTAL, 1)
			.Add(fProjectsButton)
			.Add(fOutputButton)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.AddGlue()
			.SetInsets(1, 1, 1, 1)
		)
		.Add(new BSeparatorView(B_HORIZONTAL, B_PLAIN_BORDER))
	;

	// Project View
	fProjectsTabView = new BTabView("ProjectsTabview");
	fProjectsOutline = new BOutlineListView("ProjectsOutline", B_SINGLE_SELECTION_LIST);
	fProjectsScroll = new BScrollView(B_TRANSLATE("Projects"),
		fProjectsOutline, B_FRAME_EVENTS | B_WILL_DRAW, true, true, B_NO_BORDER);
	fProjectsTabView->AddTab(fProjectsScroll);

	// Editor tab & view
	fEditorObjectList = new BObjectList<Editor>();

	fTabManager = new TabManager(BMessenger(this));
	fTabManager->TabGroup()->SetExplicitMaxSize(BSize(B_SIZE_UNSET, 30.0));

	fEditorTabsGroup = BLayoutBuilder::Group<>(B_VERTICAL, 0.0)
		.SetInsets(0, 0, 0, 0)
		.Add(BLayoutBuilder::Group<>(B_VERTICAL, 0.0)
			.Add(fTabManager->TabGroup())
			.Add(fTabManager->ContainerView())
		)
	;

	// Output
	fOutputTabView = new BTabView("OutputTabview");

	fNotificationsListView = new BColumnListView(B_TRANSLATE("Notifications"),
									B_NAVIGABLE, B_PLAIN_BORDER, true);
	fNotificationsListView->AddColumn(new BDateColumn(B_TRANSLATE("Time"),
								140.0, 140.0, 140.0), kTimeColumn);
	fNotificationsListView->AddColumn(new BStringColumn(B_TRANSLATE("Message"),
								400.0, 400.0, 800.0, 0), kMessageColumn);
	fNotificationsListView->AddColumn(new BStringColumn(B_TRANSLATE("Type"),
								140.0, 140.0, 140.0, 0), kTypeColumn);

//	fNotificationText = new BTextView("NotificationText");

	fOutputTabView->AddTab(fNotificationsListView);

	// Layout
	fRootLayout = BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(0, 0, 0, 0)
		.Add(menuBar)
		.Add(toolBar)
			.AddSplit(B_VERTICAL, 0) // output split
				.AddSplit(B_HORIZONTAL, 0) // sidebar split
					.Add(fProjectsTabView, kProjectsWeight)
					.AddGroup(B_VERTICAL, 0, kEditorWeight)  // Editor
						.Add(fEditorTabsGroup)
					.End() // editor group
				.End() // sidebar split
				.Add(fOutputTabView, kOutputWeight)
			.End() //  output split
	;

}

IdeamWindow::~IdeamWindow()
{
	delete fEditorObjectList;
	delete fTabManager;

}

void
IdeamWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_ABOUT_REQUESTED:
			be_app->PostMessage(B_ABOUT_REQUESTED);
			break;
		case B_REFS_RECEIVED:
			_FilesOpen(message);
			break;
		case MSG_FILE_NEW:
		{
			entry_ref ref;
			ref.set_name("New file");

			fEditor = new Editor(&ref, BMessenger(this));

			fTabManager->AddTab(fEditor, ref.name);
			fEditorObjectList->AddItem(fEditor);
			fTabManager->SelectTab(fTabManager->CountTabs() - 1);

			break;
		}
		case MSG_SHOW_HIDE_PROJECTS:
		{
			if (fProjectsTabView->IsHidden()) {
				fProjectsTabView->Show();
			} else {
				fProjectsTabView->Hide();
			}
			break;
		}
		case MSG_SHOW_HIDE_OUTPUT:
		{
			if (fOutputTabView->IsHidden()) {
				fOutputTabView->Show();
			} else {
				fOutputTabView->Hide();
			}
			break;
		}
		case TABMANAGER_TAB_CHANGED:
		{

			break;
		}
		case TABMANAGER_TAB_CLOSE:
		{
			int32 index;
			if (message->FindInt32("index", &index) != B_OK)
				break;

			BView* view = fTabManager->RemoveTab(index);
			Editor* editorView = dynamic_cast<Editor*>(view);
			fEditorObjectList->RemoveItem(fEditorObjectList->ItemAt(index));
			delete editorView;

			break;
		}
		case TABMANAGER_TAB_NEW_OPENED:
		{

			break;
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}

status_t
IdeamWindow::_FilesOpen(BMessage* msg)
{
	return B_OK;
}

BIconButton*
IdeamWindow::_LoadIconButton(const char* name, int32 msg,
								int32 resIndex, bool enabled, const char* tooltip)
{
	BIconButton* button = new BIconButton(name, NULL, new BMessage(msg));
//	button->SetIcon(_LoadSizedVectorIcon(resIndex, kToolBarSize));
	button->SetIcon(resIndex);
	button->SetEnabled(enabled);
//	button->SetToolTip(tooltip);

	return button;
}

BBitmap*
IdeamWindow::_LoadSizedVectorIcon(int32 resourceID, int32 size)
{
	BResources* res = BApplication::AppResources();
	size_t iconSize;
	const void* data = res->LoadResource(B_VECTOR_ICON_TYPE, resourceID, &iconSize);

	assert(data != NULL);

	BBitmap* bitmap = new BBitmap(BRect(0, 0, size, size), B_RGBA32);

	status_t status = BIconUtils::GetVectorIcon(static_cast<const uint8*>(data),
						iconSize, bitmap);

	assert(status == B_OK);

	return bitmap;
}
