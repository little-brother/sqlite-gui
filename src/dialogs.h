#ifndef __DIALOGS_H__
#define __DIALOGS_H__

#include<windows.h>

namespace dialogs {
	BOOL CALLBACK cbDlgQueryList (HWND, UINT, WPARAM, LPARAM);
	BOOL CALLBACK cbDlgAdd (HWND, UINT, WPARAM, LPARAM);
	BOOL CALLBACK cbDlgEdit (HWND, UINT, WPARAM, LPARAM);
	BOOL CALLBACK cbDlgEditData (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgRow (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgAddColumn (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgSettings (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}

#endif
