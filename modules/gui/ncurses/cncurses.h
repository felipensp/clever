/**
 * Clever programming language
 * Copyright (c) Clever Team
 *
 * This file is distributed under the MIT license. See LICENSE for details.
 */

#ifndef CLEVER_GUI_CNCURSES_H
#define CLEVER_GUI_CNCURSES_H

#include <iostream>

#include <curses.h>

#include "core/cstring.h"
#include "core/value.h"
#include "core/type.h"
#include "modules/std/core/map.h"


namespace clever {


class CNCurses {
public:
	CNCurses(int m_sleep_time = 0, WINDOW* father = NULL, bool m_enable_colors = false,
			 int w = 0, int h = 0, int x = 0, int y = 0, bool m_enable_keypad = false);
	~CNCurses();

	void close();
	void hide();
	void exit();

	bool status();
	bool hasColors();
	void startColor() { ::start_color(); }
	static bool isPrintable(int ch);
	bool isChild();

	void noEcho() { ::noecho(); }

	int nColors();
	int setColor(short id, void* handler);
	int getKey();

	void box(int x, int y);

	void move(int x, int y);

	void enableKeyPad();
	void deleteLine();
	void addColor(short id, short color_1, short color_2);
	void addStr(int x, int y, const char* str);
	void addChar(int x, int y, int v);
	void printStr(int x, int y, const char* str);
	void refresh();
	void sleep();

	WINDOW* getWindow() { return m_win; }

	int getPosX() { return pos_x; }
	int getPosY() { return pos_y; }

	int getWidth() { return width; }
	int getHeight() { return height; }

private:
	bool m_is_closed;
	int m_sleep_time;

	bool m_enable_colors;

	WINDOW* m_win;
	WINDOW* m_father;

	int width, height;
	int pos_x, pos_y;

	bool m_status;


	DISALLOW_COPY_AND_ASSIGN(CNCurses);
};

} // clever

#endif // CLEVER_GUI_CNCURSES_H
