#ifndef __PREFS_H__
#define __PREFS_H__

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include "sqlite3.h"

namespace prefs {
	extern sqlite3* db;

	bool load(char* path);
	bool save();
	bool backup();
	bool setSyncMode(int mode);

	int get(const char* name);
	bool set(const char* name, int value);

	char* get(const char* name, const char* def);
	bool set(const char* name, const char* value);
}

#endif
