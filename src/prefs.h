#ifndef __PREFS_H__
#define __PREFS_H__

#include <stdio.h>
#include <windows.h>
#include <tchar.h>

namespace prefs {
	bool load(char* path);
	bool save();
	bool backup();

	int get(const char* name);
	void set(const char* name, int value);

	char* get(const char* name, const char* def);
	bool set(const char* name, char* value);

	bool setRecent(char* path);
	int getRecents(char** recents);

	bool setQuery(const char* table, const char* query);
	int deleteQuery(const char* table, const char* query);
	int getQueries(const char* table, const char* filter, char** queries);

	bool getDiagramRect(const char* dbname, const char* table, RECT* rect);
	bool setDiagramRect(const char* dbname, const char* table, RECT rect);
}

#endif
