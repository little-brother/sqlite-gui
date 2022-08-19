#define GUI_VERSION                "1.7.7"
#define GUI_VERSION2               1, 7, 7, 0
#ifdef __MINGW64__
#define GUI_PLATFORM               64
#else
#define GUI_PLATFORM               32
#endif
#define HELP_VERSION               3

#define IDD_ADDVIEWEDIT            11
#define IDD_EDITDATA               12
#define IDD_QUERYLIST              13
#define IDD_ROW                    14
#define IDD_ADD_COLUMN             15
#define IDD_SETTINGS               16
#define IDD_ADD_TABLE              17
#define IDD_FIND                   18
#define IDD_DDL                    19
#define IDD_CHART                  20
#define IDD_EDITDATA_VALUE         21
#define IDD_VIEWDATA_VALUE         22
#define IDD_BIND_PARAMETERS        23
#define IDD_ENCRYPTION             24
#define IDD_DROP                   25
#define IDD_TABLENAME              26
#define IDD_RESULTS_COMPARISON     27
#define IDD_SHORTCUTS              28
#define IDD_ADD_INDEX              29
#define IDD_CUSTOM_FUNCTIONS       30
#define IDD_RESULT_FIND            31
#define IDD_TEXT_COMPARISON        32
#define IDD_FK_SELECTOR            33

#define IDD_TOOL_IMPORT_CSV        35
#define IDD_TOOL_IMPORT_JSON       36
#define IDD_TOOL_EXPORT_CSV        37
#define IDD_TOOL_EXPORT_JSON       38
#define IDD_TOOL_EXPORT_SQL        39
#define IDD_TOOL_EXPORT_IMPORT_ODBC 40
#define IDD_TOOL_GENERATE_DATA     41
#define IDD_TOOL_DATABASE_DIAGRAM  42
#define IDD_TOOL_COMPARE_DATABASE  43
#define IDD_TOOL_DATABASE_SEARCH   44
#define IDD_TOOL_STATISTICS        45
#define IDD_TOOL_FOREIGN_KEY_CHECK 46
#define IDD_TOOL_DESKTOP_SHORTCUT  47

#define IDC_DLG_OK                 51
#define IDC_DLG_CANCEL             52
#define IDC_DLG_DELETE             53

#define IDC_STATUSBAR              60
#define IDC_TOOLBAR                61
#define IDC_MAINTAB                62
#define IDC_EDITOR                 63
#define IDC_TAB                    64
#define IDC_TREE                   65
#define IDC_SCHEMA                 66
#define IDC_CLI_EDITOR             70
#define IDC_CLI_RESULT             71
#define IDC_CLI_RAWDATA            72
#define IDC_QUERYLIST              80
#define IDC_REFLIST                81
#define IDC_PREVIEW_TEXT           82
#define IDC_PREVIEW_IMAGE          83
#define IDC_FUNCTION_CODES         84

#define IDC_DLG_EDITOR             100
#define IDC_DLG_LABEL              101
#define IDC_DLG_TABLENAME          102
#define IDC_DLG_ISCOLUMNS          103
#define IDC_DLG_DELIMITER          104
#define IDC_DLG_ENCODING           105
#define IDC_DLG_NEWLINE            106
#define IDC_DLG_OBJECTLIST         107
#define IDC_DLG_DATAONLY           108
#define IDC_DLG_DDLONLY            109
#define IDC_DLG_DATADDL            110
#define IDC_DLG_PREVIEW            111
#define IDC_DLG_COLUMNS            112

#define IDC_DLG_FONT_FAMILY        113
#define IDC_DLG_FONT_SIZE          114
#define IDC_DLG_AUTOLOAD           115
#define IDC_DLG_RESTORE_DB         116
#define IDC_DLG_RESTORE_EDITOR     117
#define IDC_DLG_CHECK_UPDATES      118
#define IDC_DLG_ROW_LIMIT          119
#define IDC_DLG_RETAIN_PASSPHRASE  120
#define IDC_DLG_EXIT_BY_ESCAPE     121
#define IDC_DLG_INDENT             122
#define IDC_DLG_TOOLBAR            123
#define IDC_DLG_HTTP_SERVER        124
#define IDC_DLG_HTTP_SERVER_PORT   125
#define IDC_DLG_STARTUP            126
#define IDC_DLG_MULTIPLE_INSERT    127
#define IDC_DLG_EDIT_VALUE         128
#define IDC_DLG_TAB_AUTOCOMPLETE   129

