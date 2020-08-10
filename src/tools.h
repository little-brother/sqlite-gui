#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <windows.h>

namespace tools {
	extern const TCHAR* DELIMITERS[5];

	BOOL CALLBACK cbDlgImportCSV (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExportCSV (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExportSQL (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgDataGenerator (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void importSqlFile(TCHAR* path16);
}
#endif
