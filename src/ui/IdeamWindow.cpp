/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IdeamWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
#include <IconUtils.h>
#include <LayoutBuilder.h>
#include <MenuBar.h>
#include <PopUpMenu.h>
#include <Resources.h>
#include <SeparatorView.h>

#include <cassert>
#include <iostream>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "IdeamWindow"

//#define MULTIFILE_OPEN_SELECT_FIRST_FILE

// If enabled check menu open point
//static const auto kToolBarSize = 29;

static const float kTabBarHeight = 30.0f;

static float kProjectsWeight = 1.0f;
static float kEditorWeight  = 3.0f;
static float kOutputWeight  = 0.4f;

BRect dirtyFrameHack;

enum {
	// File menu
	MSG_FILE_NEW				= 'fnew',
	MSG_FILE_OPEN				= 'fope',
	MSG_FILE_SAVE				= 'fsav',
	MSG_FILE_SAVE_AS			= 'fsas',
	MSG_FILE_SAVE_ALL			= 'fsal',
	MSG_FILE_CLOSE				= 'fclo',
	MSG_FILE_CLOSE_ALL			= 'fcal',

	// Toolbar
	MSG_FILE_MENU_SHOW			= 'fmsh',
	MSG_FILE_NEXT_SELECTED		= 'fnse',
	MSG_FILE_PREVIOUS_SELECTED	= 'fpse',
	MSG_SHOW_HIDE_PROJECTS		= 'shpr',
	MSG_SHOW_HIDE_OUTPUT		= 'shou',
};

