/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef IDEAMWINDOW_H
#define IDEAMWINDOW_H

#include <Bitmap.h>
#include <ColumnListView.h>
#include <ColumnTypes.h>
#include <FilePanel.h>
#include <GroupLayout.h>
#include <IconButton.h>
#include <MenuBar.h>
#include <ObjectList.h>
#include <OutlineListView.h>
#include <ScrollView.h>
#include <StatusBar.h>
#include <TabView.h>
#include <TabView.h>
#include <TextControl.h>
#include <Window.h>

#include "Editor.h"
#include "TabManager.h"

enum {
	kTimeColumn = 0,
	kMessageColumn,
	kTypeColumn
};

class IdeamWindow : public BWindow
{
public:
								IdeamWindow(BRect frame);
	virtual						~IdeamWindow();

	virtual void				DispatchMessage(BMessage* message, BHandler* handler);
	virtual void				MessageReceived(BMessage* message);
	virtual bool				QuitRequested();

private:

			status_t			_AddEditorTab(entry_ref* ref, int32 index);
			status_t			_FileClose(int32 index, bool ignoreModifications = false);
			void				_FileCloseAll();
			status_t			_FileOpen(BMessage* msg);
			status_t			_FileSave(int32	index);
			void				_FileSaveAll();
			status_t			_FileSaveAs(int32 selection, BMessage* message);
			bool				_FilesNeedSave();
			int32				_GetEditorIndex(entry_ref* ref);
			int32				_GetEditorIndex(node_ref* nref);
			void				_HandleExternalMoveModification(entry_ref* oldRef, entry_ref* newRef);
			void				_HandleExternalRemoveModification(int32 index);
			void				_HandleExternalStatModification(int32 index);
			void				_HandleNodeMonitorMsg(BMessage* msg);
			void				_InitMenu();
			void				_InitWindow();
			BIconButton*		_LoadIconButton(const char* name, int32 msg,
									int32 resIndex, bool enabled, const char* tooltip);
			BBitmap*			_LoadSizedVectorIcon(int32 resourceID, int32 size);
			void				_SendNotification(BString message, BString type);
			status_t			_UpdateLabel(int32 index, bool isModified);
			void				_UpdateSelectionChange(int32 index);
			void				_UpdateStatusBarText(int line, int column);
private:
			BMenuBar*			fMenuBar;
			BMenuItem*			fFileNewMenuItem;
			BMenuItem*			fSaveMenuItem;
			BMenuItem*			fSaveAsMenuItem;
			BMenuItem*			fSaveAllMenuItem;
			BMenuItem*			fCloseMenuItem;
			BMenuItem*			fCloseAllMenuItem;
			BMenuItem*			fUndoMenuItem;
			BMenuItem*			fRedoMenuItem;
			BMenuItem*			fCutMenuItem;
			BMenuItem*			fCopyMenuItem;
			BMenuItem*			fPasteMenuItem;
			BMenuItem*			fDeleteMenuItem;
			BMenuItem*			fSelectAllMenuItem;
			BMenuItem*			fToggleWhiteSpacesItem;
			BMenuItem*			fToggleLineEndingsItem;
			BMenuItem*			fGoToLineItem;
			BMenuItem*			fBookmarkToggleItem;
			BMenuItem*			fBookmarkClearAllItem;
			BMenuItem*			fBookmarkGoToNextItem;
			BMenuItem*			fBookmarkGoToPreviousItem;

			BGroupLayout*		fRootLayout;
			BGroupLayout* 		fToolBar;
			BGroupLayout*		fEditorTabsGroup;

			BIconButton*		fProjectsButton;
			BIconButton*		fOutputButton;
			BIconButton*		fUndoButton;
			BIconButton*		fRedoButton;
			BIconButton*		fFileSaveButton;
			BIconButton*		fFileSaveAllButton;
			BIconButton*		fFileUnlockedButton;
			BIconButton*		fFilePreviousButton;
			BIconButton*		fFileNextButton;
			BIconButton*		fFileCloseButton;
			BIconButton*		fFileMenuButton;
			BTextControl*		fGotoLine;

			BTabView*	  		fProjectsTabView;
			BOutlineListView*	fProjectsOutline;
			BScrollView*		fProjectsScroll;

			TabManager*			fTabManager;

		BObjectList<Editor>*	fEditorObjectList;
			Editor*				fEditor;

			BStatusBar*			fStatusBar;
			BFilePanel*			fOpenPanel;
			BFilePanel*			fSavePanel;

			BTabView*			fOutputTabView;
			BColumnListView*	fNotificationsListView;

};

#endif //IDEAMWINDOW_H