#define IDC_DLG_QUERYADD           130
#define IDC_DLG_QUERYFILTER        131
#define IDC_DLG_QUERYLIST          132
#define IDC_DLG_USERDATA           133
#define IDC_DLG_ROWS               134

#define IDC_DLG_IDXNAME            135
#define IDC_DLG_IDXWHERE           136
#define IDC_DLG_INDEXED_COLUMNS    137

#define IDC_DLG_COLNAME            140
#define IDC_DLG_COLTYPE            141
#define IDC_DLG_DEFVALUE           142
#define IDC_DLG_CHECK              143
#define IDC_DLG_ISUNIQUE           144
#define IDC_DLG_ISNOTNULL          145
#define IDC_DLG_EXAMPLE            146
#define IDC_DLG_CLI_ROW_LIMIT      147
#define IDC_DLG_BEEP_ON_QUERY_END  148
#define IDC_DLG_SYNC_OFF           149

#define IDC_DLG_ROW_ADD            150
#define IDC_DLG_ROW_DEL            151
#define IDC_DLG_ROW_UP             152
#define IDC_DLG_ROW_DOWN           153
#define IDC_DLG_ISWITHOUT_ROWID    154
#define IDC_DLG_MORE               155
#define IDC_DLG_STRICT             156

#define IDC_DLG_REFRESH            157
#define IDC_DLG_CONNECTION_STRING  158
#define IDC_DLG_ISTABLE            159
#define IDC_DLG_ISQUERY            160
#define IDC_DLG_HELP               163
#define IDC_DLG_ODBC_STRATEGY      164
#define IDC_DLG_ODBC_SCHEMA        165
#define IDC_DLG_ODBC_SCHEMA_LABEL  166
#define IDC_DLG_DATABASE           170
#define IDC_DLG_DATABASE_SELECTOR  171
#define IDC_DLG_COMPARE            172
#define IDC_DLG_COMPARE_SCHEMA     173
#define IDC_DLG_COMPARE_DATA       174
#define IDC_DLG_SCHEMA_DIFF        175
#define IDC_DLG_DATA_DIFF          176
#define IDC_DLG_ORIGINAL           177
#define IDC_DLG_COMPARED           178
#define IDC_DLG_ORIGINAL_DDL       179
#define IDC_DLG_COMPARED_DDL       180
#define IDC_DLG_ORIGINAL_LABEL     181
#define IDC_DLG_COMPARED_LABEL     182
#define IDC_DLG_ORIGINAL_COUNT     183
#define IDC_DLG_COMPARED_COUNT     184
#define IDC_DLG_DIFF_ROWS          185
#define IDC_DLG_SEARCH_TEXT        186
#define IDC_DLG_SEARCH             187
#define IDC_DLG_PATTERN            188
#define IDC_DLG_TABLENAMES         189
#define IDC_DLG_ISTRUNCATE         190
#define IDC_DLG_ISREPLACE          191
#define IDC_DLG_IMPORT_ACTION      192
#define IDC_DLG_IMPORT_ACTION2     193
#define IDC_DLG_TABLES             194
#define IDC_DLG_SEARCH_QUERY       195
#define IDC_DLG_SEARCH_QUERY_TEXT  196
#define IDC_DLG_SEARCH_RESULT      197
#define IDC_DLG_SEARCH_ROWS        198
#define IDC_DLG_STATISTICS         199
#define IDC_DLG_FILTER             200
#define IDC_DLG_ODBC_MANAGER       201
#define IDC_DLG_ASK_DELETE         202
#define IDC_DLG_WORD_WRAP          203
#define IDC_DLG_CLEAR_VALUES       204
#define IDC_DLG_GRID_COLOR         210 // + 4 next
#define IDC_DLG_GRID_COLOR_EDIT    215
#define IDC_DLG_VALUE_SELECTOR     216
#define IDC_DLG_TYPE               217
#define IDC_DLG_FOREIGN_KEY_CHECK  218
#define IDC_DLG_TRIM_VALUES        219
#define IDC_DLG_SKIP_EMPTY         220
#define IDC_DLG_ANORT_ON_ERROR     221
#define IDC_DLG_LINK_NAME          222

