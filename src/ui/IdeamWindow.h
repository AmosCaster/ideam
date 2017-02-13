/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef IDEAMWINDOW_H
#define IDEAMWINDOW_H

#include <GroupLayout.h>
#include <Window.h>

class IdeamWindow : public BWindow
{
public:
								IdeamWindow(BRect frame);
	virtual						~IdeamWindow();

	virtual void				MessageReceived(BMessage* message);

private:
			BGroupLayout*		fRootLayout;
};

#endif //IDEAMWINDOW_H
