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
#include <NodeMonitor.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <RecentItems.h>
#include <Resources.h>
#include <SeparatorView.h>

#include <cassert>
#include <iostream>
#include <sstream>

#include "IdeamNamespace.h"
#include "TPreferences.h"
#include "SettingsWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "IdeamWindow"

#define MULTIFILE_OPEN_SELECT_FIRST_FILE


constexpr auto kRecentFilesNumber = 14 + 1;

// If enabled check menu open point
//static const auto kToolBarSize = 29;

static constexpr float kTabBarHeight = 30.0f;

static constexpr auto kGotolineMaxBytes = 6;

// Find group
static constexpr auto kFindReplaceMaxBytes = 50;
static constexpr auto kFindReplaceMinBytes = 32;
static constexpr float kFindReplaceOPSize = 120.0;
static constexpr auto kFindReplaceMenuItems = 10;


static float kEditorWeight  = 3.0f;
static float kOutputWeight  = 0.4f;

BRect dirtyFrameHack;

enum {
	// File menu
	MSG_FILE_NEW				= 'fine',
	MSG_FILE_OPEN				= 'fiop',
	MSG_FILE_SAVE				= 'fisa',
	MSG_FILE_SAVE_AS			= 'fsas',
	MSG_FILE_SAVE_ALL			= 'fsal',
	MSG_FILE_CLOSE				= 'ficl',
	MSG_FILE_CLOSE_ALL			= 'fcal',
	MSG_FILE_FOLD_TOGGLE		= 'fifo',

	// Edit menu
	MSG_TEXT_DELETE				= 'tede',
	MSG_TEXT_OVERWRITE			= 'teov',
	MSG_WHITE_SPACES_TOGGLE		= 'whsp',
	MSG_LINE_ENDINGS_TOGGLE		= 'lien',

	// Search menu & group
	MSG_FIND_GROUP_SHOW			= 'figs',
	MSG_FIND_MENU_SELECTED		= 'fmse',
	MSG_FIND_PREVIOUS			= 'fipr',
	MSG_FIND_MARK_ALL			= 'fmal',
	MSG_FIND_NEXT				= 'fite',
	MSG_REPLACE_GROUP_SHOW		= 'regs',
	MSG_REPLACE_MENU_SELECTED 	= 'rmse',
	MSG_REPLACE_ONE				= 'reon',
	MSG_REPLACE_NEXT			= 'rene',
	MSG_REPLACE_PREVIOUS		= 'repr',
	MSG_REPLACE_ALL				= 'real',
	MSG_GOTO_LINE				= 'goli',
	MSG_BOOKMARK_CLEAR_ALL		= 'bcal',
	MSG_BOOKMARK_GOTO_NEXT		= 'bgne',
	MSG_BOOKMARK_GOTO_PREVIOUS	= 'bgpr',
	MSG_BOOKMARK_TOGGLE			= 'book',

	// Window menu
	MSG_WINDOW_SETTINGS			= 'wise',
	MSG_TOGGLE_TOOLBAR			= 'toto',

	// Toolbar
	MSG_BUFFER_LOCK				= 'bulo',
	MSG_FILE_MENU_SHOW			= 'fmsh',
	MSG_FILE_NEXT_SELECTED		= 'fnse',
	MSG_FILE_PREVIOUS_SELECTED	= 'fpse',
	MSG_FIND_GROUP_TOGGLED		= 'figt',
	MSG_FIND_IN_FILES			= 'fifi',
	MSG_LINE_TO_GOTO			= 'ltgt',
	MSG_REPLACE_GROUP_TOGGLED	= 'regt',
	MSG_SHOW_HIDE_PROJECTS		= 'shpr',
	MSG_SHOW_HIDE_OUTPUT		= 'shou',

	MSG_SELECT_TAB				= 'seta'
};

IdeamWindow::IdeamWindow(BRect frame)
	:
	BWindow(frame, "Ideam", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS |
												B_QUIT_ON_WINDOW_CLOSE)
{
	_InitMenu();

	_InitWindow();

	// Fill Settings vars before using
	IdeamNames::LoadSettingsVars();

	// Layout
	fRootLayout = BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(0.0f, 0.0f, 0.0f, 0.0f)
		.Add(fMenuBar)
		.Add(fToolBar)

			.AddSplit(B_VERTICAL, 0.0f) // output split
				.AddSplit(B_HORIZONTAL, 0.0f) // sidebar split
					.Add(fProjectsTabView)
					.AddGroup(B_VERTICAL, 0, kEditorWeight)  // Editor
						.SetInsets(2.0f, 2.0f, 0.0f, 2.0f)
						.Add(fEditorTabsGroup)
					.End() // editor group
				.End() // sidebar split
				.Add(fOutputTabView, kOutputWeight)
			.End() //  output split
	;

	// Shortcuts
	for (int32 index = 1; index < 10; index++) {
		constexpr auto kAsciiPos {48};
		BMessage* selectTab = new BMessage(MSG_SELECT_TAB);
		selectTab->AddInt32("index", index - 1);
		AddShortcut(index + kAsciiPos, B_COMMAND_KEY, selectTab);
	}
	AddShortcut(B_LEFT_ARROW, B_OPTION_KEY, new BMessage(MSG_FILE_PREVIOUS_SELECTED));
	AddShortcut(B_RIGHT_ARROW, B_OPTION_KEY, new BMessage(MSG_FILE_NEXT_SELECTED));

	// Interface elements
	if (IdeamNames::Settings.show_projects == false)
		fProjectsTabView->Hide();

	if (IdeamNames::Settings.show_output == false)
		fOutputTabView->Hide();

	if (IdeamNames::Settings.show_toolbar == false)
		fToolBar->View()->Hide();

	// Reopen files
	if (IdeamNames::Settings.reopen_files == true) {
		TPreferences* files = new TPreferences(IdeamNames::kSettingsFilesToReopen,
												IdeamNames::kApplicationName, 'FRSE');
		if (!files->IsEmpty()) {
			entry_ref ref;
			int32 index = -1, count;
			BMessage *message = new BMessage(B_REFS_RECEIVED);

			if (files->FindInt32("opened_index", &index) == B_OK) {
				message->AddInt32("opened_index", index);

				for (count = 0; files->FindRef("file_to_reopen", count, &ref) == B_OK; count++)
					message->AddRef("refs", &ref);
				// Found an index and found some files, post message
				if (index > -1 && count > 0)
					PostMessage(message);
			}
		}
		delete files;
	}
}

IdeamWindow::~IdeamWindow()
{
	delete fEditorObjectList;
	delete fTabManager;

	delete fOpenPanel;
	delete fSavePanel;
}

void
IdeamWindow::DispatchMessage(BMessage* message, BHandler* handler)
{
	if (handler == fFindTextControl->TextView()) {
		if (message->what == B_KEY_DOWN) {
			int8 key;
			if (message->FindInt8("byte", 0, &key) == B_OK) {
				if (key == B_ESCAPE) {
					// If keep focus activated TODO
					#if 0
					int32 index = fTabManager->SelectedTabIndex();
					if (index > -1 && index < fTabManager->CountTabs()) {
						fEditor = fEditorObjectList->ItemAt(index);
						fEditor->GrabFocus();
					}
					#else
					{
						fFindGroup->SetVisible(false);
						fReplaceGroup->SetVisible(false);
					}
					#endif
				}
			}
		}
	} else if (handler == fReplaceTextControl->TextView()) {
		if (message->what == B_KEY_DOWN) {
			int8 key;
			if (message->FindInt8("byte", 0, &key) == B_OK) {
				if (key == B_ESCAPE) {
					fReplaceGroup->SetVisible(false);
					fFindTextControl->MakeFocus(true);
				}
			}
		}
	} else if (handler == fGotoLine->TextView()) {
		if (message->what == B_KEY_DOWN) {
			int8 key;
			if (message->FindInt8("byte", 0, &key) == B_OK) {
				if (key == B_ESCAPE) {
					fGotoLine->Hide();
					int32 index = fTabManager->SelectedTabIndex();
					if (index > -1 && index < fTabManager->CountTabs()) {
						fEditor = fEditorObjectList->ItemAt(index);
						fEditor->GrabFocus();
					}
				}
			}
		}
	}

	BWindow::DispatchMessage(message, handler);
}