#define IDC_DLG_FIND_STRING        224
#define IDC_DLG_REPLACE_STRING     225
#define IDC_DLG_FIND               226
#define IDC_DLG_REPLACE            227
#define IDC_DLG_REPLACE_ALL        228
#define IDC_DLG_CASE_SENSITIVE     229

#define IDC_DLG_SHORTCUTS          235
#define IDC_TAB_EDIT               236
#define IDC_DLG_FUNCTIONS          237
#define IDC_DLG_NAME               238
#define IDC_DLG_NAME_LABEL         239
#define IDC_DLG_CODE_LABEL         240

#define IDC_DLG_CIPHER_KEY                  247
#define IDC_DLG_CIPHER_STORE_KEY            248
#define IDC_DLG_CIPHER                      249
#define IDC_DLG_CIPHER_LEGACY               250 // iterable!
#define IDC_DLG_CIPHER_PAGESIZE_LABEL       261
#define IDC_DLG_CIPHER_PAGESIZE             262
#define IDC_DLG_CIPHER_PROFILE_LABEL        263
#define IDC_DLG_CIPHER_PROFILE              264
#define IDC_DLG_CIPHER_KDF_ITER_LABEL       265
#define IDC_DLG_CIPHER_KDF_ITER             266
#define IDC_DLG_CIPHER_KDF_ALGORITHM_LABEL  267
#define IDC_DLG_CIPHER_KDF_ALGORITHM        268
#define IDC_DLG_CIPHER_HMAC_USE_LABEL       269
#define IDC_DLG_CIPHER_HMAC_USE             270
#define IDC_DLG_CIPHER_HMAC_ALGORITHM_LABEL 271
#define IDC_DLG_CIPHER_HMAC_ALGORITHM       272
#define IDC_DLG_CIPHER_FAST_KDF_ITER_LABEL  273
#define IDC_DLG_CIPHER_FAST_KDF_ITER        274
#define IDC_DLG_CIPHER_HMAC_SALT_LABEL      275
#define IDC_DLG_CIPHER_HMAC_SALT            276
#define IDC_DLG_CIPHER_HMAC_PGNO_LABEL      277
#define IDC_DLG_CIPHER_HMAC_PGNO            278
#define IDC_DLG_CIPHER_HEADER_SIZE_LABEL    279
#define IDC_DLG_CIPHER_HEADER_SIZE          280

#define IDC_DLG_GEN_ISTRUNCATE     281
#define IDC_DLG_GEN_ROW_COUNT      282
#define IDC_DLG_GEN_COLUMNS        283
#define IDC_DLG_GEN_COLUMN         284
#define IDC_DLG_GEN_COLUMN_NAME    285
#define IDC_DLG_GEN_COLUMN_TYPE    286
#define IDC_DLG_GEN_OPTION         287
#define IDC_DLG_GEN_OPTION_LABEL   288
#define IDC_DLG_GEN_OPTION_START   289
#define IDC_DLG_GEN_OPTION_END     290
#define IDC_DLG_GEN_OPTION_TABLE   291
#define IDC_DLG_GEN_OPTION_COLUMN  292
#define IDC_DLG_GEN_OPTION_MULTIPLIER  293
#define IDC_DLG_GEN_OPTION_EXPR    294

#define IDC_DLG_CHART              295
#define IDC_DLG_CHART_OPTIONS      296
#define IDC_DLG_CHART_TYPE         297
#define IDC_DLG_CHART_BASE_LABEL   298
#define IDC_DLG_CHART_BASE         299
#define IDC_DLG_CHART_COLUMN       300 // iterable

#define IDC_MENU_MAIN              500
#define IDC_MENU_EDITOR            501
#define IDC_MENU_RESULT            502
#define IDC_MENU_TAB_RESULT        503
#define IDC_MENU_EDIT_DATA         504
#define IDC_MENU_VIEW_DATA         505
#define IDC_MENU_BLOB              506
#define IDC_MENU_DIAGRAM           507
#define IDC_MENU_QUERYLIST         508
#define IDC_MENU_CHART             509
#define IDC_MENU_CLI               510
#define IDC_MENU_PREVIEW_TEXT      511
#define IDC_MENU_PREVIEW_IMAGE     512

