/*
 * Copyright 2017 A. Mosca <amoscaster@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef EDITOR_H
#define EDITOR_H

#include <Entry.h>
#include <ScintillaView.h>

class Editor : public BScintillaView {
public:
								Editor(entry_ref* ref, const BMessenger& target);
								~Editor();
	virtual	void 				MessageReceived(BMessage* message);

};

#endif // EDITOR_H
