/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef NEW_PROJECT_WINDOW_H
#define NEW_PROJECT_WINDOW_H

#include <Box.h>
#include <CheckBox.h>
#include <FilePanel.h>
#include <ListView.h>
#include <map>
#include <String.h>
#include <TextControl.h>
#include <TextView.h>
#include <Window.h>

#include "TitleItem.h"
#include "TPreferences.h"


enum {
	// Project context menu
	NEWPROJECTWINDOW_PROJECT_CARGO_NEW	= 'Npcn',
	NEWPROJECTWINDOW_PROJECT_OPEN_NEW	= 'Npon',
};

typedef std::map<BStringItem*, BString const> ProjectTypeMap;
typedef std::pair<BStringItem*, BString const> ProjectTypePair;
typedef std::map<BStringItem*, BString const>::const_iterator ProjectTypeIterator;

class NewProjectWindow : public BWindow
{
public:
								NewProjectWindow();
								~NewProjectWindow();

	virtual	bool				QuitRequested();
			void 				MessageReceived(BMessage *msg);

private:
			status_t			_CreateProject();
			status_t			_CreateAppProject();
			status_t			_CreateAppMenuProject();
			status_t			_CreateCargoProject();
			status_t			_CreateHelloCplusProject();
			status_t			_CreateHelloCProject();
			status_t			_CreatePrinciplesProject();
			status_t			_CreateEmptyProject();
			status_t			_CreateHaikuSourcesProject();
			status_t			_CreateLocalSourcesProject();
			status_t			_CreateSkeleton();
			status_t			_GetSourcesFiles(const char* dirpath);
			void				_MapItems();
			void				_OnEditingHaikuAppText();
			void				_OnEditingLocalAppText();
			void				_RemoveStaleEntries(const char* dirpath);
			bool				_FindMakefile(BString& target);
			bool				_ParseMakefile(BString& target, const BEntry* entry);
			void				_UpdateControlsState(int32 selection);
			void				_UpdateControlsData(int32 selection);
			status_t			_WriteMakefile();
			status_t			_WriteAppfiles();
			status_t			_WriteAppMenufiles();
			status_t			_WriteHelloCplusfile();
			status_t			_WriteHelloCfile();
			status_t			_WritePrinciplesfile();
			status_t			_WriteHelloCMakefile();

private:
			BOutlineListView*	fTypeListView;
			BScrollView*		typeListScrollView;

			TPreferences*		fProjectFile;
			BString				fProjectExtensionedName;

			TitleItem*			haikuItem;
			BStringItem*		appItem;
			BStringItem* 		appMenuItem;
/*
			BStringItem* 		appLayoutItem;
			BStringItem*		sharedItem;
			BStringItem*		staticItem;
			BStringItem*		driverItem;
			BStringItem*		trackerItem;
*/
			TitleItem*			genericItem;
			BStringItem*		helloCplusItem;
			BStringItem*		helloCItem;
			BStringItem*		principlesItem;
			BStringItem*		emptyItem;
			TitleItem*			importItem;
			BStringItem*		sourcesItem;
			BStringItem*		existingItem;

			TitleItem*			rustItem;
			BStringItem*		cargoItem;

			BTextView*			fProjectDescription;
			BScrollView*		fScrollText;

			BBox*				fProjectBox;
			BTextControl*		fProjectNameText;
			BTextControl*		fProjectTargetTC;
			BCheckBox*			fRunInTeminal;
			BTextControl*		fProjectsDirectoryText;
			BTextControl*		fAddFileTC;
			BCheckBox*			fAddHeader;
			BTextControl*		fAddSecondFileTC;
			BCheckBox*			fAddSecondHeader;
			BTextControl*		fHaikuAppDirTC;
			BButton*			fBrowseHaikuAppButton;
			BFilePanel*			fOpenPanel;
			BTextControl*		fLocalAppDirTC;
			BButton*			fBrowseLocalAppButton;
			BTextControl*		fCargoPathText;
			BCheckBox*			fCargoBin;
			BCheckBox*			fCargoVcs;

			BButton*			fCreateButton;
			BButton*			fCancelButton;

			BStringItem*		fCurrentItem;
			int32				fYear;

};

#endif // NEW_PROJECT_WINDOW_H
