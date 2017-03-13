/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Editor.h"

#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <Volume.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Editor"

// Differentiate unset parameters from 0 ones
// in scintilla messages
#define UNSET 0

Editor::Editor(entry_ref* ref, const BMessenger& target)
	:
	BScintillaView(ref->name, 0, true, true, B_NO_BORDER)
	, fFileRef(*ref)
	, fModified(false)
{
	fName = BString(ref->name);
	SetTarget(target);
}

Editor::~Editor()
{
	// Stop monitoring
	StopMonitoring();
}

void
Editor::MessageReceived(BMessage* message)
{
	switch (message->what) {

		default:
			BScintillaView::MessageReceived(message);
			break;
	}
}

bool
Editor::CanClear()
{
	return ((SendMessage(SCI_GETSELECTIONEMPTY, UNSET, UNSET) == 0) &&
				!IsReadOnly());
}

bool
Editor::CanCopy()
{
	return (SendMessage(SCI_GETSELECTIONEMPTY, UNSET, UNSET) == 0);
}
bool
Editor::CanCut()
{
	return ((SendMessage(SCI_GETSELECTIONEMPTY, UNSET, UNSET) == 0) &&
				!IsReadOnly());
}

bool
Editor::CanPaste()
{
	return SendMessage(SCI_CANPASTE, UNSET, UNSET);
}

bool
Editor::CanRedo()
{
	return SendMessage(SCI_CANREDO, UNSET, UNSET);
}

bool
Editor::CanUndo()
{
	return SendMessage(SCI_CANUNDO, UNSET, UNSET);
}

void
Editor::Clear()
{
	SendMessage(SCI_CLEAR, UNSET, UNSET);
}

void
Editor::Copy()
{
	SendMessage(SCI_COPY, UNSET, UNSET);
}

void
Editor::Cut()
{
	SendMessage(SCI_CUT, UNSET, UNSET);
}

const BString
Editor::FilePath() const
{
	BPath path(&fFileRef);

	return path.Path();
}

void
Editor::GrabFocus()
{
	SendMessage(SCI_GRABFOCUS, UNSET, UNSET);
}

bool
Editor::IsReadOnly()
{
	return SendMessage(SCI_GETREADONLY, UNSET, UNSET);
}

/*
 * Code (editable) taken from stylededit
 */
status_t
Editor::LoadFromFile()
{
	status_t status;
	BFile file;
	struct stat st;

	if ((status = file.SetTo(&fFileRef, B_READ_ONLY)) != B_OK)
		return status;
	if ((status = file.InitCheck()) != B_OK)
		return status;
	if ((status = file.Lock()) != B_OK)
		return status;
	if ((status = file.GetStat(&st)) != B_OK)
		return status;

	bool editable = (getuid() == st.st_uid && S_IWUSR & st.st_mode)
					|| (getgid() == st.st_gid && S_IWGRP & st.st_mode)
					|| (S_IWOTH & st.st_mode);
	BVolume volume(fFileRef.device);
	editable = editable && !volume.IsReadOnly();

	off_t size;
	file.GetSize(&size);

	char* buffer = new char[size + 1];

	off_t len = file.Read(buffer, size);

	buffer[size] = '\0';

	SendMessage(SCI_SETTEXT, 0, (sptr_t) buffer);
	delete[] buffer;

	if (len != size)
		return B_ERROR;

	if ((status = file.Unlock()) != B_OK)
		return status;

	SendMessage(SCI_EMPTYUNDOBUFFER, UNSET, UNSET);
	SendMessage(SCI_SETSAVEPOINT, UNSET, UNSET);

	if (editable == false)
		SetReadOnly();

	// Monitor node
	StartMonitoring();

	return B_OK;
}

void
Editor::NotificationReceived(SCNotification* notification)
{
	Sci_NotifyHeader* pNmhdr = &notification->nmhdr;

	switch (pNmhdr->code) {
		case SCN_SAVEPOINTLEFT: {
			fModified = true;
			BMessage message(EDITOR_SAVEPOINT_LEFT);
			message.AddRef("ref", &fFileRef);
			fTarget.SendMessage(&message);
			break;
		}
		case SCN_SAVEPOINTREACHED: {
			fModified = false;
			BMessage message(EDITOR_SAVEPOINT_REACHED);
			message.AddRef("ref", &fFileRef);
			fTarget.SendMessage(&message);	
			break;
		}
		case SCN_UPDATEUI: {
			// Selection has changed
			if (notification->updated & SC_UPDATE_SELECTION) {
				BMessage message(EDITOR_SELECTION_CHANGED);
				message.AddRef("ref", &fFileRef);
				fTarget.SendMessage(&message);
			}
			break;
		}
	}
}