#define IDC_MENU_TABLEVIEW         521
#define IDC_MENU_INDEXTRIGGER      522
#define IDC_MENU_TABLE             523
#define IDC_MENU_VIEW              524
#define IDC_MENU_INDEX             525
#define IDC_MENU_TRIGGER           526
#define IDC_MENU_COLUMN            527
#define IDC_MENU_DISABLED          528
#define IDC_MENU_TEMP              529

#define IDM_OPEN                   1401
#define IDM_CLOSE                  1402
#define IDM_SAVE_AS                1403
#define IDM_EXIT                   1404
#define IDM_ENCRYPTION             1405
#define IDM_ATTACH                 1406
#define IDM_SETTINGS               1407
#define IDM_SCHEMA                 1450 // iterable

#define IDM_SHORTCUTS              1510
#define IDM_SAVE                   1511
#define IDM_EXECUTE                1512
#define IDM_EXECUTE_BATCH          1513
#define IDM_PLAN                   1514
#define IDM_EXECUTE_CURRENT        1515
#define IDM_PLAN_CURRENT           1516
#define IDM_HISTORY                1517
#define IDM_GISTS                  1518
#define IDM_INTERRUPT              1519

#define IDM_IMPORT_SQL             1520
#define IDM_IMPORT_CSV             1521
#define IDM_IMPORT_JSON            1522
#define IDM_IMPORT_ODBC            1523
#define IDM_EXPORT_SQL             1524
#define IDM_EXPORT_CSV             1525
#define IDM_EXPORT_JSON            1526
#define IDM_EXPORT_EXCEL           1527
#define IDM_EXPORT_ODBC            1528
#define IDM_CHECK_INTEGRITY        1529
#define IDM_FOREIGN_KEY_CHECK      1530
#define IDM_VACUUM                 1531
#define IDM_REINDEX                1532
#define IDM_DESKTOP_SHORTCUT       1533
#define IDM_LOCATE_FILE            1534
#define IDM_STATISTICS             1535
#define IDM_GENERATE_DATA          1536
#define IDM_DATABASE_DIAGRAM       1537
#define IDM_COMPARE_DATABASE       1538
#define IDM_DATABASE_SEARCH        1539
#define IDM_CUSTOM_FUNCTIONS       1540

#define IDM_HELP                   1550
#define IDM_ABOUT                  1551
#define IDM_HOMEPAGE               1552
#define IDM_WIKI                   1553
#define IDM_HOTKEYS                1554
#define IDM_TIPS                   1555
#define IDM_EXTENSIONS             1556
#define IDM_SQLITE_HOMEPAGE        1557
#define IDM_TUTORIAL1              1558
#define IDM_TUTORIAL2              1559

#define IDM_NEXT_RESULT            1590
#define IDM_CHANGE_FOCUS           1591
#define IDM_PROCESS_INDENT         1592
#define IDM_ESCAPE                 1593

#define IDM_OPEN_EDITOR            1595
#define IDM_CLOSE_EDITOR           1596
#define IDM_PREV_EDITOR            1597
#define IDM_NEXT_EDITOR            1598

#define IDM_EDITOR_CUT             1601
#define IDM_EDITOR_COPY            1602
#define IDM_EDITOR_PASTE           1603
#define IDM_EDITOR_DELETE          1604
#define IDM_EDITOR_FIND            1605
#define IDM_EDITOR_FIND_NEXT       1606
#define IDM_EDITOR_REPLACE_NEXT    1607
#define IDM_EDITOR_COMMENT         1608
#define IDM_EDITOR_FORMAT          1609
#define IDM_EDITOR_COMPARE         1610

#define IDM_RESULT_PREVIEW         1620
#define IDM_RESULT_FILTERS         1621
#define IDM_RESULT_CHART           1622
#define IDM_RESULT_VALUE_FILTER    1623
#define IDM_RESULT_COPY_CELL       1624
#define IDM_RESULT_COPY_ROW        1625
#define IDM_RESULT_AS_TABLE        1626
#define IDM_RESULT_EXPORT          1627
#define IDM_RESULT_EXCEL           1628
#define IDM_RESULT_TRANSPOSE       1629
#define IDM_RESULT_HEATMAP         1630
#define IDM_RESULT_COMPARE         1640 // iterable, 50

#define IDM_BLOB_VIEW              1680
#define IDM_BLOB_NULL              1681
#define IDM_BLOB_IMPORT            1682
#define IDM_BLOB_EXPORT            1683

