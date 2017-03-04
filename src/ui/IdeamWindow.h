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
#include <ObjectList.h>
#include <OutlineListView.h>
#include <ScrollView.h>
#include <StatusBar.h>
#include <TabView.h>
#include <TextView.h>
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

	virtual void				MessageReceived(BMessage* message);

private:

			status_t			_AddEditorTab(entry_ref* ref, int32 index);
			status_t			_FileClose(int32 index);
			void				_FileCloseAll();
			status_t			_FileOpen(BMessage* msg);
			status_t			_FileSave(int32	index);
			void				_FileSaveAll();
			bool				_FilesNeedSave();
			int32				_GetEditorIndex(entry_ref* ref);
			BIconButton*		_LoadIconButton(const char* name, int32 msg,
									int32 resIndex, bool enabled, const char* tooltip);
			BBitmap*			_LoadSizedVectorIcon(int32 resourceID, int32 size);
			void				_SendNotification(const char* message, const char* type);
			status_t			_UpdateLabel(int32 index, bool isModified);
			void				_UpdateSelectionChange(int32 index);
private:
			BGroupLayout*		fRootLayout;
			BGroupLayout*		fEditorTabsGroup;

			BIconButton*		fProjectsButton;
			BIconButton*		fOutputButton;
			BIconButton*		fFileSaveButton;
			BIconButton*		fFileSaveAllButton;
			BIconButton*		fFilePreviousButton;
			BIconButton*		fFileNextButton;
			BIconButton*		fFileCloseButton;
			BIconButton*		fFileMenuButton;

			BTabView*	  		fProjectsTabView;
			BOutlineListView*	fProjectsOutline;
			BScrollView*		fProjectsScroll;

			TabManager*			fTabManager;

		BObjectList<Editor>*	fEditorObjectList;
			Editor*				fEditor;

			BStatusBar*			fStatusBar;
			BFilePanel*			fOpenPanel;

			BTabView*			fOutputTabView;
			BColumnListView*	fNotificationsListView;

};

#endif //IDEAMWINDOW_H