void
IdeamWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_ABOUT_REQUESTED:
			be_app->PostMessage(B_ABOUT_REQUESTED);
			break;
		case B_COPY: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->Copy();
			}
			break;
		}
		case B_CUT: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->Cut();
			}
			break;
		}
		case B_NODE_MONITOR:
			_HandleNodeMonitorMsg(message);
			break;
		case B_PASTE: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->Paste();
			}
			break;
		}
		case B_REDO: {
			int32 index =  fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				if (fEditor->CanRedo())
					fEditor->Redo();
				_UpdateSelectionChange(index);
			}
			break;
		}
		case B_REFS_RECEIVED:
			_FileOpen(message);
			Activate();
			break;
		case B_SAVE_REQUESTED:
			_FileSaveAs(fTabManager->SelectedTabIndex(), message);
			break;
		case B_SELECT_ALL: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->SelectAll();
			}
			break;
		}
		case B_UNDO: {
			int32 index =  fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				if (fEditor->CanUndo())
					fEditor->Undo();
				_UpdateSelectionChange(index);
			}
			break;
		}
		case EDITOR_BOOKMARK_MARK: {
			entry_ref ref;
			if (message->FindRef("ref", &ref) == B_OK) {
				int32 index = _GetEditorIndex(&ref);
				if (index == fTabManager->SelectedTabIndex()) {
					fEditor = fEditorObjectList->ItemAt(index);

					int line;
					if (message->FindInt32("line", &line) == B_OK) {
						BString text;
						text << fEditor->Name() << " :" << line;
						_SendNotification( text.String(), "MARK_FOUND");
					}
				}
			}

			break;
		}
		case EDITOR_REPLACED_ONE: {
			entry_ref ref;
			if (message->FindRef("ref", &ref) == B_OK) {
				int32 index =  _GetEditorIndex(&ref);
				if (index == fTabManager->SelectedTabIndex()) {
					int line, column;
					BString selection, replacement;
					if (message->FindInt32("line", &line) == B_OK
						&& message->FindInt32("column", &column) == B_OK
						&& message->FindString("selection", &selection) == B_OK
						&& message->FindString("replacement", &replacement) == B_OK) {
						BString text;
						text << fEditor->Name() << " " << line  << ":" << column
							 << " -- " << selection << " -> " << replacement;
						_SendNotification( text.String(), "REPLACE_DONE");
					}
				}
			}
			break;
		}
		case EDITOR_SELECTION_CHANGED: {
			entry_ref ref;
			if (message->FindRef("ref", &ref) == B_OK) {
				int32 index =  _GetEditorIndex(&ref);
				if (index == fTabManager->SelectedTabIndex()) {
					int line, column;
					_UpdateSelectionChange(index);
					if (message->FindInt32("line", &line) == B_OK
						&& message->FindInt32("column", &column) == B_OK)
						// Do not leave garbage on last file closed
						if (fTabManager->CountTabs() > 0)
							_UpdateStatusBarText(line, column);
				}
			}
			break;
		}
		case EDITOR_SAVEPOINT_LEFT: {
			entry_ref ref;
			if (message->FindRef("ref", &ref) == B_OK) {
				int32 index = _GetEditorIndex(&ref);
				_UpdateLabel(index, true);
std::cerr << "EDITOR_SAVEPOINT_LEFT " << "index: " << index << std::endl;
				_UpdateSelectionChange(index);
			}

			break;
		}
		case EDITOR_SAVEPOINT_REACHED: {
			entry_ref ref;
			if (message->FindRef("ref", &ref) == B_OK) {
				int32 index = _GetEditorIndex(&ref);
				_UpdateLabel(index, false);
std::cerr << "EDITOR_SAVEPOINT_REACHED " << "index: " << index << std::endl;
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
std::cerr << "SELECT_FIRST_FILE " << "index: " << index << std::endl;
				_UpdateSelectionChange(index);
#endif
			break;
		}
		case MSG_BOOKMARK_CLEAR_ALL: {
			int32 index =  fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->BookmarkClearAll(sci_BOOKMARK);
			}
			break;
		}
		case MSG_BOOKMARK_GOTO_NEXT: {
			int32 index =  fTabManager->SelectedTabIndex();
			bool found;
			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				found = fEditor->BookmarkGoToNext();

				if (found == false)
					_SendNotification(B_TRANSLATE("Next Bookmark not found"),
													"BOOKMARK");
			}

			break;
		}
		case MSG_BOOKMARK_GOTO_PREVIOUS: {
			int32 index =  fTabManager->SelectedTabIndex();
			bool found;
			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				found = fEditor->BookmarkGoToPrevious();

				if (found == false)
					_SendNotification(B_TRANSLATE("Previous Bookmark not found"),
													"BOOKMARK");
			}


			break;
		}
		case MSG_BOOKMARK_TOGGLE: {
			int32 index =  fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				// TODO position in callee?
				fEditor->BookmarkToggle(fEditor->GetCurrentPosition());
			}
			break;
		}
		case MSG_BUFFER_LOCK: {
			int32 index =  fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->SetReadOnly();
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
		case MSG_FILE_FOLD_TOGGLE: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->ToggleFolding();
			}
			break;
		}
		case MSG_FILE_MENU_SHOW: {
			/* Adapted from tabview */
				BPopUpMenu* tabMenu = new BPopUpMenu("filetabmenu", true, false);
				int tabCount = fTabManager->CountTabs();
				for (int index = 0; index < tabCount; index++) {
						BString label;
						label << index + 1 << ". " << fTabManager->TabLabel(index);
						BMenuItem* item = new BMenuItem(label.String(), nullptr);
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
		case MSG_FILE_NEW: {
			//TODO
			break;
		}
		case MSG_FILE_NEXT_SELECTED: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index < fTabManager->CountTabs() - 1)
				fTabManager->SelectTab(index + 1);
			break;
		}	
		case MSG_FILE_OPEN:
			fOpenPanel->Show();
			break;
		case MSG_FILE_PREVIOUS_SELECTED: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index > 0 && index < fTabManager->CountTabs())
				fTabManager->SelectTab(index - 1);
			break;
		}	
		case MSG_FILE_SAVE:
			_FileSave(fTabManager->SelectedTabIndex());
			break;
		case MSG_FILE_SAVE_AS: {
			// fEditor should be already set
			// fEditor = fEditorObjectList->ItemAt(fTabManager->SelectedTabIndex());
			BEntry entry(fEditor->FileRef());
			entry.GetParent(&entry);
			fSavePanel->SetPanelDirectory(&entry);
			fSavePanel->Show();
			break;
		}
		case MSG_FILE_SAVE_ALL:
			_FileSaveAll();
			break;
		case MSG_FIND_GROUP_SHOW:
			_FindGroupShow();
			break;
		case MSG_FIND_MARK_ALL: {
			BString textToFind(fFindTextControl->Text());

			if (!textToFind.IsEmpty()) {
				int32 counts = _FindMarkAll(textToFind);
				BString text;
				text << "\"" << textToFind << "\""
					<< B_TRANSLATE(" occurrences found: ") << counts;
				_SendNotification( text.String(), "BOOKMARKS");
			}
			break;
		}
		case MSG_FIND_MENU_SELECTED: {
			int32 index;
			if (message->FindInt32("index", &index) == B_OK) {
				BMenuItem* item = fFindMenuField->Menu()->ItemAt(index);
				fFindTextControl->SetText(item->Label());
			}
			break;
		}
		case MSG_FIND_NEXT: {
			const BString& text(fFindTextControl->Text());
//			if (!text.IsEmpty())
			_FindNext(text, false);
			break;
		}
		case MSG_FIND_PREVIOUS: {
			const BString& text(fFindTextControl->Text());
//			if (!text.IsEmpty())
			_FindNext(text, true);
			break;
		}
		case MSG_FIND_GROUP_TOGGLED:
			_FindGroupToggled();
			break;
		case MSG_GOTO_LINE:
			fGotoLine->Show();
			fGotoLine->MakeFocus();
			break;
		case MSG_REPLACE_GROUP_SHOW:
			_ReplaceGroupShow();
			break;
		case MSG_REPLACE_ALL: {
			int32 counts = _Replace(REPLACE_ALL);
			BString text;
			text << B_TRANSLATE(" replacements done: ") << counts;
			_SendNotification( text.String(), "REPLACEMENTS");
			break;
		}
		case MSG_REPLACE_GROUP_TOGGLED:
			_ReplaceGroupToggled();
			break;
		case MSG_REPLACE_MENU_SELECTED: {
			int32 index;
			if (message->FindInt32("index", &index) == B_OK) {
				BMenuItem* item = fReplaceMenuField->Menu()->ItemAt(index);
				fReplaceTextControl->SetText(item->Label());
			}
			break;
		}
		case MSG_REPLACE_NEXT:
			_Replace(REPLACE_NEXT);
			break;
		case MSG_REPLACE_ONE:
			_Replace(REPLACE_ONE);
			break;
		case MSG_REPLACE_PREVIOUS:
			_Replace(REPLACE_PREVIOUS);
			break;
		case MSG_SELECT_TAB: {
			int32 index;
			// Shortcut selection, be careful
			if (message->FindInt32("index", &index) == B_OK) {
				if (index < fTabManager->CountTabs() 
					&& index != fTabManager->SelectedTabIndex())
					fTabManager->SelectTab(index);
			}
			break;
		}	
		case MSG_SHOW_HIDE_PROJECTS: {
			if (fProjectsTabView->IsHidden()) {
				fProjectsTabView->Show();
			} else {
				fProjectsTabView->Hide();
			}
			break;
		}
		case MSG_SHOW_HIDE_OUTPUT: {
			if (fOutputTabView->IsHidden()) {
				fOutputTabView->Show();
			} else {
				fOutputTabView->Hide();
			}
			break;
		}
		case MSG_TEXT_DELETE: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->Clear();
			}
			break;
		}
		case MSG_TEXT_OVERWRITE: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->OverwriteToggle();
				_UpdateStatusBarTrailing(index);
			}
			break;
		}
		case MSG_LINE_ENDINGS_TOGGLE: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->ToggleLineEndings();
			}
			break;
		}
		case MSG_LINE_TO_GOTO: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index < 0 || index >= fTabManager->CountTabs())
				break;

			std::string linestr(fGotoLine->Text());
			int32 line;
			std::istringstream (linestr) >>  line;

			fEditor = fEditorObjectList->ItemAt(index);

			if (line <= fEditor->CountLines())
				fEditor->GoToLine(line);

			fEditor->GrabFocus();
			fGotoLine->SetText("");
			fGotoLine->Hide();

			break;
		}
		case MSG_TOGGLE_TOOLBAR: {
			if (fToolBar->View()->IsHidden()) {
				fToolBar->View()->Show();
			} else {
				fToolBar->View()->Hide();
			}
			break;
		}
		case MSG_WHITE_SPACES_TOGGLE: {
			int32 index = fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->ToggleWhiteSpaces();
			}
			break;
		}
		case MSG_WINDOW_SETTINGS: {
			SettingsWindow *window = new SettingsWindow();
			window->Show();

			break;
		}
		case TABMANAGER_TAB_CHANGED: {
			int32 index;
			if (message->FindInt32("index", &index) == B_OK) {
					fEditor = fEditorObjectList->ItemAt(index);
					// TODO notify and check index too
/*					if (fEditor == nullptr) {
std::cerr << "TABMANAGER_TAB_CHANGED " << "NULL on index: " << index << std::endl;
						break;
					}	
*/					fEditor->GrabFocus();
					// In multifile open not-focused files place scroll just after
					// caret line when reselected. Ensure visibility.
					// TODO caret policy
					fEditor->EnsureVisiblePolicy();

std::cerr << "TABMANAGER_TAB_CHANGED " << fEditor->Name() << " index: " << index << std::endl;
				fEditor->SendCurrentPosition();
				_UpdateSelectionChange(index);
				_UpdateStatusBarTrailing(index);
			}
			break;
		}
		case TABMANAGER_TAB_CLOSE: {
			int32 index;
			if (message->FindInt32("index", &index) == B_OK)
				_FileClose(index);

			break;
		}
		case TABMANAGER_TAB_NEW_OPENED: {
			int32 index;
			if (message->FindInt32("index", &index) == B_OK) {
std::cerr << "TABMANAGER_TAB_NEW_OPENED" << " index: " << index << std::endl;
				fEditor = fEditorObjectList->ItemAt(index);
				fEditor->SetSavedCaretPosition();
			}
			break;
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool
IdeamWindow::QuitRequested()
{
	// Is there any modified file?
	if (_FilesNeedSave()) {
		BAlert* alert = new BAlert("QuitAndSaveDialog",
	 		B_TRANSLATE("There are modified files, do you want to save changes before quitting?"),
 			B_TRANSLATE("Cancel"), B_TRANSLATE("Don't save"), B_TRANSLATE("Save"),
 			B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
  
		alert->SetShortcut(0, B_ESCAPE);
		
		int32 choice = alert->Go();

		if (choice == 0)
			return false;
		else if (choice == 1) { 

		} else if (choice == 2) {
			_FileSaveAll();
		}
	}

	// Files to reopen
	if (IdeamNames::Settings.reopen_files == true) {
		TPreferences* files = new TPreferences(IdeamNames::kSettingsFilesToReopen,
												IdeamNames::kApplicationName, 'FRSE');
		// Just empty it for now TODO check if equal
		files->MakeEmpty();
			// Save if there is an opened file
			int32 index = fTabManager->SelectedTabIndex();

			if (index > -1 && index < fTabManager->CountTabs()) {
				files->AddInt32("opened_index", index);

				for (int32 index = 0; index < fTabManager->CountTabs(); index++) {
					fEditor = fEditorObjectList->ItemAt(index);
					files->AddRef("file_to_reopen", fEditor->FileRef());
				}
			}
		delete files;
	}

	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
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

/*
 * ignoreModifications: the file is modified but has been removed
 * 						externally and user choose to discard it.
 */
status_t
IdeamWindow::_FileClose(int32 index, bool ignoreModifications /* = false */)
{
	BString notification;

	// Should not happen
	if (index < 0) {
		notification << (B_TRANSLATE("No file selected"));
		_SendNotification(notification, "FILE_ERR");
		return B_ERROR;
	}
#ifdef DEBUG
BView* myview = dynamic_cast<BView*>(fTabManager->ViewForTab(index));
notification << "Child name is: " << myview->ChildAt(0)->Name();
//notification << "NextSibling name is: " << myview->NextSibling()->Name();
notification << " View name is: " << myview->Name();
_SendNotification(notification, "FILE_ERR");
notification.SetTo("");
#endif
	fEditor = fEditorObjectList->ItemAt(index);

	if (fEditor == nullptr) {
		notification << (B_TRANSLATE("NULL editor pointer"));
		_SendNotification(notification, "FILE_ERR");
		return B_ERROR;
	}

	if (fEditor->IsModified() && ignoreModifications == false) {
		BString text(B_TRANSLATE("Save changes to file \"%file%\""));
		text.ReplaceAll("%file%", fEditor->Name());
		
		BAlert* alert = new BAlert("CloseAndSaveDialog", text,
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
	_SendNotification(notification, "FILE_CLOSE");

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
	int32 nextIndex;
	BString notification;

	// If user choose to reopen files reopen right index
	// otherwise use default behaviour (see below)
	if (msg->FindInt32("opened_index", &nextIndex) != B_OK)
		nextIndex = fTabManager->CountTabs();

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
			_SendNotification(notification, "FILE_ERR");
			return B_ERROR;
		}

		status = fEditor->LoadFromFile();

		if (status != B_OK) {
			continue;
		}

		fEditor->ApplySettings();

		// First tab gets selected by tabview
		if (index > 0)
			fTabManager->SelectTab(index, true);

		notification << fEditor->Name() << " " << B_TRANSLATE("opened with index")
			<< " " << fTabManager->CountTabs() - 1;
		_SendNotification(notification, "FILE_OPEN");
		notification.SetTo("");
	}

#if defined MULTIFILE_OPEN_SELECT_FIRST_FILE
	// Needs modified libscintilla
	// If at least 1 item or more were added select the first
	// of them. see below
	if (nextIndex < fTabManager->CountTabs()) {
		fTabManager->SelectTab(nextIndex);
//		fEditor->GrabFocus();
}
#else
	// If at least one item added, select last opened file:
	// it grabs keyboard focus anyway so fix that if you want to change
	//  selection management on multi-open.
	int32 tabs = fTabManager->CountTabs();
	if (nextIndex < tabs) {
		fTabManager->SelectTab(tabs - 1);
		fEditor->GrabFocus();
	}
#endif

	return status;
}

status_t
IdeamWindow::_FileSave(int32 index)
{
//	status_t status;
	BString notification;

	// Should not happen
	if (index < 0) {
		notification << (B_TRANSLATE("No file selected"));
		_SendNotification(notification, "FILE_ERR");
		return B_ERROR;
	}

	fEditor = fEditorObjectList->ItemAt(index);

	if (fEditor == nullptr) {
		notification << (B_TRANSLATE("NULL editor pointer"));
		_SendNotification(notification, "FILE_ERR");
		return B_ERROR;
	}

	// Readonly file, should not happen
	if (fEditor->IsReadOnly()) {
		notification << (B_TRANSLATE("File is Read-only"));
		_SendNotification(notification, "FILE_ERR");
		return B_ERROR;
	}

	// File not modified, happens at file save as
/*	if (!fEditor->IsModified()) {
		notification << (B_TRANSLATE("File not modified"));
		_SendNotification(notification, "FILE_ERR");
		return B_ERROR;
	}
*/
	// Stop monitoring if needed
	fEditor->StopMonitoring();

	ssize_t written = fEditor->SaveToFile();
	ssize_t length = fEditor->SendMessage(SCI_GETLENGTH, 0, 0);

	// Restart monitoring
	fEditor->StartMonitoring();

	notification << fEditor->Name()<< B_TRANSLATE(" saved.")
		<< "\t\t" << B_TRANSLATE("length: ") << length << B_TRANSLATE(" bytes -> ")
		<< written<< B_TRANSLATE(" bytes written");

	_SendNotification(notification, length == written ? "FILE_SAVE" : "FILE_ERR");

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
			_SendNotification(notification, "FILE_ERR");
			continue;
		}

		if (fEditor->IsModified())
			_FileSave(index);
	}
}

status_t
IdeamWindow::_FileSaveAs(int32 selection, BMessage* message)
{
	entry_ref ref;
	BString name;
	status_t status;

	if ((status = message->FindRef("directory", &ref)) != B_OK)
		return status;
	if ((status = message->FindString("name", &name)) != B_OK)
		return status;

	BPath path(&ref);
	path.Append(name);
	BEntry entry(path.Path(), true);
	entry_ref newRef;

	if ((status = entry.GetRef(&newRef)) != B_OK)
		return status;

	fEditor = fEditorObjectList->ItemAt(selection);

	if (fEditor == nullptr) {
		BString notification;
		notification << B_TRANSLATE("Index ") << selection
			<< (B_TRANSLATE(": NULL editor pointer"));
		_SendNotification(notification, "FILE_ERR");
		return B_ERROR;
	}

	fEditor->SetFileRef(&newRef);
	fTabManager->SetTabLabel(selection, fEditor->Name().String());

	/* Modified files 'Saved as' get saved to an unmodified state.
	 * It should be cool to take the modified state to the new file and let
	 * user choose to save or discard modifications. Left as a TODO.
	 * In case do not forget to update label
	 */
	//_UpdateLabel(selection, fEditor->IsModified());

	_FileSave(selection);

	return B_OK;
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

void
IdeamWindow::_FindGroupShow()
{
	if (!fFindGroup->IsVisible()) {
		fFindGroup->SetVisible(true);
		_GetFocusAndSelection(fFindTextControl);
	}
	else if (fFindGroup->IsVisible())
		_GetFocusAndSelection(fFindTextControl);
}

void
IdeamWindow::_FindGroupToggled()
{
	fFindGroup->SetVisible(!fFindGroup->IsVisible());

	if (fFindGroup->IsVisible()) {
		_GetFocusAndSelection(fFindTextControl);
	}
	else {
		if (fReplaceGroup->IsVisible())
			fReplaceGroup->SetVisible(false);
		int32 index = fTabManager->SelectedTabIndex();
		if (index > -1 && index < fTabManager->CountTabs()) {
			fEditor = fEditorObjectList->ItemAt(index);
			fEditor->GrabFocus();
		}
	}
}

int32
IdeamWindow::_FindMarkAll(const BString text)
{
	fEditor = fEditorObjectList->ItemAt(fTabManager->SelectedTabIndex());

	int flags = fEditor->SetSearchFlags(fFindCaseSensitiveCheck->Value(),
										fFindWholeWordCheck->Value(),
										false, false, false);

	int countMarks = fEditor->FindMarkAll(text, flags);

	fEditor->GrabFocus();

	_UpdateFindMenuItems(text);

	return countMarks;
}

void
IdeamWindow::_FindNext(const BString& strToFind, bool backwards)
{
	if (strToFind.IsEmpty())
		return;

	fEditor = fEditorObjectList->ItemAt(fTabManager->SelectedTabIndex());
//fFindTextControl->MakeFocus(true);
	fEditor->GrabFocus();

	int flags = fEditor->SetSearchFlags(fFindCaseSensitiveCheck->Value(),
										fFindWholeWordCheck->Value(),
										false, false, false);
	bool wrap = fFindWrapCheck->Value();

	if (backwards == false)
		fEditor->FindNext(strToFind, flags, wrap);
	else
		fEditor->FindPrevious(strToFind, flags, wrap);

	_UpdateFindMenuItems(strToFind);
}

int32
IdeamWindow::_GetEditorIndex(entry_ref* ref)
{
	BEntry entry(ref, true);
	int32 filesCount = fEditorObjectList->CountItems();

	// Could try to reopen at start a saved index that was deleted,
	// check existence
	if (entry.Exists() == false)
		return -1;
		
	for (int32 index = 0; index < filesCount; index++) {

		fEditor = fEditorObjectList->ItemAt(index);

		if (fEditor == nullptr) {
			BString notification;
			notification << B_TRANSLATE("Index ") << index
				<< (B_TRANSLATE(": NULL editor pointer"));
			_SendNotification(notification, "FILE_ERR");
			continue;
		}

		BEntry matchEntry(fEditor->FileRef(), true);

		if (matchEntry == entry)
			return index;
	}
	return -1;
}

int32
IdeamWindow::_GetEditorIndex(node_ref* nref)
{
	int32 filesCount = fEditorObjectList->CountItems();
	
	for (int32 index = 0; index < filesCount; index++) {

		fEditor = fEditorObjectList->ItemAt(index);

		if (fEditor == nullptr) {
			BString notification;
			notification << B_TRANSLATE("Index ") << index
				<< (B_TRANSLATE(": NULL editor pointer"));
			_SendNotification(notification, "FILE_ERR");
			continue;
		}

		if (*nref == *fEditor->NodeRef())
			return index;
	}
	return -1;
}


void
IdeamWindow::_GetFocusAndSelection(BTextControl* control)
{
	control->MakeFocus(true);
	// If some text is selected, use that TODO index check
	fEditor = fEditorObjectList->ItemAt(fTabManager->SelectedTabIndex());
	if (fEditor->IsTextSelected()) {
		int32 size = fEditor->SendMessage(SCI_GETSELTEXT, 0, 0);
		char text[size];
		fEditor->SendMessage(SCI_GETSELTEXT, 0, (sptr_t)text);
		control->SetText(text);
	}
	else
		control->TextView()->Clear();
}


void
IdeamWindow::_HandleExternalMoveModification(entry_ref* oldRef, entry_ref* newRef)
{
	BEntry oldEntry(oldRef, true), newEntry(newRef, true);
	BPath oldPath, newPath;

	oldEntry.GetPath(&oldPath);
	newEntry.GetPath(&newPath);

	BString text;
	text << IdeamNames::kApplicationName << ":\n";
	text << (B_TRANSLATE("File \"%file%\" was moved externally,\n"
							"do You want to ignore, close or reload it?"));
	text.ReplaceAll("%file%", oldRef->name);

	BAlert* alert = new BAlert("FileMoveDialog", text,
 		B_TRANSLATE("Ignore"), B_TRANSLATE("Close"), B_TRANSLATE("Reload"),
 		B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);

 	alert->SetShortcut(0, B_ESCAPE);

	int32 index = _GetEditorIndex(oldRef);

 	int32 choice = alert->Go();
 
 	if (choice == 0)
		return;
	else if (choice == 1)
		_FileClose(index);
	else if (choice == 2) {
		fEditor = fEditorObjectList->ItemAt(index);
		fEditor->SetFileRef(newRef);
		fTabManager->SetTabLabel(index, fEditor->Name().String());
		_UpdateLabel(index, fEditor->IsModified());

		BString notification;
		notification << oldPath.Path() << B_TRANSLATE(" moved externally to ");
		notification << newPath.Path();
		_SendNotification(notification, "FILE_INFO");
	}
}

void
IdeamWindow::_HandleExternalRemoveModification(int32 index)
{
	if (index < 0) {
		return; //TODO notify
	}

	fEditor = fEditorObjectList->ItemAt(index);

	BString text;
	text << IdeamNames::kApplicationName << ":\n";
	text << (B_TRANSLATE("File \"%file%\" was removed externally,\n"
							"do You want to keep the file or discard it?\n"
							"If kept and modified save it or it will be lost"));
	text.ReplaceAll("%file%", fEditor->Name());

	BAlert* alert = new BAlert("FileRemoveDialog", text,
 		B_TRANSLATE("Keep"), B_TRANSLATE("Discard"), nullptr,
 		B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);

 	alert->SetShortcut(0, B_ESCAPE);

 	int32 choice = alert->Go();

 	if (choice == 0) {
	 	// If not modified save it or it will be lost, if modified let
	 	// the user decide
	 	if (fEditor->IsModified() == false)
			_FileSave(index);
		return;
	}	
	else if (choice == 1) {

		_FileClose(index, true);

		BString notification;
		notification << fEditor->Name() << B_TRANSLATE(" removed externally");
		_SendNotification(notification, "FILE_INFO");
	}
}

void
IdeamWindow::_HandleExternalStatModification(int32 index)
{
	if (index < 0) {
		return; //TODO notify
	}

	fEditor = fEditorObjectList->ItemAt(index);

	BString text;
	text << IdeamNames::kApplicationName << ":\n";
	text << (B_TRANSLATE("File \"%file%\" was modified externally, reload it?"));
	text.ReplaceAll("%file%", fEditor->Name());

	BAlert* alert = new BAlert("FileReloadDialog", text,
 		B_TRANSLATE("Ignore"), B_TRANSLATE("Reload"), nullptr,
 		B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);

 	alert->SetShortcut(0, B_ESCAPE);

 	int32 choice = alert->Go();
 
 	if (choice == 0)
		return;
	else if (choice == 1) {
		fEditor->Reload();

		BString notification;
		notification << fEditor->Name() << B_TRANSLATE(" modified externally");
		_SendNotification(notification, "FILE_INFO");
	}
}

void
IdeamWindow::_HandleNodeMonitorMsg(BMessage* msg)
{
	int32 opcode;
	status_t status;

	if ((status = msg->FindInt32("opcode", &opcode)) != B_OK) {
		// TODO notify
		return;
	}	

	switch (opcode) {
		case B_ENTRY_MOVED: {
			int32 device;
			int64 srcDir;
			int64 dstDir;
			BString name;
			const char* oldName;

			if (msg->FindInt32("device", &device) != B_OK
				|| msg->FindInt64("to directory", &dstDir) != B_OK
				|| msg->FindInt64("from directory", &srcDir) != B_OK
				|| msg->FindString("name", &name) != B_OK
				|| msg->FindString("from name", &oldName) != B_OK)
					break;

			entry_ref oldRef(device, srcDir, oldName);
			entry_ref newRef(device, dstDir, name);

			_HandleExternalMoveModification(&oldRef, &newRef);

			break;
		}
		case B_ENTRY_REMOVED: {
			node_ref nref;
			BString name;
			int64 dir;

			if (msg->FindInt32("device", &nref.device) != B_OK
				|| msg->FindString("name", &name) != B_OK
				|| msg->FindInt64("directory", &dir) != B_OK
				|| msg->FindInt64("node", &nref.node) != B_OK)
					break;

			_HandleExternalRemoveModification(_GetEditorIndex(&nref));
			//entry_ref ref(device, dir, name);
			//_HandleExternalRemoveModification(_GetEditorIndex(&ref));
			break;
		}
		case B_STAT_CHANGED: {
			node_ref nref;
			int32 fields;

			if (msg->FindInt32("device", &nref.device) != B_OK
				|| msg->FindInt64("node", &nref.node) != B_OK
				|| msg->FindInt32("fields", &fields) != B_OK)
					break;
#if defined DEBUG
switch (fields) {
	case B_STAT_MODE:
	case B_STAT_UID:
	case B_STAT_GID:
std::cerr << __PRETTY_FUNCTION__ << " MODES" << std::endl;
		break;
	case B_STAT_SIZE:
std::cerr << __PRETTY_FUNCTION__ << " B_STAT_SIZE" << std::endl;
		break;
	case B_STAT_ACCESS_TIME:
std::cerr << __PRETTY_FUNCTION__ << " B_STAT_ACCESS_TIME" << std::endl;
		break;
	case B_STAT_MODIFICATION_TIME:
std::cerr << __PRETTY_FUNCTION__ << " B_STAT_MODIFICATION_TIME" << std::endl;
		break;
	case B_STAT_CREATION_TIME:
std::cerr << __PRETTY_FUNCTION__ << " B_STAT_CREATION_TIME" << std::endl;
		break;
	case B_STAT_CHANGE_TIME:
std::cerr << __PRETTY_FUNCTION__ << " B_STAT_CHANGE_TIME" << std::endl;
		break;
	case B_STAT_INTERIM_UPDATE:
std::cerr << __PRETTY_FUNCTION__ << " B_STAT_INTERIM_UPDATE" << std::endl;
		break;
	default:
std::cerr << __PRETTY_FUNCTION__ << "fields is: 0x" << std::hex << fields << std::endl;
		break;
}
#endif
			if ((fields & (B_STAT_MODE | B_STAT_UID | B_STAT_GID)) != 0)
				; 			// TODO recheck permissions
			/* 
			 * Note: Pe and StyledEdit seems to cope differently on modifications,
			 *       firing different messages on the same modification of the
			 *       same file.
			 *   E.g. on changing file size
			 *				Pe fires				StyledEdit fires
			 *			B_STAT_CHANGE_TIME			B_STAT_CHANGE_TIME
			 *			B_STAT_CHANGE_TIME			fields is: 0x2008
			 *			fields is: 0x28				B_STAT_CHANGE_TIME
			 *										B_STAT_CHANGE_TIME
			 *										B_STAT_CHANGE_TIME
			 *										B_STAT_MODIFICATION_TIME
			 *
			 *   E.g. on changing file data but keeping the same file size
			 *				Pe fires				StyledEdit fires
			 *			B_STAT_CHANGE_TIME			B_STAT_CHANGE_TIME
			 *			B_STAT_CHANGE_TIME			fields is: 0x2008
			 *			B_STAT_MODIFICATION_TIME	B_STAT_CHANGE_TIME
			 *										B_STAT_CHANGE_TIME
			 *										B_STAT_CHANGE_TIME
			 *										B_STAT_MODIFICATION_TIME
			 */
			if (((fields & B_STAT_MODIFICATION_TIME)  != 0)
			// Do not reload if the file just got touched 
				&& ((fields & B_STAT_ACCESS_TIME)  == 0)) {
				_HandleExternalStatModification(_GetEditorIndex(&nref));
			}

			break;
		}
		default:
			break;
	}
}

void
IdeamWindow::_InitMenu()
{
	// Menu
	fMenuBar = new BMenuBar("menubar");

	BMenu* menu = new BMenu(B_TRANSLATE("Project"));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Quit"),
		new BMessage(B_QUIT_REQUESTED), 'Q'));
	fMenuBar->AddItem(menu);

	menu = new BMenu(B_TRANSLATE("File"));
	menu->AddItem(fFileNewMenuItem = new BMenuItem(B_TRANSLATE("New"),
		new BMessage(MSG_FILE_NEW)));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Open"),
		new BMessage(MSG_FILE_OPEN), 'O'));
	menu->AddItem(new BMenuItem(BRecentFilesList::NewFileListMenu(
			B_TRANSLATE("Open recent" B_UTF8_ELLIPSIS), nullptr, nullptr, this,
			kRecentFilesNumber, true, nullptr, IdeamNames::kApplicationSignature), nullptr));
	menu->AddSeparatorItem();
	menu->AddItem(fSaveMenuItem = new BMenuItem(B_TRANSLATE("Save"),
		new BMessage(MSG_FILE_SAVE), 'S'));
	menu->AddItem(fSaveAsMenuItem = new BMenuItem(B_TRANSLATE("Save as" B_UTF8_ELLIPSIS),
		new BMessage(MSG_FILE_SAVE_AS)));
	menu->AddItem(fSaveAllMenuItem = new BMenuItem(B_TRANSLATE("Save all"),
		new BMessage(MSG_FILE_SAVE_ALL), 'S', B_SHIFT_KEY));
	menu->AddSeparatorItem();
	menu->AddItem(fCloseMenuItem = new BMenuItem(B_TRANSLATE("Close"),
		new BMessage(MSG_FILE_CLOSE), 'W'));
	menu->AddItem(fCloseAllMenuItem = new BMenuItem(B_TRANSLATE("Close all"),
		new BMessage(MSG_FILE_CLOSE_ALL), 'W', B_SHIFT_KEY));
	fFileNewMenuItem->SetEnabled(false);

	menu->AddSeparatorItem();
	menu->AddItem(fFoldMenuItem = new BMenuItem(B_TRANSLATE("Fold"),
		new BMessage(MSG_FILE_FOLD_TOGGLE)));

	fSaveMenuItem->SetEnabled(false);
	fSaveAsMenuItem->SetEnabled(false);
	fSaveAllMenuItem->SetEnabled(false);
	fCloseMenuItem->SetEnabled(false);
	fCloseAllMenuItem->SetEnabled(false);
	fFoldMenuItem->SetEnabled(false);

	fMenuBar->AddItem(menu);

	menu = new BMenu(B_TRANSLATE("Edit"));
	menu->AddItem(fUndoMenuItem = new BMenuItem(B_TRANSLATE("Undo"),
		new BMessage(B_UNDO), 'Z'));
	menu->AddItem(fRedoMenuItem = new BMenuItem(B_TRANSLATE("Redo"),
		new BMessage(B_REDO), 'Z', B_SHIFT_KEY));
	menu->AddSeparatorItem();
	menu->AddItem(fCutMenuItem = new BMenuItem(B_TRANSLATE("Cut"),
		new BMessage(B_CUT), 'X'));
	menu->AddItem(fCopyMenuItem = new BMenuItem(B_TRANSLATE("Copy"),
		new BMessage(B_COPY), 'C'));
	menu->AddItem(fPasteMenuItem = new BMenuItem(B_TRANSLATE("Paste"),
		new BMessage(B_PASTE), 'V'));
	menu->AddItem(fDeleteMenuItem = new BMenuItem(B_TRANSLATE("Delete"),
		new BMessage(MSG_TEXT_DELETE), 'D'));
	menu->AddSeparatorItem();
	menu->AddItem(fSelectAllMenuItem = new BMenuItem(B_TRANSLATE("Select all"),
		new BMessage(B_SELECT_ALL), 'A'));
	menu->AddSeparatorItem();
	menu->AddItem(fOverwiteItem = new BMenuItem(B_TRANSLATE("Overwrite"),
		new BMessage(MSG_TEXT_OVERWRITE), B_INSERT));


	menu->AddSeparatorItem();
	menu->AddItem(fToggleWhiteSpacesItem = new BMenuItem(B_TRANSLATE("Toggle white spaces"),
		new BMessage(MSG_WHITE_SPACES_TOGGLE)));
	menu->AddItem(fToggleLineEndingsItem = new BMenuItem(B_TRANSLATE("Toggle line endings"),
		new BMessage(MSG_LINE_ENDINGS_TOGGLE)));

	fUndoMenuItem->SetEnabled(false);
	fRedoMenuItem->SetEnabled(false);
	fCutMenuItem->SetEnabled(false);
	fCopyMenuItem->SetEnabled(false);
	fPasteMenuItem->SetEnabled(false);
	fDeleteMenuItem->SetEnabled(false);
	fSelectAllMenuItem->SetEnabled(false);
	fOverwiteItem->SetEnabled(false);
	fToggleWhiteSpacesItem->SetEnabled(false);
	fToggleLineEndingsItem->SetEnabled(false);

	fMenuBar->AddItem(menu);

	menu = new BMenu(B_TRANSLATE("Search"));

	menu->AddItem(fFindItem = new BMenuItem(B_TRANSLATE("Find"),
		new BMessage(MSG_FIND_GROUP_SHOW), 'F'));
	menu->AddItem(fReplaceItem = new BMenuItem(B_TRANSLATE("Replace"),
		new BMessage(MSG_REPLACE_GROUP_SHOW), 'R'));
	menu->AddItem(fGoToLineItem = new BMenuItem(B_TRANSLATE("Go to line" B_UTF8_ELLIPSIS),
		new BMessage(MSG_GOTO_LINE), '<'));

	BMenu* submenu = new BMenu(B_TRANSLATE("Bookmark"));
	submenu->AddItem(fBookmarkToggleItem = new BMenuItem(B_TRANSLATE("Toggle"),
		new BMessage(MSG_BOOKMARK_TOGGLE)));
	submenu->AddItem(fBookmarkClearAllItem = new BMenuItem(B_TRANSLATE("Clear all"),
		new BMessage(MSG_BOOKMARK_CLEAR_ALL)));
	submenu->AddItem(fBookmarkGoToNextItem = new BMenuItem(B_TRANSLATE("Go to next"),
		new BMessage(MSG_BOOKMARK_GOTO_NEXT)));
	submenu->AddItem(fBookmarkGoToPreviousItem = new BMenuItem(B_TRANSLATE("Go to previous"),
		new BMessage(MSG_BOOKMARK_GOTO_PREVIOUS)));

	fFindItem->SetEnabled(false);
	fReplaceItem->SetEnabled(false);
	fGoToLineItem->SetEnabled(false);
	fBookmarkToggleItem->SetEnabled(false);
	fBookmarkClearAllItem->SetEnabled(false);
	fBookmarkGoToNextItem->SetEnabled(false);
	fBookmarkGoToPreviousItem->SetEnabled(false);

	menu->AddItem(submenu);
	fMenuBar->AddItem(menu);

	menu = new BMenu(B_TRANSLATE("Window"));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Settings"),
		new BMessage(MSG_WINDOW_SETTINGS), 'P', B_OPTION_KEY));
	submenu = new BMenu(B_TRANSLATE("Interface"));
	submenu->AddItem(new BMenuItem(B_TRANSLATE("Toggle Projects panes"),
		new BMessage(MSG_SHOW_HIDE_PROJECTS)));
	submenu->AddItem(new BMenuItem(B_TRANSLATE("Toggle Output panes"),
		new BMessage(MSG_SHOW_HIDE_OUTPUT)));
	submenu->AddItem(new BMenuItem(B_TRANSLATE("Toggle ToolBar"),
		new BMessage(MSG_TOGGLE_TOOLBAR)));
	menu->AddItem(submenu);

	fMenuBar->AddItem(menu);

	menu = new BMenu(B_TRANSLATE("Help"));
	menu->AddItem(new BMenuItem(B_TRANSLATE("About" B_UTF8_ELLIPSIS),
		new BMessage(B_ABOUT_REQUESTED)));

	fMenuBar->AddItem(menu);
}


