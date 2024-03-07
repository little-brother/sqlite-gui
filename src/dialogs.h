#ifndef __DIALOGS_H__
#define __DIALOGS_H__

#include<windows.h>

namespace dialogs {
	extern const TCHAR* INDENTS[5];

	BOOL CALLBACK cbDlgQueryList (HWND, UINT, WPARAM, LPARAM);
	BOOL CALLBACK cbDlgAddViewEdit (HWND, UINT, WPARAM, LPARAM);
	BOOL CALLBACK cbDlgAddTable (HWND, UINT, WPARAM, LPARAM);
	BOOL CALLBACK cbDlgEditData (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgRow (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgViewEditDataValue (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgAddColumn (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgAddIndex (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgSettings (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgEncryption (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgFind (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgChart (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgBindParameters (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgDrop (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgTableName (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgResultsComparison (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgShortcuts (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgAttachODBC (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExtensions (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgCustomFunctions (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgResultFind (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgUriDbPath (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgInfo (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	LRESULT CALLBACK cbNewScroll(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool getAttachString(const char* path8, const char* alias8, const char* key8, char* result8);
}

#endif