IdeamWindow::IdeamWindow(BRect frame)
	:
	BWindow(frame, "Ideam", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS |
												B_QUIT_ON_WINDOW_CLOSE)
{
	// Menu
	BMenuBar* menuBar = new BMenuBar("menubar");
	BLayoutBuilder::Menu<>(menuBar)
		.AddMenu(B_TRANSLATE("Project"))
			.AddItem(B_TRANSLATE("Quit"), B_QUIT_REQUESTED, 'Q')
		.End()
		.AddMenu(B_TRANSLATE("File"))
			.AddItem(B_TRANSLATE("New"), MSG_FILE_NEW).SetEnabled(false)
			.AddItem(B_TRANSLATE("Open"), MSG_FILE_OPEN, 'O')
			.AddSeparator()
			.AddItem(B_TRANSLATE("Save"), MSG_FILE_SAVE, 'S')
			.AddItem(B_TRANSLATE("Save as" B_UTF8_ELLIPSIS), MSG_FILE_SAVE_AS)
			.AddItem(B_TRANSLATE("Save all"), MSG_FILE_SAVE_ALL, 'S', B_SHIFT_KEY)
			.AddSeparator()
			.AddItem(B_TRANSLATE("Close"), MSG_FILE_CLOSE, 'W')
			.AddItem(B_TRANSLATE("Close all"), MSG_FILE_CLOSE_ALL, 'W', B_SHIFT_KEY)
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
	fFileSaveButton = _LoadIconButton("FileSaveButton", MSG_FILE_SAVE,
						206, false, B_TRANSLATE("Save current File"));
	fFileSaveAllButton = _LoadIconButton("FileSaveAllButton", MSG_FILE_SAVE_ALL,
						207, false, B_TRANSLATE("Save all Files"));

	fFilePreviousButton = _LoadIconButton("FilePreviousButton", MSG_FILE_PREVIOUS_SELECTED,
						208, false, B_TRANSLATE("Select previous File"));
	fFileNextButton = _LoadIconButton("FileNextButton", MSG_FILE_NEXT_SELECTED,
						209, false, B_TRANSLATE("Select next File"));
	fFileCloseButton = _LoadIconButton("FileCloseButton", MSG_FILE_CLOSE,
						210, false, B_TRANSLATE("Close File"));
	fFileMenuButton = _LoadIconButton("FileMenuButton", MSG_FILE_MENU_SHOW,
						211, false, B_TRANSLATE("Indexed File list"));
	AddShortcut(B_LEFT_ARROW, B_OPTION_KEY, new BMessage(MSG_FILE_PREVIOUS_SELECTED));
	AddShortcut(B_RIGHT_ARROW, B_OPTION_KEY, new BMessage(MSG_FILE_NEXT_SELECTED));


	BGroupLayout* toolBar = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.Add(BLayoutBuilder::Group<>(B_HORIZONTAL, 1)
			.AddGlue()
			.Add(fProjectsButton)
			.Add(fOutputButton)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.Add(fFileSaveButton)
			.Add(fFileSaveAllButton)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.AddGlue()
			.Add(fFilePreviousButton)
			.Add(fFileNextButton)
			.Add(fFileCloseButton)
			.Add(fFileMenuButton)
			.SetInsets(1, 1, 1, 1)
		)
		.Add(new BSeparatorView(B_HORIZONTAL, B_PLAIN_BORDER))
	;

	// Projects View
	fProjectsTabView = new BTabView("ProjectsTabview");
	fProjectsOutline = new BOutlineListView("ProjectsOutline", B_SINGLE_SELECTION_LIST);
	fProjectsScroll = new BScrollView(B_TRANSLATE("Projects"),
		fProjectsOutline, B_FRAME_EVENTS | B_WILL_DRAW, true, true, B_NO_BORDER);
	fProjectsTabView->AddTab(fProjectsScroll);

	// Editor tab & view
	fEditorObjectList = new BObjectList<Editor>();

	fTabManager = new TabManager(BMessenger(this));
	fTabManager->TabGroup()->SetExplicitMaxSize(BSize(B_SIZE_UNSET, kTabBarHeight));

	dirtyFrameHack = fTabManager->TabGroup()->Frame();

	// Status Bar
	fStatusBar = new BStatusBar("Statusbar");
	fStatusBar->SetBarHeight(1.0);

	fEditorTabsGroup = BLayoutBuilder::Group<>(B_VERTICAL, 0.0)
		.SetInsets(1, 1, 1, 1)
		.Add(BLayoutBuilder::Group<>(B_VERTICAL, 0.0)
			.Add(fTabManager->TabGroup())
			.Add(fTabManager->ContainerView())
			.Add(new BSeparatorView(B_HORIZONTAL))
			.Add(fStatusBar)
		)
	;

	// Panels
	fOpenPanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL, B_FILE_NODE, true);

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

	fOutputTabView->AddTab(fNotificationsListView);

	// Layout
	fRootLayout = BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(0, 0, 0, 0)
		.Add(menuBar)
		.Add(toolBar)
			.AddSplit(B_VERTICAL, 0.0f) // output split
				.AddSplit(B_HORIZONTAL, 0.0f) // sidebar split
					.Add(fProjectsTabView, kProjectsWeight)
//					.AddGroup(B_VERTICAL, 0, kEditorWeight)  // Editor
						.Add(fEditorTabsGroup, kEditorWeight)
//					.End() // editor group
				.End() // sidebar split
				.Add(fOutputTabView, kOutputWeight)
			.End() //  output split
	;

}

IdeamWindow::~IdeamWindow()
{
	delete fEditorObjectList;
	delete fTabManager;
	delete fOpenPanel;
}