void
IdeamWindow::_InitWindow()
{
	// toolbar group
	fProjectsButton = _LoadIconButton("ProjectsButton", MSG_SHOW_HIDE_PROJECTS,
						111, true, B_TRANSLATE("Show/Hide Projects split"));
	fOutputButton = _LoadIconButton("OutputButton", MSG_SHOW_HIDE_OUTPUT,
						115, true, B_TRANSLATE("Show/Hide Output split"));

	fFindButton = _LoadIconButton("Find", MSG_FIND_GROUP_TOGGLED, 199, false,
						B_TRANSLATE("Find toggle (closes Replace bar if open)"));

	fReplaceButton = _LoadIconButton("Replace", MSG_REPLACE_GROUP_TOGGLED, 200, false,
						B_TRANSLATE("Replace toggle (leaves Find bar open)"));

	fFindinFilesButton = _LoadIconButton("FindinFiles", MSG_FIND_IN_FILES, 201, false,
						B_TRANSLATE("Find in files"));

	fFoldButton = _LoadIconButton("Fold", MSG_FILE_FOLD_TOGGLE, 213, false,
						B_TRANSLATE("Fold toggle"));
	fUndoButton = _LoadIconButton("UndoButton", B_UNDO, 204, false,
						B_TRANSLATE("Undo"));
	fRedoButton = _LoadIconButton("RedoButton", B_REDO, 205, false,
						B_TRANSLATE("Redo"));
	fFileSaveButton = _LoadIconButton("FileSaveButton", MSG_FILE_SAVE,
						206, false, B_TRANSLATE("Save current File"));
	fFileSaveAllButton = _LoadIconButton("FileSaveAllButton", MSG_FILE_SAVE_ALL,
						207, false, B_TRANSLATE("Save all Files"));

	fFileUnlockedButton = _LoadIconButton("FileUnlockedButton", MSG_BUFFER_LOCK,
						212, false, B_TRANSLATE("Set buffer read-only"));
	fFilePreviousButton = _LoadIconButton("FilePreviousButton", MSG_FILE_PREVIOUS_SELECTED,
						208, false, B_TRANSLATE("Select previous File"));
	fFileNextButton = _LoadIconButton("FileNextButton", MSG_FILE_NEXT_SELECTED,
						209, false, B_TRANSLATE("Select next File"));
	fFileCloseButton = _LoadIconButton("FileCloseButton", MSG_FILE_CLOSE,
						210, false, B_TRANSLATE("Close File"));
	fFileMenuButton = _LoadIconButton("FileMenuButton", MSG_FILE_MENU_SHOW,
						211, false, B_TRANSLATE("Indexed File list"));

	fGotoLine = new BTextControl("GotoLine", nullptr, nullptr, new BMessage(MSG_LINE_TO_GOTO));
	fGotoLine->SetToolTip(B_TRANSLATE("Go to line" B_UTF8_ELLIPSIS));
	float stringWidth = fGotoLine->StringWidth(" 123456") + 24.0f;
	fGotoLine->SetExplicitMaxSize(BSize(stringWidth, 24.0f)); //B_SIZE_UNSET
	for (auto i = 0; i < 256; i++)
		if (i < '0' || i > '9')
			fGotoLine->TextView()->DisallowChar(i);
	fGotoLine->TextView()->SetMaxBytes(kGotolineMaxBytes);
	fGotoLine->TextView()->SetAlignment(B_ALIGN_RIGHT);
	fGotoLine->Hide();

	fToolBar = BLayoutBuilder::Group<>(B_VERTICAL, 0)
		.Add(BLayoutBuilder::Group<>(B_HORIZONTAL, 1)
			.AddGlue()
			.Add(fProjectsButton)
			.Add(fOutputButton)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.Add(fFoldButton)
			.Add(fUndoButton)
			.Add(fRedoButton)
			.Add(fFileSaveButton)
			.Add(fFileSaveAllButton)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.Add(fFindButton)
			.Add(fReplaceButton)
			.Add(fFindinFilesButton)
			.AddGlue()
			.Add(fFileUnlockedButton)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.Add(fGotoLine)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
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

	// Find group
	fFindMenuField = new BMenuField("FindMenuField", NULL, new BMenu(B_TRANSLATE("Find:")));
	fFindMenuField->SetExplicitMaxSize(BSize(kFindReplaceOPSize, B_SIZE_UNSET));
	fFindMenuField->SetExplicitMinSize(BSize(kFindReplaceOPSize, B_SIZE_UNSET));

	fFindTextControl = new BTextControl("FindTextControl", "" , "", nullptr);
	fFindTextControl->TextView()->SetMaxBytes(kFindReplaceMaxBytes);
	float charWidth = fFindTextControl->StringWidth("0", 1);
	fFindTextControl->SetExplicitMinSize(
						BSize(charWidth * kFindReplaceMinBytes + 10.0f,
						B_SIZE_UNSET));
	fFindTextControl->SetExplicitMaxSize(fFindTextControl->MinSize());

	fFindNextButton = _LoadIconButton("FindNextButton", MSG_FIND_NEXT, 164, true,
						B_TRANSLATE("Find Next"));
	fFindPreviousButton = _LoadIconButton("FindPreviousButton", MSG_FIND_PREVIOUS,
							165, true, B_TRANSLATE("Find previous"));
	fFindMarkAllButton = _LoadIconButton("FindMarkAllButton", MSG_FIND_MARK_ALL,
							202, true, B_TRANSLATE("Mark all"));
//	AddShortcut(B_DOWN_ARROW, B_COMMAND_KEY, new BMessage(MSG_FIND_NEXT));
//	AddShortcut(B_UP_ARROW, B_COMMAND_KEY, new BMessage(MSG_FIND_PREVIOUS));
//	AddShortcut(B_PAGE_DOWN, B_COMMAND_KEY, new BMessage(MSG_FIND_NEXT));
//	AddShortcut(B_PAGE_UP, B_COMMAND_KEY, new BMessage(MSG_FIND_PREVIOUS));

	fFindCaseSensitiveCheck = new BCheckBox(B_TRANSLATE("Match case"));
	fFindWholeWordCheck = new BCheckBox(B_TRANSLATE("Whole word"));
	fFindWrapCheck = new BCheckBox(B_TRANSLATE("Wrap"));

	fFindGroup = BLayoutBuilder::Group<>(B_VERTICAL, 0.0)
		.Add(BLayoutBuilder::Group<>(B_HORIZONTAL, B_USE_SMALL_SPACING)
			.Add(fFindMenuField)
			.Add(fFindTextControl)
			.Add(fFindNextButton)
			.Add(fFindPreviousButton)
			.Add(fFindWrapCheck)
			.Add(fFindWholeWordCheck)
			.Add(fFindCaseSensitiveCheck)
			.Add(fFindMarkAllButton)
			.AddGlue()
///			.SetInsets(2, 2, 2, 2)
		)
		.Add(new BSeparatorView(B_HORIZONTAL, B_PLAIN_BORDER))
	;
	fFindGroup->SetVisible(false);

	// Replace group
	fReplaceMenuField = new BMenuField("ReplaceMenu", NULL,
										new BMenu(B_TRANSLATE("Replace:")));
	fReplaceMenuField->SetExplicitMaxSize(BSize(kFindReplaceOPSize, B_SIZE_UNSET));
	fReplaceMenuField->SetExplicitMinSize(BSize(kFindReplaceOPSize, B_SIZE_UNSET));

	fReplaceTextControl = new BTextControl("ReplaceTextControl", "", "", NULL);
	fReplaceTextControl->TextView()->SetMaxBytes(kFindReplaceMaxBytes);
	fReplaceTextControl->SetExplicitMaxSize(fFindTextControl->MaxSize());
	fReplaceTextControl->SetExplicitMinSize(fFindTextControl->MinSize());

	fReplaceOneButton = _LoadIconButton("ReplaceOneButton", MSG_REPLACE_ONE,
						166, true, B_TRANSLATE("Replace selection"));
	fReplaceAndFindNextButton = _LoadIconButton("ReplaceFindNextButton", MSG_REPLACE_NEXT,
								167, true, B_TRANSLATE("Replace and find next"));
	fReplaceAndFindPrevButton = _LoadIconButton("ReplaceFindPrevButton", MSG_REPLACE_PREVIOUS,
								168, false, B_TRANSLATE("Replace and find previous"));
	fReplaceAllButton = _LoadIconButton("ReplaceAllButton", MSG_REPLACE_ALL,
							169, true, B_TRANSLATE("Replace all"));

	fReplaceGroup = BLayoutBuilder::Group<>(B_VERTICAL, 0.0)
		.Add(BLayoutBuilder::Group<>(B_HORIZONTAL, B_USE_SMALL_SPACING)
			.Add(fReplaceMenuField)
			.Add(fReplaceTextControl)
			.Add(fReplaceOneButton)
			.Add(fReplaceAndFindNextButton)
			.Add(fReplaceAndFindPrevButton)
			.Add(fReplaceAllButton)
			.AddGlue()
///			.SetInsets(2, 2, 2, 2)
		)
		.Add(new BSeparatorView(B_HORIZONTAL, B_PLAIN_BORDER))
	;
	fReplaceGroup->SetVisible(false);

	// Editor tab & view
	fEditorObjectList = new BObjectList<Editor>();

	fTabManager = new TabManager(BMessenger(this));
	fTabManager->TabGroup()->SetExplicitMaxSize(BSize(B_SIZE_UNSET, kTabBarHeight));

	dirtyFrameHack = fTabManager->TabGroup()->Frame();

	// Status Bar
	fStatusBar = new BStatusBar("StatusBar");
	fStatusBar->SetBarHeight(1.0);

	fEditorTabsGroup = BLayoutBuilder::Group<>(B_VERTICAL, 0.0)
		.SetInsets(1, 1, 1, 1)
		.Add(BLayoutBuilder::Group<>(B_VERTICAL, 0.0)
			.Add(fFindGroup)
			.Add(fReplaceGroup)
			.Add(fTabManager->TabGroup())
			.Add(fTabManager->ContainerView())
			.Add(new BSeparatorView(B_HORIZONTAL))
			.Add(fStatusBar)
		)
	;

	// Panels
	fOpenPanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), nullptr, B_FILE_NODE, true);
	fSavePanel = new BFilePanel(B_SAVE_PANEL, new BMessenger(this), nullptr, B_FILE_NODE, false);

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
}

