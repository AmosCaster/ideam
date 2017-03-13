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
	const BString kUISettingsFileName("ui.settings");

	int32 CompareVersion(const BString appVersion, const BString fileVersion);
	BString GetSignature();
	BString GetVersionInfo();
	status_t UpdateSettingsFile();



}

#endif // IDEAM_NAMESPACE_H
