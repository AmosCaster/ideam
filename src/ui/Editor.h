/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef EDITOR_H
#define EDITOR_H

#include <Entry.h>
#include <File.h>
#include <Messenger.h>
#include <ScintillaView.h>
#include <String.h>

enum {
	EDITOR_SAVEPOINT_REACHED	= 'Esre',
	EDITOR_SAVEPOINT_LEFT		= 'Esle',
	EDITOR_SELECTION_CHANGED	= 'Esch',
};

const auto sci_NUMBER_MARGIN = 0;
const auto sci_BOOKMARK_MARGIN = 1;
const auto sci_FOLD_MARGIN = 2;

const auto sci_BOOKMARK = 0;

// Colors
static const auto kWhiteSpaceFore = 0x3030C0;
static const auto kWhiteSpaceBack = 0xB0B0B0;
static const auto kSelectionBackColor = 0x80FFFF;
static const auto kCaretLineBackColor = 0xF8EFE9;
static const auto kEdgeColor = 0xE0E0E0;
static const auto kBookmarkColor = 0x3030C0;




class Editor : public BScintillaView {
public:
								Editor(entry_ref* ref, const BMessenger& target);
								~Editor();
	virtual	void 				MessageReceived(BMessage* message);

			void				ApplySettings();
			void				BookmarkClearAll(int marker);
			bool				BookmarkGoToNext(bool wrap = false
									/*, int marker */);
			bool				BookmarkGoToPrevious(bool wrap = false
									/*, int marker */);
			void				BookmarkToggle(int position);
			bool				CanClear();
			bool				CanCopy();
			bool				CanCut();
			bool				CanPaste();
			bool				CanRedo();
			bool				CanUndo();
			void				Clear();
			void				Copy();
			int32				CountLines();
			void				Cut();
			void				EnsureVisiblePolicy();
		const BString			FilePath() const;
			entry_ref*			FileRef() { return &fFileRef; }
			int32				GetCurrentPosition();
			void				GoToLine(int32 line);
			void				GrabFocus();
			bool				IsModified() { return fModified; }
			bool				IsReadOnly();
			status_t			LoadFromFile();
			BString				Name() const { return fName; }
			node_ref*			NodeRef() { return &fNodeRef; }
			void				NotificationReceived(SCNotification* n);
			void				Paste();
			void				Redo();
			status_t			Reload();
			ssize_t				SaveToFile();
			void				ScrollCaret();
			void				SelectAll();
			void				SendCurrentPosition();
			status_t			SetFileRef(entry_ref* ref);
			void				SetReadOnly();
			status_t			SetSavedCaretPosition();
			void				SetTarget(const BMessenger& target);
			status_t			StartMonitoring();
			status_t			StopMonitoring();
			void				ToggleLineEndings();
			void				ToggleWhiteSpaces();
			void				Undo();

private:

			entry_ref			fFileRef;
			bool				fModified;
			BString				fName;
			node_ref			fNodeRef;
			BMessenger			fTarget;

};

#endif // EDITOR_H