BIconButton*
IdeamWindow::_LoadIconButton(const char* name, int32 msg,
								int32 resIndex, bool enabled, const char* tooltip)
{
	BIconButton* button = new BIconButton(name, nullptr, new BMessage(msg));
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

	assert(data != nullptr);

	BBitmap* bitmap = new BBitmap(BRect(0, 0, size, size), B_RGBA32);

	status_t status = BIconUtils::GetVectorIcon(static_cast<const uint8*>(data),
						iconSize, bitmap);

	assert(status == B_OK);

	return bitmap;
}

int
IdeamWindow::_Replace(int what)
{
	if (_ReplaceAllow() == false)
		return REPLACE_SKIP;

	BString selection(fFindTextControl->Text());
	BString replacement(fReplaceTextControl->Text());
	int retValue = REPLACE_NONE;

	fEditor = fEditorObjectList->ItemAt(fTabManager->SelectedTabIndex());
	int flags = fEditor->SetSearchFlags(fFindCaseSensitiveCheck->Value(),
										fFindWholeWordCheck->Value(),
										false, false, false);

	bool wrap = fFindWrapCheck->Value();

	switch (what) {
		case REPLACE_ALL: {
			retValue = fEditor->ReplaceAll(selection, replacement, flags);
			fEditor->GrabFocus();
			break;
		}
		case REPLACE_NEXT: {
			retValue = fEditor->ReplaceAndFindNext(selection, replacement, flags, wrap);
			break;
		}
		case REPLACE_ONE: {
			retValue = fEditor->ReplaceOne(selection, replacement);
			break;
		}
		case REPLACE_PREVIOUS: {
//			retValue = fEditor->ReplaceAndFindPrevious(selection, replacement, flags, wrap);
			break;
		}
		default:
			return REPLACE_NONE;
	}
	_UpdateFindMenuItems(fFindTextControl->Text());
	_UpdateReplaceMenuItems(fReplaceTextControl->Text());

	return retValue;
}

