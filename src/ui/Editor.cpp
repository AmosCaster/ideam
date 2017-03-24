/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Editor.h"

#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
#include <Control.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <Volume.h>

#include <iostream>
#include <sstream>

#include "IdeamNamespace.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Editor"

using namespace IdeamNames;

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

	// Filter notifying changes
/*	SendMessage(SCI_SETMODEVENTMASK,SC_MOD_INSERTTEXT
									 | SC_MOD_DELETETEXT
									 | SC_MOD_CHANGESTYLE
									 | SC_MOD_CHANGEFOLD
									 | SC_PERFORMED_USER
									 | SC_PERFORMED_UNDO
									 | SC_PERFORMED_REDO
									 | SC_MULTISTEPUNDOREDO
									 | SC_LASTSTEPINUNDOREDO
									 | SC_MOD_CHANGEMARKER
									 | SC_MOD_BEFOREINSERT
									 | SC_MOD_BEFOREDELETE
									 | SC_MULTILINEUNDOREDO
									 | SC_MODEVENTMASKALL
									 , UNSET);
*/

//SendMessage(SCI_SETYCARETPOLICY, CARET_SLOP | CARET_STRICT | CARET_JUMPS, 20);
//CARET_SLOP  CARET_STRICT  CARET_JUMPS  CARET_EVEN
}

Editor::~Editor()
{
	// Stop monitoring
	StopMonitoring();

	// Set caret position
	if (Settings.save_caret == true) {
		BNode node(&fFileRef);
		if (node.InitCheck() == B_OK) {
			int32 pos = GetCurrentPosition();
			node.WriteAttr("be:caret_position", B_INT32_TYPE, 0, &pos, sizeof(pos));
		}
	}
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

void
Editor::ApplySettings()
{
	// White spaces color
	SendMessage(SCI_SETWHITESPACESIZE, 4, UNSET);
	SendMessage(SCI_SETWHITESPACEFORE, 1, kWhiteSpaceFore);
	SendMessage(SCI_SETWHITESPACEBACK, 1, kWhiteSpaceBack);

	// Selection background
	SendMessage(SCI_SETSELBACK, 1, kSelectionBackColor);

	// Font & Size
	SendMessage(SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t) "Noto Mono");
	SendMessage(SCI_STYLESETSIZE, STYLE_DEFAULT, Settings.edit_fontsize);
	SendMessage(SCI_STYLECLEARALL, UNSET, UNSET);

	// Caret line visible
	if (Settings.mark_caretline == true) {
		SendMessage(SCI_SETCARETLINEVISIBLE, 1, UNSET);
		SendMessage(SCI_SETCARETLINEBACK, kCaretLineBackColor, UNSET);
	}

	// Edge line
	if (Settings.show_edgeline == true) {
		SendMessage(SCI_SETEDGEMODE, EDGE_LINE, UNSET);

		std::string column(Settings.edgeline_column);
		int32 col;
		std::istringstream (column) >>  col;
		SendMessage(SCI_SETEDGECOLUMN, col, UNSET);
		SendMessage(SCI_SETEDGECOLOUR, kEdgeColor, UNSET);
	}

	// Tab width
	SendMessage(SCI_SETTABWIDTH, Settings.tab_width, UNSET);

	// MARGINS
	SendMessage(SCI_SETMARGINS, 3, UNSET);
	// Line numbers
	if (Settings.show_linenumber == true) {

		int32 pixW = SendMessage(SCI_TEXTWIDTH, STYLE_LINENUMBER, (sptr_t) "_123456");

		SendMessage(SCI_SETMARGINWIDTHN, sci_NUMBER_MARGIN, pixW);
		SendMessage(SCI_STYLESETBACK, STYLE_LINENUMBER, 0xD3D3D3);
	}

	// Bookmark margin
	SendMessage(SCI_SETMARGINTYPEN, sci_BOOKMARK_MARGIN, SC_MARGIN_SYMBOL);
	SendMessage(SCI_SETMARGINSENSITIVEN, sci_BOOKMARK_MARGIN, 1);
	SendMessage(SCI_MARKERDEFINE, sci_BOOKMARK, SC_MARK_BOOKMARK);
//	SendMessage(SCI_MARKERSETFORE, sci_BOOKMARK, 0x3030C0);
	SendMessage(SCI_MARKERSETBACK, sci_BOOKMARK, kBookmarkColor);

}

void
Editor::BookmarkClearAll(int marker)
{
	SendMessage(SCI_MARKERDELETEALL, marker, UNSET);
}

bool
Editor::BookmarkGoToNext(bool wrap /*= false*/)
{
	int32 position = GetCurrentPosition();

	int lineToGoTo;
	int line = SendMessage(SCI_LINEFROMPOSITION, position, UNSET);

	line += 1;
	lineToGoTo = SendMessage(SCI_MARKERNEXT, line, 1 << sci_BOOKMARK);


	if (lineToGoTo == -1) {
			if (wrap == false)
				return false;
			else {
				lineToGoTo = SendMessage(SCI_MARKERNEXT, 0, 1 << sci_BOOKMARK);
				if (lineToGoTo == -1)
					return false;
		}
	}

	SendMessage(SCI_ENSUREVISIBLEENFORCEPOLICY, lineToGoTo, UNSET);
	SendMessage(SCI_GOTOLINE, lineToGoTo, UNSET);

	return true;
}