void
Editor::Paste()
{
	if (SendMessage(SCI_CANPASTE, UNSET, UNSET))
		SendMessage(SCI_PASTE, UNSET, UNSET);
}

void
Editor::Redo()
{
	SendMessage(SCI_REDO, UNSET, UNSET);
}

status_t
Editor::Reload()
{
	status_t status;
	BFile file;

	//TODO errors should be notified
	if ((status = file.SetTo(&fFileRef, B_READ_ONLY)) != B_OK)
		return status;
	if ((status = file.InitCheck()) != B_OK)
		return status;

	if ((status = file.Lock()) != B_OK)
		return status;

	// Enable external modifications of readonly file/buffer
	bool readOnly = IsReadOnly();

	if (readOnly == true)
		SendMessage(SCI_SETREADONLY, 0, UNSET);

	off_t size;
	file.GetSize(&size);

	char* buffer = new char[size + 1];
	off_t len = file.Read(buffer, size);
	buffer[size] = '\0';
	SendMessage(SCI_CLEARALL, UNSET, UNSET);
	SendMessage(SCI_SETTEXT, 0, (sptr_t) buffer);
	delete[] buffer;

	if (readOnly == true)
		SendMessage(SCI_SETREADONLY, 1, UNSET);

	if (len != size)
		return B_ERROR;

	if ((status = file.Unlock()) != B_OK)
		return status;

	SendMessage(SCI_EMPTYUNDOBUFFER, UNSET, UNSET);
	SendMessage(SCI_SETSAVEPOINT, UNSET, UNSET);

	return B_OK;
}

ssize_t
Editor::SaveToFile()
{
	BFile file;
	status_t status;

	status = file.SetTo(&fFileRef, B_READ_WRITE | B_ERASE_FILE | B_CREATE_FILE);
	if (status != B_OK)
		return 0;

	// TODO warn user
	if ((status = file.Lock()) != B_OK)
		return 0;

	off_t size = SendMessage(SCI_GETLENGTH, UNSET, UNSET);
	file.Seek(0, SEEK_SET);
	char* buffer = new char[size + 1];

	SendMessage(SCI_GETTEXT, size + 1, (sptr_t)buffer);

	off_t bytes = file.Write(buffer, size);
	file.Flush();

	// TODO warn user
	if ((status = file.Unlock()) != B_OK)
		return 0;

	delete[] buffer;

	SendMessage(SCI_SETSAVEPOINT, UNSET, UNSET);

	return bytes;
}

void
Editor::SelectAll()
{
	SendMessage(SCI_SELECTALL, UNSET, UNSET);
}

status_t
Editor::SetFileRef(entry_ref* ref)
{
	if (ref == nullptr)
		return B_ERROR;

	fFileRef = *ref;
	fName = BString(fFileRef.name);

	return B_OK;
}

void
Editor::SetReadOnly()
{
	if (IsModified()) {
		BString text(B_TRANSLATE("Save changes to file"));
		text << " \"" << Name() << "\" ?";
		
		BAlert* alert = new BAlert(B_TRANSLATE("Save dialog"), text,
 			B_TRANSLATE("Cancel"), B_TRANSLATE("Don't save"), B_TRANSLATE("Save"),
 			B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
   			 
		alert->SetShortcut(0, B_ESCAPE);
		
		int32 choice = alert->Go();

		if (choice == 0)
			return ;
		else if (choice == 2) {
			SaveToFile();
		}
	}

	fModified = false;

	SendMessage(SCI_SETREADONLY, 1, UNSET);
}

void
Editor::SetTarget(const BMessenger& target)
{
    fTarget = target;
}

status_t
Editor::StartMonitoring()
{
	status_t status;

	// start monitoring this file for changes
	BEntry entry(&fFileRef, true);

	if ((status = entry.GetNodeRef(&fNodeRef)) != B_OK)
		return status;

	return	watch_node(&fNodeRef, B_WATCH_NAME | B_WATCH_STAT, fTarget);
}

status_t
Editor::StopMonitoring()
{
	return watch_node(&fNodeRef, B_STOP_WATCHING, fTarget);
}

void
Editor::Undo()
{
	SendMessage(SCI_UNDO, UNSET, UNSET);
}