#define IDM_DEMODB_BOOKSTORE       1685
#define IDM_DEMODB_CHINOOK         1686
#define IDM_DEMODB_NORTHWIND       1687
#define IDM_DEMODB_WORLD           1688

#define IDM_QUERY_COPY             1690
#define IDM_QUERY_ADD_NEW          1691
#define IDM_QUERY_ADD_OLD          1692
#define IDM_QUERY_DELETE           1693

#define IDM_TAB_RESULT_PIN         1695
#define IDM_TAB_RESULT_COPY_QUERY  1696

#define IDM_QUERY_DATA             1701
#define IDM_EDIT_DATA              1702
#define IDM_EDIT                   1703
#define IDM_DELETE                 1704
#define IDM_RENAME                 1705
#define IDM_ADD_COLUMN             1706
#define IDM_ADD_INDEX              1707
#define IDM_ERASE_DATA             1708
#define IDM_DDL                    1709
#define IDM_ENABLE                 1710
#define IDM_DISABLE                1711
#define IDM_VIEW                   1712
#define IDM_TEMP_EXPLAIN           1713
#define IDM_PIN_ON_TOP             1714

#define IDM_ADD                    1715
#define IDM_REFRESH                1716
#define IDM_ENABLE_ALL             1717
#define IDM_DISABLE_ALL            1718

#define IDM_VALUE_EDIT             1721
#define IDM_ROW_ADD                1722
#define IDM_ROW_EDIT               1723
#define IDM_ROW_DELETE             1724
#define IDM_ROW_REFRESH            1725
#define IDM_ROW_DUPLICATE          1726
#define IDM_LAST_SEPARATOR         1727
#define IDM_EXPORT_PNG             1729
#define IDM_EXPORT_CLIPBOARD       1730
#define IDM_EXPORT_FILE            1731


#define IDM_LINK_FK                1735
#define IDM_LINK_VIEW              1736
#define IDM_LINK_TRIGGER           1737

#define IDM_CLI_COPY               1740
#define IDM_CLI_CUT                1741
#define IDM_CLI_CLEAR_ALL          1742

#define IDM_PREV_DIALOG            1750
#define IDM_NEXT_DIALOG            1751

#define IDM_TEST                   1760

// Iterable. Should have a gap.
#define IDC_HEADER_EDIT            2500
#define IDC_TAB_ROWS               2600
#define IDC_TAB_MESSAGE            2700
#define IDC_TAB_PREVIEW            2800

#define IDC_ROW_LABEL              3000
#define IDC_ROW_EDIT               3500
#define IDC_ROW_SWITCH             4000
#define IDM_RECENT                 4500
#define IDC_DATABASE_DIAGRAM_TABLE 5000

#define IDI_LOGO                   6000
#define IDI_LOGO2                  6001
#define IDB_TREEVIEW               6002
#define IDB_TOOLBAR                6003
#define IDB_TOOLBAR_DIAGRAM        6006
#define IDB_TOOLBAR_DATA           6010
#define IDB_TOOLBAR_FUNCTIONS      6011
#define IDB_TAB                    6012

#define IDA_ACCEL                  6100
#define IDA_ACCEL2                 6101

#define IDT_EDIT_DATA              7000
#define IDT_HIGHLIGHT              7001
#define IDT_REFERENCE              7002

#define IDR_HELP                   9000
#define IDR_EXCEL                  9001

#define IDS_CREATE_DDL             10000
#define IDS_CREATE_TABLE           10001
#define IDS_CREATE_VIEW            10002
#define IDS_CREATE_INDEX           10003
#define IDS_CREATE_TRIGGER         10004
#define IDS_ABOUT                  10010
#define IDS_HOTKEYS                10011
#define IDS_TIPS                   10012
#define IDS_EXTENSIONS             10013
#define IDS_WELCOME                10015
#define IDS_ODBC_HELP              10016
#define IDS_CLI_HELP               10017
#define IDS_FUNCTIONS_HELP         10018
#define IDS_TEMP_EXPLAIN           10019
#define IDS_TOOLTIP_OPEN           IDM_OPEN
#define IDS_TOOLTIP_CLOSE          IDM_CLOSE
#define IDS_TOOLTIP_SAVE           IDM_SAVE
#define IDS_TOOLTIP_PLAN           IDM_PLAN
#define IDS_TOOLTIP_EXECUTE        IDM_EXECUTE
#define IDS_TOOLTIP_INTERRUPT      IDM_INTERRUPT

