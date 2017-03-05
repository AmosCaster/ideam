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

class Editor : public BScintillaView {
public:
								Editor(entry_ref* ref, const BMessenger& target);
								~Editor();
	virtual	void 				MessageReceived(BMessage* message);

			bool				CanClear();
			bool				CanCopy();
			bool				CanCut();
			bool				CanPaste();
			bool				CanRedo();
			bool				CanUndo();
			void				Clear();
			void				Copy();
			void				Cut();
			entry_ref*			FileRef() { return &fFileRef; }
			void				GrabFocus();
			bool				IsModified() { return fModified; }
			bool				IsReadOnly();
			status_t			LoadFromFile();
			BString				Name() const { return fName; }
			void				NotificationReceived(SCNotification* n);
			void				Paste();
			void				Redo();
			ssize_t				SaveToFile();
			void				SelectAll();
			status_t			SetFileRef(entry_ref* ref);
			void				SetReadOnly();
			void				SetTarget(const BMessenger& target);
			void				Undo();
//	virtual void				WindowActivated(bool active);
private:

			entry_ref			fFileRef;
			bool				fModified;
			BString				fName;
			BMessenger			fTarget;

};

#endif // EDITOR_H
