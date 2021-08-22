#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <windows.h>

namespace tools {
	extern const TCHAR* DELIMITERS[4];

	BOOL CALLBACK cbDlgImportCSV (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgImportJSON (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExportCSV (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExportJSON (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExportExcel (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExportSQL (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgExportImportODBC (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgDataGenerator (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgDatabaseDiagram (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgCompareDatabase (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgDatabaseSearch (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgStatistics (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CALLBACK cbDlgForeignKeyCheck (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool exportCSV(TCHAR* path16, TCHAR* query16);
	bool exportExcel(TCHAR* path16, TCHAR* query16);
	bool importSqlFile(TCHAR* path16);
	bool reindexDatabase();
}
#endif