void
IdeamWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_ABOUT_REQUESTED:
			be_app->PostMessage(B_ABOUT_REQUESTED);
			break;
		case B_REFS_RECEIVED:
			_FileOpen(message);
			break;
		case B_SAVE_REQUESTED:

			break;
		case EDITOR_SAVEPOINT_REACHED: {
			entry_ref ref;
			if (message->FindRef("ref", &ref) == B_OK) {
				int32 index = _GetEditorIndex(&ref);
				_UpdateLabel(index, false);
				_UpdateSelectionChange(index);
			}
#if defined MULTIFILE_OPEN_SELECT_FIRST_FILE
				/* Have to call _UpdateSelectionChange again because SAVEPOINT_REACHED
				 * comes after file selection. On multifile open first the first
				 * newly open file is selected but savepoint reached is for the
				 * last file and previous/next arrows get wrong.
				 */
				int32 index = fTabManager->SelectedTabIndex();
				fEditor = fEditorObjectList->ItemAt(index);
				_UpdateSelectionChange(index);
#endif
			break;
		}
		case EDITOR_SAVEPOINT_LEFT: {
			entry_ref ref;
			if (message->FindRef("ref", &ref) == B_OK) {
				int32 index = _GetEditorIndex(&ref);
				_UpdateLabel(index, true);
				_UpdateSelectionChange(index);
			}

			break;
		}
		case MSG_FILE_CLOSE:
			_FileClose(fTabManager->SelectedTabIndex());
			break;
		case MSG_FILE_CLOSE_ALL:
			_FileCloseAll();
			break;
		case MSG_FILE_MENU_SHOW:
		{
			/* Adapted from tabview */
				BPopUpMenu* tabMenu = new BPopUpMenu("filetabmenu", true, false);
				int tabCount = fTabManager->CountTabs();
				for (int index = 0; index < tabCount; index++) {
						BString label;
						label << index + 1 << ". " << fTabManager->TabLabel(index);
						BMenuItem* item = new BMenuItem(label.String(), NULL);
						tabMenu->AddItem(item);
						if (index == fTabManager->SelectedTabIndex())
							item->SetMarked(true);
				}

				// Force layout to get the final menu size. InvalidateLayout()
				// did not seem to work here.
				tabMenu->AttachedToWindow();
				BRect buttonFrame = fFileMenuButton->Frame();
				BRect menuFrame = tabMenu->Frame();
				BPoint openPoint = ConvertToScreen(buttonFrame.LeftBottom());
				// Open with the right side of the menu aligned with the right
				// side of the button and a little below.
				openPoint.x -= menuFrame.Width() - buttonFrame.Width() + 2;
				openPoint.y += 20;

				BMenuItem *selected = tabMenu->Go(openPoint, false, false,
					ConvertToScreen(buttonFrame));
				if (selected) {
					selected->SetMarked(true);
					int32 index = tabMenu->IndexOf(selected);
					if (index != B_ERROR)
						fTabManager->SelectTab(index);
				}
				delete tabMenu;
			break;
		}
		case MSG_FILE_NEW:
		{
			//TODO
			break;
		}
		case MSG_FILE_NEXT_SELECTED:
		{
			int32 index = fTabManager->SelectedTabIndex();
			if (index < fTabManager->CountTabs() - 1)
				fTabManager->SelectTab(index + 1);
			break;
		}	
		case MSG_FILE_OPEN:
			fOpenPanel->Show();
			break;
		case MSG_FILE_PREVIOUS_SELECTED:
		{
			int32 index = fTabManager->SelectedTabIndex();
			if (index > 0)
				fTabManager->SelectTab(index - 1);
			break;
		}	
		case MSG_FILE_SAVE:
		{
			_FileSave(fTabManager->SelectedTabIndex());
			break;
		}
		case MSG_FILE_SAVE_AS:
		{
			//TODO
			break;
		}
		case MSG_FILE_SAVE_ALL:
		{
			_FileSaveAll();
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
			int32 index;
			if (message->FindInt32("index", &index) == B_OK) {
					fEditor = fEditorObjectList->ItemAt(index);
					fEditor->GrabFocus();
std::cerr << "TABMANAGER_TAB_CHANGED" << " index: " << index << std::endl;
				_UpdateSelectionChange(index);
			}
			break;
		}
		case TABMANAGER_TAB_CLOSE:
		{
			int32 index;
			if (message->FindInt32("index", &index) == B_OK)
				_FileClose(index);

			break;
		}
		case TABMANAGER_TAB_NEW_OPENED:
		{
			int32 index;
			if (message->FindInt32("index", &index) == B_OK) {
std::cerr << "TABMANAGER_TAB_NEW_OPENED" << " index: " << index << std::endl;

			}
			break;
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}