bool
IdeamWindow::_ReplaceAllow()
{
	BString selection(fFindTextControl->Text());
	BString replacement(fReplaceTextControl->Text());

	if (selection.Length() < 1
//			|| replacement.Length() < 1
			|| selection == replacement)
		return false;

	return true;
}
/*
void
IdeamWindow::_ReplaceAndFind()
{
	if (_ReplaceAllow() == false)
		return;

	BString selection(fFindTextControl->Text());
	BString replacement(fReplaceTextControl->Text());
	fEditor = fEditorObjectList->ItemAt(fTabManager->SelectedTabIndex());

	int flags = fEditor->SetSearchFlags(fFindCaseSensitiveCheck->Value(),
										fFindWholeWordCheck->Value(),
										false, false, false);

	bool wrap = fFindWrapCheck->Value();

	fEditor->ReplaceAndFind(selection, replacement, flags, wrap);

	_UpdateFindMenuItems(fFindTextControl->Text());
	_UpdateReplaceMenuItems(fReplaceTextControl->Text());
}
*/
void
IdeamWindow::_ReplaceGroupShow()
{
	bool findGroupOpen = fFindGroup->IsVisible();

	if (findGroupOpen == false)
		_FindGroupToggled();

	if (!fReplaceGroup->IsVisible()) {
		fReplaceGroup->SetVisible(true);
		fReplaceTextControl->TextView()->Clear();
		// If find group was not open focus and selection go there
		if (findGroupOpen == false)
			_GetFocusAndSelection(fFindTextControl);
		else
			_GetFocusAndSelection(fReplaceTextControl);
	}
	// Replace group was opened, get focus and selection
	else if (fReplaceGroup->IsVisible())
		_GetFocusAndSelection(fReplaceTextControl);
}

