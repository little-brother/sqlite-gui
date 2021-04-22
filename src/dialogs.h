#ifndef __DIALOGS_H__
#define __DIALOGS_H__

#include<windows.h>

namespace dialogs {
	extern const TCHAR* INDENTS[5];

	BOOL CALLBACK cbDlgQueryList (HWND, UINT, WPARAM, LPARAM);
	BOOL CALLBACK cbDlgAddEdit (HWND, UINT, WPARAM, LPARAM);
	BOOL CALLBACK cbDlgAddTable (HWND, UINT, WPARAM, LPARAM);
	BOOL CALLBACK cbDlgEditData (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgRow (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgViewEditDataValue (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgAddColumn (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgSettings (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgEncryption (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgFind (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgDDL (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgChart (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgBindParameters (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	LRESULT CALLBACK cbNewScroll(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}

#endif