#define WMU_SET_DLG_ROW_DATA       WM_USER + 1
#define WMU_UPDATE_DATA            WM_USER + 2
#define WMU_SOURCE_UPDATED         WM_USER + 3
#define WMU_ADD_ROW                WM_USER + 4
#define WMU_UPDATE_ROWNO           WM_USER + 5
#define WMU_TARGET_CHANGED         WM_USER + 6
#define WMU_TYPE_CHANGED           WM_USER + 7
#define WMU_REFTABLE_CHANGED       WM_USER + 8
#define WMU_SHOW_INFO              WM_USER + 10
#define WMU_RESET_LISTVIEW         WM_USER + 11
#define WMU_EDIT_VALUE             WM_USER + 12
#define WMU_UPDATE_COLSIZE         WM_USER + 13
#define WMU_SET_CURRENT_CELL       WM_USER + 14
#define WMU_SYNC_CURRENT_CELL      WM_USER + 15
#define WMU_UPDATE_DIAGRAM         WM_USER + 16
#define WMU_UNREGISTER_DIALOG      WM_USER + 17
#define WMU_OBJECT_CREATED         WM_USER + 18
#define WMU_APPEND_TEXT            WM_USER + 19
#define WMU_SET_SCROLL_HEIGHT      WM_USER + 20
#define WMU_CIPHER_CHANGED         WM_USER + 21
#define WMU_SET_VALUE              WM_USER + 22
#define WMU_GET_VALUE              WM_USER + 23
#define WMU_SET_ICON               WM_USER + 24
#define WMU_OPEN_NEW_TAB           WM_USER + 25
#define WMU_UPDATE_CARET_INFO      WM_USER + 26
#define WMU_UPDATE_CHART           WM_USER + 27
#define WMU_UPDATE_MINMAX          WM_USER + 28
#define WMU_RESORT_DATA            WM_USER + 29
#define WMU_CREATE_VALUE_SELECTOR  WM_USER + 30
#define WMU_OPEN_FK_VALUE_SELECTOR WM_USER + 31
#define WMU_CTLCOLOREDIT           WM_USER + 32
#define WMU_GET_CURRENT_RESULTSET  WM_USER + 33
#define WMU_UPDATE_SIZES           WM_USER + 34
#define WMU_UPDATE_PREVIEW         WM_USER + 35
#define WMU_RESET_CACHE            WM_USER + 36
#define WMU_FUNCTION_SAVE          WM_USER + 40
#define WMU_REGISTER_FUNCTION      WM_USER + 41
#define WMU_UNREGISTER_FUNCTION    WM_USER + 42
#define WMU_UPDATE_RESULTSET       WM_USER + 43
#define WMU_UPDATE_FILTER_SIZE     WM_USER + 44
#define WMU_SET_HEADER_FILTERS     WM_USER + 45
#define WMU_AUTO_COLUMN_SIZE       WM_USER + 46
#define WMU_UPDATE_SB_RESULTSET    WM_USER + 50
#define WMU_RESULT_SEARCH          WM_USER + 51
#define WMU_COMPARE                WM_USER + 52
#define WMU_HEATMAP                WM_USER + 53

#define WMU_HIGHLIGHT              WM_USER + 60
#define WMU_SELECTION_CHANGED      WM_USER + 61
#define WMU_TEXT_CHANGED           WM_USER + 62

#define WMU_TAB_ADD                WM_USER + 140
#define WMU_TAB_DELETE             WM_USER + 141
#define WMU_TAB_SET_CURRENT        WM_USER + 143
#define WMU_TAB_SET_TEXT           WM_USER + 144
#define WMU_TAB_GET_TEXT           WM_USER + 145
#define WMU_TAB_GET_COUNT          WM_USER + 146
#define WMU_TAB_GET_CURRENT        WM_USER + 147
#define WMU_TAB_SET_STYLE          WM_USER + 148
#define WMU_TAB_GET_STYLE          WM_USER + 149

#define NM_TAB_ADD                 WM_USER + 150
#define NM_TAB_DELETE              WM_USER + 151
#define NM_TAB_REQUEST_DELETE      WM_USER + 152
#define NM_TAB_CHANGE              WM_USER + 153
