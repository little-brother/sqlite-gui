#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <windows.h>

namespace tools {
	BOOL CALLBACK cbDlgImportCSV (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExportCSV (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExportSQL (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void importSqlFile(TCHAR* path16);
}
#endif
