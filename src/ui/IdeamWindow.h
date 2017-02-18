/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef IDEAMWINDOW_H
#define IDEAMWINDOW_H

#include <Bitmap.h>
#include <ColumnListView.h>
#include <ColumnTypes.h>
#include <GroupLayout.h>
#include <IconButton.h>
#include <OutlineListView.h>
#include <ScrollView.h>
#include <TabView.h>
#include <TextView.h>
#include <Window.h>

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
			BIconButton*		_LoadIconButton(const char* name, int32 msg,
									int32 resIndex, bool enabled, const char* tooltip);
			BBitmap*			_LoadSizedVectorIcon(int32 resourceID, int32 size);

			BGroupLayout*		fRootLayout;

			BIconButton*		fProjectsButton;
			BIconButton*		fOutputButton;

			BTabView*	  		fProjectsTabView;
			BOutlineListView*	fProjectsOutline;
			BScrollView*		fProjectsScroll;

			BTabView*			fOutputTabView;
			BColumnListView*	fNotificationsListView;
			BTextView*			fNotificationText;

};

#endif //IDEAMWINDOW_H