status_t
IdeamWindow::_AddEditorTab(entry_ref* ref, int32 index)
{
	// Check existence
	BEntry entry(ref);

	if (entry.Exists() == false)
		return B_ERROR;

	fEditor = new Editor(ref, BMessenger(this));

	if (fEditor == nullptr)
		return B_ERROR;

	fTabManager->AddTab(fEditor, ref->name, index);

	bool added = fEditorObjectList->AddItem(fEditor);

	assert(added == true);

	return B_OK;
}

status_t
IdeamWindow::_FileClose(int32 index)
{
	BString notification;

	if (index < 0) {
		notification << (B_TRANSLATE("No file selected"));
		_SendNotification(notification.String(), "FILE_INFO");
		return B_ERROR;
	}
#ifdef DEBUG
BView* myview = dynamic_cast<BView*>(fTabManager->ViewForTab(index));
notification << "Child name is: " << myview->ChildAt(0)->Name();
//notification << "NextSibling name is: " << myview->NextSibling()->Name();
notification << " View name is: " << myview->Name();
_SendNotification(notification.String(), "FILE_ERR");
notification.SetTo("");
#endif
	fEditor = fEditorObjectList->ItemAt(index);

	if (fEditor == nullptr) {
		notification << (B_TRANSLATE("NULL editor pointer"));
		_SendNotification(notification.String(), "FILE_ERR");
		return B_ERROR;
	}

	if (fEditor->IsModified()) {
		BString text(B_TRANSLATE("Save changes to file \"%file%\""));
		text.ReplaceAll("%file%", fEditor->Name());
		
		BAlert* alert = new BAlert(B_TRANSLATE("Close and save dialog"), text,
 			B_TRANSLATE("Cancel"), B_TRANSLATE("Don't save"), B_TRANSLATE("Save"),
 			B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
   			 
		alert->SetShortcut(0, B_ESCAPE);
		
		int32 choice = alert->Go();

		if (choice == 0)
			return B_ERROR;
		else if (choice == 2) {
			_FileSave(index);
		}
	}

	notification << fEditor->Name() << " " << B_TRANSLATE("closed");
	_SendNotification(notification.String(), "FILE_CLOSE");

	BView* view = fTabManager->RemoveTab(index);
	Editor* editorView = dynamic_cast<Editor*>(view);
	fEditorObjectList->RemoveItem(fEditorObjectList->ItemAt(index));
	delete editorView;

	// Was it the last one?
	if (fTabManager->CountTabs() == 0)
		_UpdateSelectionChange(-1);

	return B_OK;
}

void
IdeamWindow::_FileCloseAll()
{
	int32 tabsCount = fTabManager->CountTabs();
	// If there is something to close
	if (tabsCount > 0) {
		// Don't lose time in changing selection on removal
		fTabManager->SelectTab(0);

		for (int32 index = tabsCount - 1; index >= 0; index--) {
			fTabManager->CloseTab(index);
		}
	}
}

status_t
IdeamWindow::_FileOpen(BMessage* msg)
{
	entry_ref ref;
	status_t status = B_OK;
	int32 refsCount = 0;
	int32 openedIndex;
	int32 nextIndex = fTabManager->CountTabs();
	BString notification;

	while (msg->FindRef("refs", refsCount, &ref) == B_OK) {

		refsCount++;

		// Do not reopen an already opened file
		if ((openedIndex = _GetEditorIndex(&ref)) != -1) {
			if (openedIndex != fTabManager->SelectedTabIndex())
				fTabManager->SelectTab(openedIndex);
			continue;
		}

		int32 index = fTabManager->CountTabs();
std::cerr << __PRETTY_FUNCTION__ << " index: " << index << std::endl;

		if (_AddEditorTab(&ref, index) != B_OK)
			continue;

		assert(index >= 0);

		fEditor = fEditorObjectList->ItemAt(index);

		if (fEditor == nullptr) {
			notification << ref.name << ": "
						 << (B_TRANSLATE("NULL editor pointer"));
			_SendNotification(notification.String(), "FILE_ERR");
			return B_ERROR;
		}

		status = fEditor->LoadFromFile();

		if (status != B_OK) {
			//TODO
			// _FileClose(index, false);
			// strerror(status)
			continue;
		}
		// First tab gets selected by tabview
		if (index > 0)
			fTabManager->SelectTab(index);

		notification << fEditor->Name() << " " << B_TRANSLATE("opened with index")
			<< " " << fTabManager->CountTabs() - 1;
		_SendNotification(notification.String(), "FILE_OPEN");
		notification.SetTo("");
	}

#if defined MULTIFILE_OPEN_SELECT_FIRST_FILE
	// Needs modified libscintilla
	// If at least 1 item or more were added select the first
	// of them. see below
	if (nextIndex < fTabManager->CountTabs()) {
		fTabManager->SelectTab(nextIndex);
}
#else
	// If at least one item added, select last opened file:
	// it grabs keyboard focus anyway so fix that if you want to change
	//  selection management on multi-open.
	int32 tabs = fTabManager->CountTabs();
	if (nextIndex < tabs)
		fTabManager->SelectTab(tabs - 1);
#endif

	return status;
}

status_t
IdeamWindow::_FileSave(int32 index)
{
//	status_t status;
	BString notification;

	if (index < 0) {
		notification << (B_TRANSLATE("No file selected"));
		_SendNotification(notification.String(), "FILE_INFO");
		return B_ERROR;
	}

	fEditor = fEditorObjectList->ItemAt(index);

	if (fEditor == nullptr) {
		notification << (B_TRANSLATE("NULL editor pointer"));
		_SendNotification(notification.String(), "FILE_ERR");
		return B_ERROR;
	}

	// If readonly file warn and exit
	if (fEditor->IsReadOnly()) {
		notification << (B_TRANSLATE("File is Read-only"));
		_SendNotification(notification.String(), "FILE_INFO");
		return B_ERROR;
	}

	// If file not modified warn and exit
	if (!fEditor->IsModified()) {
		notification << (B_TRANSLATE("File not modified"));
		_SendNotification(notification.String(), "FILE_INFO");
		return B_ERROR;
	}

	ssize_t written = fEditor->SaveToFile();
	ssize_t length = fEditor->SendMessage(SCI_GETLENGTH, 0, 0);

	notification << fEditor->Name()<< B_TRANSLATE(" saved.")
		<< "\t\t" << B_TRANSLATE("length: ") << length << B_TRANSLATE(" bytes -> ")
		<< written<< B_TRANSLATE(" bytes written");

	_SendNotification(notification.String(), length == written ? "FILE_SAVE" : "FILE_ERR");

	return B_OK;
}

void
IdeamWindow::_FileSaveAll()
{
	int32 filesCount = fEditorObjectList->CountItems();

	for (int32 index = 0; index < filesCount; index++) {

		fEditor = fEditorObjectList->ItemAt(index);

		if (fEditor == nullptr) {
			BString notification;
			notification << B_TRANSLATE("Index ") << index
				<< (B_TRANSLATE(": NULL editor pointer"));
			_SendNotification(notification.String(), "FILE_ERR");
			continue;
		}

		if (fEditor->IsModified())
			_FileSave(index);
	}
}

bool
IdeamWindow::_FilesNeedSave()
{
	for (int32 index = 0; index < fEditorObjectList->CountItems(); index++) {
		fEditor = fEditorObjectList->ItemAt(index);
		if (fEditor->IsModified()) {
			return true;
		}
	}

	return false;
}

int32
IdeamWindow::_GetEditorIndex(entry_ref* ref)
{
	BEntry entry(ref, true);
	int32 filesCount = fEditorObjectList->CountItems();
	
	for (int32 index = 0; index < filesCount; index++) {

		fEditor = fEditorObjectList->ItemAt(index);

		if (fEditor == nullptr) {
			BString notification;
			notification << B_TRANSLATE("Index ") << index
				<< (B_TRANSLATE(": NULL editor pointer"));
			_SendNotification(notification.String(), "FILE_ERR");
			continue;
		}

		BEntry matchEntry(fEditor->FileRef(), true);

		if (matchEntry == entry)
			return index;
	}
	return -1;
}

BIconButton*
IdeamWindow::_LoadIconButton(const char* name, int32 msg,
								int32 resIndex, bool enabled, const char* tooltip)
{
	BIconButton* button = new BIconButton(name, NULL, new BMessage(msg));
//	button->SetIcon(_LoadSizedVectorIcon(resIndex, kToolBarSize));
	button->SetIcon(resIndex);
	button->SetEnabled(enabled);
	button->SetToolTip(tooltip);

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

void
IdeamWindow::_SendNotification(const char* message, const char* type)
{
       BRow* fRow = new BRow();
       time_t now =  static_cast<bigtime_t>(real_time_clock());

       fRow->SetField(new BDateField(&now), kTimeColumn);
       fRow->SetField(new BStringField(message), kMessageColumn);
       fRow->SetField(new BStringField(type), kTypeColumn);
       fNotificationsListView->AddRow(fRow, 0);
}

status_t
IdeamWindow::_UpdateLabel(int32 index, bool isModified)
{
	if (index > -1) {
		if (isModified == true) {
				// Add '*' to file name
				BString label(fTabManager->TabLabel(index));
				label.Append("*");
				fTabManager->SetTabLabel(index, label.String());
		} else {
				// Remove '*' from file name
				BString label(fTabManager->TabLabel(index));
				label.RemoveLast("*");
				fTabManager->SetTabLabel(index, label.String());
		}
		return B_OK;
	}
	
	return B_ERROR;
}

void
IdeamWindow::_UpdateSelectionChange(int32 index)
{
BString text;
text << "index: " << index << " sti: " << fTabManager->SelectedTabIndex();
fStatusBar->SetTrailingText(text.String());

	if (index < -1)
		return;

	// All files are closed
	if (index == -1) {
		fFileSaveButton->SetEnabled(false);
		fFileSaveAllButton->SetEnabled(false);
		fFilePreviousButton->SetEnabled(false);
		fFileNextButton->SetEnabled(false);
		fFileCloseButton->SetEnabled(false);
		fFileMenuButton->SetEnabled(false);
		return;
	}

/*	// fEditor should be already set before the call
	fEditor = fEditorObjectList->ItemAt(index);
	// This could be checked too
	if (fTabManager->SelectedTabIndex() != index);
*/
	fFileSaveButton->SetEnabled(fEditor->IsModified());
	fFileSaveAllButton->SetEnabled(_FilesNeedSave());
	fFileCloseButton->SetEnabled(true);
	fFileMenuButton->SetEnabled(true);

	int32 maxTabIndex = (fTabManager->CountTabs() - 1);

	if (index == 0) {
		fFilePreviousButton->SetEnabled(false);
		if (maxTabIndex > 0)
				fFileNextButton->SetEnabled(true);
	} else if (index == maxTabIndex) {
			fFileNextButton->SetEnabled(false);
			fFilePreviousButton->SetEnabled(true);
	} else {
			fFilePreviousButton->SetEnabled(true);
			fFileNextButton->SetEnabled(true);
	}
}