void
IdeamWindow::_ReplaceGroupToggled()
{
	fReplaceGroup->SetVisible(!fReplaceGroup->IsVisible());

	if (fReplaceGroup->IsVisible()) {
		if (!fFindGroup->IsVisible())
			_FindGroupToggled();
		else if (fFindGroup->IsVisible()) {
			// Find group was already visible, grab focus and selection
			// on replace text control
			_GetFocusAndSelection(fReplaceTextControl);
		}
	}
}

void
IdeamWindow::_SendNotification(BString message, BString type)
{
	if (IdeamNames::Settings.enable_notifications == false)
		return;

	// TODO bring to front
       BRow* fRow = new BRow();
       time_t now =  static_cast<bigtime_t>(real_time_clock());

       fRow->SetField(new BDateField(&now), kTimeColumn);
       fRow->SetField(new BStringField(message), kMessageColumn);
       fRow->SetField(new BStringField(type), kTypeColumn);
       fNotificationsListView->AddRow(fRow, 0);
}

void
IdeamWindow::_UpdateFindMenuItems(const BString& text)
{
	int32 count = fFindMenuField->Menu()->CountItems();
	// Add item if not already present
	if (fFindMenuField->Menu()->FindItem(text) == nullptr) {
		BMenuItem* item = new BMenuItem(text, new BMessage(MSG_FIND_MENU_SELECTED));
		fFindMenuField->Menu()->AddItem(item, 0);
	}
	if (count == kFindReplaceMenuItems)
		fFindMenuField->Menu()->RemoveItem(count);
}

