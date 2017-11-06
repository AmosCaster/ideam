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
	EDITOR_BOOKMARK_MARK		= 'Ebma',
	EDITOR_REPLACED_ONE			= 'Ereo',
	EDITOR_SAVEPOINT_REACHED	= 'Esre',
	EDITOR_SAVEPOINT_LEFT		= 'Esle',
	EDITOR_SELECTION_CHANGED	= 'Esch',
};

/*
 * Not very smart: NONE,SKIP,DONE is Status
 * while the other are Function placeholders
 *
 */
enum {
	REPLACE_NONE = -1,
	REPLACE_SKIP = 0,
	REPLACE_DONE = 1,
	REPLACE_ONE,
	REPLACE_NEXT,
	REPLACE_PREVIOUS,
	REPLACE_ALL
};

constexpr auto sci_NUMBER_MARGIN = 0;
constexpr auto sci_BOOKMARK_MARGIN = 1;
constexpr auto sci_FOLD_MARGIN = 2;

constexpr auto sci_BOOKMARK = 0;

// Colors
static constexpr auto kLineNumberBack = 0xD3D3D3;
static constexpr auto kWhiteSpaceFore = 0x3030C0;
static constexpr auto kWhiteSpaceBack = 0xB0B0B0;
static constexpr auto kSelectionBackColor = 0x80FFFF;
static constexpr auto kCaretLineBackColor = 0xF8EFE9;
static constexpr auto kEdgeColor = 0xE0E0E0;
static constexpr auto kBookmarkColor = 0x3030C0;

constexpr auto kNoBrace = 0;
constexpr auto kBraceMatch = 1;
constexpr auto kBraceBad = 2;


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
			int					Find(const BString&  text, int flags, bool backwards = false);
			int					FindInTarget(const BString& search, int flags, int startPosition, int endPosition);
			int32				FindMarkAll(const BString& text, int flags);
			int					FindNext(const BString& search, int flags, bool wrap);
			int					FindPrevious(const BString& search, int flags, bool wrap);
			int32				GetCurrentPosition();
			void				GoToLine(int32 line);
			void				GrabFocus();
			bool				IsFoldingAvailable() { return fFoldingAvailable; }
			bool				IsModified() { return fModified; }
			bool				IsOverwrite();
			bool				IsReadOnly();
			bool				IsSearchSelected(const BString& search, int flags);
			bool				IsTextSelected();
			status_t			LoadFromFile();
			BString				Name() const { return fName; }
			node_ref*			NodeRef() { return &fNodeRef; }
			void				NotificationReceived(SCNotification* n);
			void				OverwriteToggle();
			void				Paste();
			void				Redo();
			status_t			Reload();
			int					ReplaceAndFindNext(const BString& selection,
									const BString& replacement, int flags, bool wrap);
			int					ReplaceAll(const BString& selection,
									const BString& replacement, int flags);
			void 				ReplaceMessage(int position, const BString& selection,
									const BString& replacement);
			int					ReplaceOne(const BString& selection,
									const BString& replacement);
			ssize_t				SaveToFile();
			void				ScrollCaret();
			void				SelectAll();
	const 	BString				Selection();
			void				SendCurrentPosition();
			status_t			SetFileRef(entry_ref* ref);
			void				SetReadOnly();
			status_t			SetSavedCaretPosition();
			int					SetSearchFlags(bool matchCase, bool wholeWord,
									bool wordStart,	bool regExp, bool posix);
			void				SetTarget(const BMessenger& target);
			status_t			StartMonitoring();
			status_t			StopMonitoring();
			void				ToggleFolding();
			void				ToggleLineEndings();
			void				ToggleWhiteSpaces();
			void				Undo();

private:
			void				_ApplyExtensionSettings();
			void				_AutoIndentLine();
			void				_CheckForBraceMatching();
			void				_FoldFile();
			BString	const		_GetFileExtension();
			void				_HighlightBraces();
			void				_HighlightFile();
			bool				_IsBrace(char character);

private:

			entry_ref			fFileRef;
			bool				fModified;
			BString				fName;
			node_ref			fNodeRef;
			BMessenger			fTarget;

			int32				fBraceHighlighted;
			bool				fBracingAvailable;
			BString				fExtension;
			bool				fFoldingAvailable;
			bool				fSyntaxAvailable;
};

#endif // EDITOR_H
