#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <windows.h>

namespace tools {
	extern const TCHAR* DELIMITERS[4];

	BOOL CALLBACK cbDlgImportCSV (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExportCSV (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExportSQL (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgDataGenerator (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgDatabaseDiagram (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool importSqlFile(TCHAR* path16);
}
#endif