status_t
IdeamWindow::_UpdateLabel(int32 index, bool isModified)
{
	if (index > -1 && index < fTabManager->CountTabs()) {
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
IdeamWindow::_UpdateReplaceMenuItems(const BString& text)
{
	int32 items = fReplaceMenuField->Menu()->CountItems();
	// Add item if not already present
	if (fReplaceMenuField->Menu()->FindItem(text) == NULL) {
		BMenuItem* item = new BMenuItem(text, new BMessage(MSG_REPLACE_MENU_SELECTED));
		fReplaceMenuField->Menu()->AddItem(item, 0);
	}
	if (items == kFindReplaceMenuItems)
		fReplaceMenuField->Menu()->RemoveItem(items);
}


/*
 * Updating menu, toolbar, title.
 * Also cleaning statusbar if no open files
 */
void
IdeamWindow::_UpdateSelectionChange(int32 index)
{
	// Should not happen
	if (index < -1)
		return;

	// All files are closed
	if (index == -1) {
		// ToolBar Items
		fFindButton->SetEnabled(false);
		fFindGroup->SetVisible(false);
		fReplaceButton->SetEnabled(false);
		fReplaceGroup->SetVisible(false);
		fFoldButton->SetEnabled(false);
		fUndoButton->SetEnabled(false);
		fRedoButton->SetEnabled(false);
		fFileSaveButton->SetEnabled(false);
		fFileSaveAllButton->SetEnabled(false);
		fFileUnlockedButton->SetEnabled(false);
		fFilePreviousButton->SetEnabled(false);
		fFileNextButton->SetEnabled(false);
		fFileCloseButton->SetEnabled(false);
		fFileMenuButton->SetEnabled(false);

		// Menu Items
		fSaveMenuItem->SetEnabled(false);
		fSaveAsMenuItem->SetEnabled(false);
		fSaveAllMenuItem->SetEnabled(false);
		fCloseMenuItem->SetEnabled(false);
		fFoldMenuItem->SetEnabled(false);
		fCloseAllMenuItem->SetEnabled(false);
		fUndoMenuItem->SetEnabled(false);
		fRedoMenuItem->SetEnabled(false);
		fCutMenuItem->SetEnabled(false);
		fCopyMenuItem->SetEnabled(false);
		fPasteMenuItem->SetEnabled(false);
		fDeleteMenuItem->SetEnabled(false);
		fSelectAllMenuItem->SetEnabled(false);
		fOverwiteItem->SetEnabled(false);
		fToggleWhiteSpacesItem->SetEnabled(false);
		fToggleLineEndingsItem->SetEnabled(false);
		fFindItem->SetEnabled(false);
		fReplaceItem->SetEnabled(false);
		fGoToLineItem->SetEnabled(false);
		fBookmarkToggleItem->SetEnabled(false);
		fBookmarkClearAllItem->SetEnabled(false);
		fBookmarkGoToNextItem->SetEnabled(false);
		fBookmarkGoToPreviousItem->SetEnabled(false);

		// Clean Status bar
		fStatusBar->Reset();

		if (IdeamNames::Settings.fullpath_title == true)
			SetTitle(IdeamNames::kApplicationName);

		return;
	}

	// ToolBar Items
	fFindButton->SetEnabled(true);
	fReplaceButton->SetEnabled(true);
	fFoldButton->SetEnabled(fEditor->IsFoldingAvailable());
	fUndoButton->SetEnabled(fEditor->CanUndo());
	fRedoButton->SetEnabled(fEditor->CanRedo());
	fFileSaveButton->SetEnabled(fEditor->IsModified());
	fFileUnlockedButton->SetEnabled(!fEditor->IsReadOnly());
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

	// Menu Items
	fSaveMenuItem->SetEnabled(fEditor->IsModified());
	fSaveAsMenuItem->SetEnabled(true);
	fCloseMenuItem->SetEnabled(true);
	fCloseAllMenuItem->SetEnabled(true);
	fFoldMenuItem->SetEnabled(fEditor->IsFoldingAvailable());
	fUndoMenuItem->SetEnabled(fEditor->CanUndo());
	fRedoMenuItem->SetEnabled(fEditor->CanRedo());
	fCutMenuItem->SetEnabled(fEditor->CanCut());
	fCopyMenuItem->SetEnabled(fEditor->CanCopy());
	fPasteMenuItem->SetEnabled(fEditor->CanPaste());
	fDeleteMenuItem->SetEnabled(fEditor->CanClear());
	fSelectAllMenuItem->SetEnabled(true);
	fOverwiteItem->SetEnabled(true);
//fOverwiteItem->SetMarked(fEditor->IsOverwrite());
	fToggleWhiteSpacesItem->SetEnabled(true);
	fToggleLineEndingsItem->SetEnabled(true);
	fFindItem->SetEnabled(true);
	fReplaceItem->SetEnabled(true);
	fGoToLineItem->SetEnabled(true);
	fBookmarkToggleItem->SetEnabled(true);
	fBookmarkClearAllItem->SetEnabled(true);
	fBookmarkGoToNextItem->SetEnabled(true);
	fBookmarkGoToPreviousItem->SetEnabled(true);

	if (IdeamNames::Settings.fullpath_title == true) {
		BString title;
		title << IdeamNames::kApplicationName << ": " << fEditor->FilePath();
		SetTitle(title.String());
	}

	// fEditor is modified by _FilesNeedSave so it should be the last
	// or reload editor pointer
	bool filesNeedSave = _FilesNeedSave();
	fFileSaveAllButton->SetEnabled(filesNeedSave);
	fSaveAllMenuItem->SetEnabled(filesNeedSave);
/*	fEditor = fEditorObjectList->ItemAt(index);
	// This could be checked too
	if (fTabManager->SelectedTabIndex() != index);
*/

}

/*
 * Status bar is cleaned (both text and trailing) when
 * _UpdateSelectionChange is called with -1 index
 *
 */
void
IdeamWindow::_UpdateStatusBarText(int line, int column)
{
	BString text;
	text << "  " << line << ':' << column;
	fStatusBar->SetText(text.String());
}

/*
 * Index has to be verified before the call
 * so it is not checked here too
 */
void
IdeamWindow::_UpdateStatusBarTrailing(int32 index)
{
	BString trailing;

	fEditor = fEditorObjectList->ItemAt(index);

	if (fEditor->IsOverwrite())
		trailing << "OVR";
	else
		trailing << "INS";
/*
	trailing << '\t' << fEditor->EndOfLineString();

	switch (fEditor->Encoding()) {
		case B_UNICODE_UTF8:
			trailing << '\t' << "UTF-8" << '\t';
			break;
		default:
			break;
	}
*/
	fStatusBar->SetTrailingText(trailing.String());
}