bool
Editor::BookmarkGoToPrevious(bool wrap)
{
	int32 position = GetCurrentPosition();

	int lineToGoTo;
	int line = SendMessage(SCI_LINEFROMPOSITION, position, UNSET);

	line -= 1;
	lineToGoTo = SendMessage(SCI_MARKERPREVIOUS, line, 1 << sci_BOOKMARK);

	if (lineToGoTo == -1) {
			if (wrap == false)
				return false;
			else {
				int32 line = SendMessage(SCI_GETLINECOUNT, UNSET, UNSET);
				lineToGoTo = SendMessage(SCI_MARKERPREVIOUS, line, 1 << sci_BOOKMARK);
				if (lineToGoTo == -1)
					return false;
		}
	}

	SendMessage(SCI_ENSUREVISIBLEENFORCEPOLICY, lineToGoTo, UNSET);
	SendMessage(SCI_GOTOLINE, lineToGoTo, UNSET);

	return true;
}

void
Editor::BookmarkToggle(int position)
{
	int line = SendMessage(SCI_LINEFROMPOSITION, position, UNSET);
	int markerSet = SendMessage(SCI_MARKERGET, line, UNSET);

	if ((markerSet & (1 << sci_BOOKMARK)) != 0)
		SendMessage(SCI_MARKERDELETE, line, sci_BOOKMARK);
	else
		SendMessage(SCI_MARKERADD, line, sci_BOOKMARK);
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

int32
Editor::CountLines()
{
	return SendMessage(SCI_GETLINECOUNT, UNSET, UNSET);
}

void
Editor::Cut()
{
	SendMessage(SCI_CUT, UNSET, UNSET);
}

void
Editor::EnsureVisiblePolicy()
{
	SendMessage(SCI_ENSUREVISIBLEENFORCEPOLICY,
		SendMessage(SCI_LINEFROMPOSITION, GetCurrentPosition(), UNSET), UNSET);
}

const BString
Editor::FilePath() const
{
	BPath path(&fFileRef);

	return path.Path();
}

int32
Editor::GetCurrentPosition()
{
	return SendMessage(SCI_GETCURRENTPOS, UNSET, UNSET);
}

/*
 * Mind that first line is 0!
 */
void
Editor::GoToLine(int32 line)
{
	// Do not go to line 0
	if (line == 0)
		return;

	line -= 1;
	SendMessage(SCI_ENSUREVISIBLEENFORCEPOLICY, line, UNSET);
	SendMessage(SCI_GOTOLINE, line, UNSET);
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
		// Bookmark toggle
		case SCN_MARGINCLICK: {
			if (notification->margin == sci_BOOKMARK_MARGIN)
				BookmarkToggle(notification->position);
			break;
		}
		case SCN_NEEDSHOWN: {
std::cerr << "SCN_NEEDSHOWN " << std::endl;
		break;
		}
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

			// Selection/Position has changed
			if (notification->updated & SC_UPDATE_SELECTION) {

				// Ugly hack to enable mouse selection scrolling
				// in both directions
				int32 position = SendMessage(SCI_GETCURRENTPOS, UNSET, UNSET);
				int32 anchor = SendMessage(SCI_GETANCHOR, UNSET, UNSET);
				if (anchor != position) {
					int32 line = SendMessage(SCI_LINEFROMPOSITION, position, UNSET);
					if (line == SendMessage(SCI_GETFIRSTVISIBLELINE, UNSET, UNSET))
						SendMessage(SCI_SETFIRSTVISIBLELINE, line - 1, UNSET);
					else
						SendMessage(SCI_SCROLLCARET, UNSET, UNSET);
				}

				// Send position to main window so it can update status bar
				SendCurrentPosition();
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
Editor::ScrollCaret()
{
	SendMessage(SCI_SCROLLCARET, UNSET, UNSET);
}

void
Editor::SelectAll()
{
	SendMessage(SCI_SELECTALL, UNSET, UNSET);
}

/*
 * Name is misleading: it sends Selection/Position changes
 */
void
Editor::SendCurrentPosition()
{
	int32 position = GetCurrentPosition();

	BMessage message(EDITOR_SELECTION_CHANGED);
	message.AddRef("ref", &fFileRef);
	int line = SendMessage(SCI_LINEFROMPOSITION, position, UNSET) + 1;
	int column = SendMessage(SCI_GETCOLUMN, position, UNSET) + 1;
	message.AddInt32("line", line);
	message.AddInt32("column", column);
	fTarget.SendMessage(&message);
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

status_t
Editor::SetSavedCaretPosition()
{
	if (Settings.save_caret == false)
		return B_ERROR; //TODO maybe tweak

	status_t status;
	// Get caret position
	BNode node(&fFileRef);
	if ((status = node.InitCheck()) != B_OK)
		return status;
	int32 pos = 0;
	ssize_t bytes = 0;
	bytes = node.ReadAttr("be:caret_position", B_INT32_TYPE, 0, &pos, sizeof(pos));

	if (bytes < (int32) sizeof(pos))
		return B_ERROR; //TODO maybe tweak + cast


	SendMessage(SCI_ENSUREVISIBLEENFORCEPOLICY,
			SendMessage(SCI_LINEFROMPOSITION, pos, UNSET), UNSET);

	SendMessage(SCI_GOTOPOS, pos, UNSET);

	return B_OK;
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
Editor::ToggleLineEndings()
{
	if (SendMessage(SCI_GETVIEWEOL, UNSET, UNSET) == false)
		SendMessage(SCI_SETVIEWEOL, 1, UNSET);
	else
		SendMessage(SCI_SETVIEWEOL, 0, UNSET);
}

void
Editor::ToggleWhiteSpaces()
{
	if (SendMessage(SCI_GETVIEWWS, UNSET, UNSET) == SCWS_INVISIBLE)
		SendMessage(SCI_SETVIEWWS, SCWS_VISIBLEALWAYS, UNSET);
	else
		SendMessage(SCI_SETVIEWWS, SCWS_INVISIBLE, UNSET);
}

void
Editor::Undo()
{
	SendMessage(SCI_UNDO, UNSET, UNSET);
}
