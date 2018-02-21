/*
 * Copyright 2017..2018 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef IDEAMWINDOW_H
#define IDEAMWINDOW_H

#include <Bitmap.h>
#include <CheckBox.h>
#include <ColumnListView.h>
#include <ColumnTypes.h>
#include <FilePanel.h>
#include <GroupLayout.h>
#include <IconButton.h>
#include <MenuBar.h>
#include <ObjectList.h>
#include <OutlineListView.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <StatusBar.h>
#include <TabView.h>
#include <TextControl.h>
#include <Window.h>

#include "ConsoleIOThread.h"
#include "ConsoleIOView.h"
#include "Editor.h"
#include "Project.h"
#include "ProjectParser.h"
#include "TabManager.h"
#include "TPreferences.h"


enum {
	kTimeColumn = 0,
	kMessageColumn,
	kTypeColumn
};

enum {
	kNotificationLog = 0,
	kBuildLog,
	kOutputLog
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
			void				_BuildDone(BMessage* msg);
			status_t			_BuildProject();
			status_t			_CargoNew(BString args);
			status_t			_CleanProject();
	static	int					_CompareListItems(const BListItem* a,
									const BListItem* b);

			status_t			_DebugProject();
			status_t			_FileClose(int32 index, bool ignoreModifications = false);
			void				_FileCloseAll();
			status_t			_FileOpen(BMessage* msg);
			status_t			_FileSave(int32	index);
			void				_FileSaveAll();
			status_t			_FileSaveAs(int32 selection, BMessage* message);
			bool				_FilesNeedSave();
			void				_FindGroupShow();
			void				_FindGroupToggled();
			int32				_FindMarkAll(const BString text);
			void				_FindNext(const BString& strToFind, bool backwards);

			int32				_GetEditorIndex(entry_ref* ref);
			int32				_GetEditorIndex(node_ref* nref);
			void				_GetFocusAndSelection(BTextControl* control);
			status_t			_Git(const BString& git_command);
			void				_HandleExternalMoveModification(entry_ref* oldRef, entry_ref* newRef);
			void				_HandleExternalRemoveModification(int32 index);
			void				_HandleExternalStatModification(int32 index);
			void				_HandleNodeMonitorMsg(BMessage* msg);
			void				_InitCentralSplit();
			void				_InitMenu();
			void				_InitOutputSplit();
			void				_InitSideSplit();
			void				_InitToolbar();
			void				_InitWindow();
			BIconButton*		_LoadIconButton(const char* name, int32 msg,
									int32 resIndex, bool enabled, const char* tooltip);
			BBitmap*			_LoadSizedVectorIcon(int32 resourceID, int32 size);
			void				_MakeBindcatalogs();
			void				_MakeCatkeys();
			void				_ProjectActivate(BString const& projectName);
			void				_ProjectClose();
			void				_ProjectDelete(BString name, bool sourcesToo);
			void				_ProjectFileAdd();
			void				_ProjectFileDelete();
			void				_ProjectFileExclude();
			BString	const		_ProjectFileFullPath();
			void				_ProjectFileOpen(const BString& filePath);
			void				_ProjectFileRemoveItem(bool addToParseless);
			void				_ProjectItemChosen();
			void				_ProjectOpen(BString const& projectName, bool activate);
			void				_ProjectOutlineDepopulate(Project* project);
			void				_ProjectOutlinePopulate(Project* project);
			Project*			_ProjectPointerFromName(BString const& projectName);
			void				_ProjectRescan(BString const& projectName);
			int					_Replace(int what);
			bool				_ReplaceAllow();
			void				_ReplaceGroupShow();
			void				_ReplaceGroupToggled();
			status_t			_RunInConsole(const BString& command);
			void				_RunTarget();
			void				_SendNotification(BString message, BString type);
			void				_ShowLog(int32 index);
			void				_UpdateFindMenuItems(const BString& text);
			status_t			_UpdateLabel(int32 index, bool isModified);
			void				_UpdateProjectActivation(bool active);
			void				_UpdateReplaceMenuItems(const BString& text);
			void				_UpdateSelectionChange(int32 index);
			void				_UpdateStatusBarText(int line, int column);
			void				_UpdateStatusBarTrailing(int32 index);
private:
			BMenuBar*			fMenuBar;
			BMenuItem*			fFileNewMenuItem;
			BMenuItem*			fSaveMenuItem;
			BMenuItem*			fSaveAsMenuItem;
			BMenuItem*			fSaveAllMenuItem;
			BMenuItem*			fCloseMenuItem;
			BMenuItem*			fCloseAllMenuItem;
			BMenuItem*			fFoldMenuItem;
			BMenuItem*			fUndoMenuItem;
			BMenuItem*			fRedoMenuItem;
			BMenuItem*			fCutMenuItem;
			BMenuItem*			fCopyMenuItem;
			BMenuItem*			fPasteMenuItem;
			BMenuItem*			fDeleteMenuItem;
			BMenuItem*			fSelectAllMenuItem;
			BMenuItem*			fOverwiteItem;
			BMenuItem*			fToggleWhiteSpacesItem;
			BMenuItem*			fToggleLineEndingsItem;
			BMenu*				fLineEndingsMenu;
			BMenuItem*			fFindItem;
			BMenuItem*			fReplaceItem;
			BMenuItem*			fGoToLineItem;
			BMenu*				fBookmarksMenu;
			BMenuItem*			fBookmarkToggleItem;
			BMenuItem*			fBookmarkClearAllItem;
			BMenuItem*			fBookmarkGoToNextItem;
			BMenuItem*			fBookmarkGoToPreviousItem;
			BMenuItem*			fBuildItem;
			BMenuItem*			fCleanItem;
			BMenuItem*			fRunItem;
			BMenuItem*			fReleaseModeItem;
			BMenuItem*			fDebugModeItem;
			BMenu*				fCargoMenu;
			BMenuItem*			fCargoUpdateItem;
			BMenuItem*			fDebugItem;
			BMenuItem*			fMakeCatkeysItem;
			BMenuItem*			fMakeBindcatalogsItem;
			BMenu*				fGitMenu;
			BMenuItem*			fGitBranchItem;
			BMenuItem*			fGitLogItem;
			BMenuItem*			fGitLogOnelineItem;
			BMenuItem*			fGitPullItem;
			BMenuItem*			fGitPullRebaseItem;
			BMenuItem*			fGitShowConfigItem;
			BMenuItem*			fGitStatusItem;
			BMenuItem*			fGitStatusShortItem;
			BMenuItem*			fGitTagItem;
			BMenu*				fHgMenu;
			BMenuItem*			fHgStatusItem;

			BGroupLayout*		fRootLayout;
			BGroupLayout* 		fToolBar;
			BGroupLayout*		fEditorTabsGroup;

			BIconButton*		fProjectsButton;
			BIconButton*		fOutputButton;
			BIconButton*		fFindButton;
			BIconButton*		fReplaceButton;
			BIconButton*		fFindinFilesButton;
			BIconButton*		fFoldButton;
			BIconButton*		fUndoButton;
			BIconButton*		fRedoButton;
			BIconButton*		fFileSaveButton;
			BIconButton*		fFileSaveAllButton;
			BIconButton*		fBuildButton;
			BIconButton*		fRunButton;
			BIconButton*		fDebugButton;
			BIconButton*		fConsoleButton;
			BIconButton*		fBuildModeButton;
			BIconButton*		fFileUnlockedButton;
			BIconButton*		fFilePreviousButton;
			BIconButton*		fFileNextButton;
			BIconButton*		fFileCloseButton;
			BIconButton*		fFileMenuButton;
			BTextControl*		fGotoLine;

			BIconButton*		fFindPreviousButton;
			BIconButton*		fFindNextButton;
			BIconButton*		fFindMarkAllButton;
			BIconButton*		fReplaceOneButton;
			BIconButton*		fReplaceAndFindNextButton;
			BIconButton*		fReplaceAndFindPrevButton;
			BIconButton*		fReplaceAllButton;

			BTabView*	  		fProjectsTabView;
			BOutlineListView*	fProjectsOutline;
			BScrollView*		fProjectsScroll;

			BPopUpMenu*			fProjectMenu;
			BMenuItem*			fCloseProjectMenuItem;
			BMenuItem*			fDeleteProjectMenuItem;
			BMenuItem*			fSetActiveProjectMenuItem;
			BMenuItem*			fRescanProjectMenuItem
			;
			BMenuItem*			fAddProjectMenuItem;
			BMenuItem*			fExcludeFileProjectMenuItem;
			BMenuItem*			fDeleteFileProjectMenuItem;
			BMenuItem*			fOpenFileProjectMenuItem;

			Project*			fActiveProject;
			bool				fIsBuilding;
			bool				fReleaseModeEnabled;
			BString				fSelectedProjectName;
			BStringItem*		fSelectedProjectItem;
			BString				fSelectedProjectItemName;
		BObjectList<Project>*	fProjectObjectList;
			BStringItem*		fSourcesItem;
			BStringItem*		fFilesItem;

			TabManager*			fTabManager;

			BGroupLayout*		fFindGroup;
			BGroupLayout*		fReplaceGroup;
			BMenuField*			fFindMenuField;
			BMenuField*			fReplaceMenuField;
			BTextControl*		fFindTextControl;
			BTextControl*		fReplaceTextControl;
			BCheckBox*			fFindCaseSensitiveCheck;
			BCheckBox*			fFindWholeWordCheck;
			BCheckBox*			fFindWrapCheck;
			BGroupLayout*		fRunConsoleProgramGroup;
			BTextControl*		fRunConsoleProgramText;
			BButton*			fRunConsoleProgramButton;


		BObjectList<Editor>*	fEditorObjectList;
			Editor*				fEditor;

			BStatusBar*			fStatusBar;
			BFilePanel*			fOpenPanel;
			BFilePanel*			fSavePanel;
			BFilePanel*			fOpenProjectPanel;

			BTabView*			fOutputTabView;
			BColumnListView*	fNotificationsListView;
			ConsoleIOThread*	fConsoleIOThread;
			ConsoleIOView*		fBuildLogView;
			ConsoleIOView*		fConsoleIOView;
			BString				fConsoleStdinLine;

};

#endif //IDEAMWINDOW_H
