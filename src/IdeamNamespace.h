/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef IDEAM_NAMESPACE_H
#define IDEAM_NAMESPACE_H

#include <Catalog.h>
#include <String.h>

namespace IdeamNames
{
	const BString kApplicationName(B_TRANSLATE_SYSTEM_NAME("Ideam"));
	const BString kApplicationSignature("application/x-vnd.Ideam-Ideam");
	const BString kSettingsFileName("ideam.settings");
	const BString kSettingsFilesToReopen("files_to_reopen.settings");
	const BString kSettingsProjectsToReopen("projects_to_reopen.settings");
	const BString kUISettingsFileName("ui.settings");
	BString const kProjectExtension(".idmpro");

	int32 CompareVersion(const BString appVersion, const BString fileVersion);
	BString GetSignature();
	BString GetVersionInfo();
	status_t LoadSettingsVars();
	status_t UpdateSettingsFile();

	typedef struct {
		BString projects_directory;
		int fullpath_title;
		int reopen_projects;
		int reopen_files;
		int show_projects;
		int show_output;
		int show_toolbar;
		int edit_fontsize;
		int syntax_highlight;
		int tab_width;
		int brace_match;
		int save_caret;
		int show_linenumber;
		int mark_caretline;
		int show_edgeline;
		BString edgeline_column;
		int enable_folding;
		int enable_notifications;

	} SettingsVars;

	extern SettingsVars Settings;

}

#endif // IDEAM_NAMESPACE_H
