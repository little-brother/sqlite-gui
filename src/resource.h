#define GUI_VERSION                "1.7.8"
#define GUI_VERSION2               1, 7, 8, 0
#ifdef __MINGW64__
#define GUI_PLATFORM               64
#else
#define GUI_PLATFORM               32
#endif
#define HELP_VERSION               4

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
#define IDD_COLOR_PICKER           34
#define IDD_URI_DB_PATH           35

#define IDD_TOOL_IMPORT_CSV        55
#define IDD_TOOL_IMPORT_JSON       56
#define IDD_TOOL_EXPORT_CSV        57
#define IDD_TOOL_EXPORT_JSON       58
#define IDD_TOOL_EXPORT_SQL        59
#define IDD_TOOL_EXPORT_IMPORT_ODBC 60
#define IDD_TOOL_GENERATE_DATA     61
#define IDD_TOOL_DATABASE_DIAGRAM  62
#define IDD_TOOL_COMPARE_DATABASE  63
#define IDD_TOOL_DATABASE_SEARCH   64
#define IDD_TOOL_STATISTICS        65
#define IDD_TOOL_FOREIGN_KEY_CHECK 66
#define IDD_TOOL_DESKTOP_SHORTCUT  67


#define IDC_MENU_MAIN              100
#define IDC_MENU_EDITOR            101
#define IDC_MENU_RESULT            102
#define IDC_MENU_TAB_RESULT        103
#define IDC_MENU_EDIT_DATA         104
#define IDC_MENU_VIEW_DATA         105
#define IDC_MENU_BLOB              106
#define IDC_MENU_DIAGRAM           107
#define IDC_MENU_QUERYLIST         108
#define IDC_MENU_CHART             109
#define IDC_MENU_CLI               110
#define IDC_MENU_PREVIEW_TEXT      111
#define IDC_MENU_PREVIEW_IMAGE     112

#define IDC_MENU_TABLEVIEW         121
#define IDC_MENU_INDEXTRIGGER      122
#define IDC_MENU_TABLE             123
#define IDC_MENU_VIEW              124
#define IDC_MENU_INDEX             125
#define IDC_MENU_TRIGGER           126
#define IDC_MENU_COLUMN            127
#define IDC_MENU_DISABLED          128
#define IDC_MENU_TEMP              129 // treeMenus

#define IDC_DLG_OK                 251
#define IDC_DLG_CANCEL             252
#define IDC_DLG_DELETE             253

#define IDC_STATUSBAR              260
#define IDC_TOOLBAR                261
#define IDC_MAINTAB                262
#define IDC_EDITOR                 263
#define IDC_TAB                    264
#define IDC_TREE                   265
#define IDC_SCHEMA                 266
#define IDC_CLI_EDITOR             270
#define IDC_CLI_RESULT             271
#define IDC_CLI_RAWDATA            272
#define IDC_QUERYLIST              280
#define IDC_REFLIST                281
#define IDC_PREVIEW_TEXT           282
#define IDC_PREVIEW_IMAGE          283
#define IDC_FUNCTION_CODES         284

#define IDC_DLG_EDITOR             300
#define IDC_DLG_LABEL              301
#define IDC_DLG_TABLENAME          302
#define IDC_DLG_ISCOLUMNS          303
#define IDC_DLG_DELIMITER          304
#define IDC_DLG_ENCODING           305
#define IDC_DLG_NEWLINE            306
#define IDC_DLG_OBJECTLIST         307
#define IDC_DLG_DATAONLY           308
#define IDC_DLG_DDLONLY            309
#define IDC_DLG_DATADDL            310
#define IDC_DLG_PREVIEW            311
#define IDC_DLG_COLUMNS            312

#define IDC_DLG_FONT_FAMILY        313
#define IDC_DLG_FONT_SIZE          314
#define IDC_DLG_AUTOLOAD           315
#define IDC_DLG_RESTORE_DB         316
#define IDC_DLG_RESTORE_EDITOR     317
#define IDC_DLG_CHECK_UPDATES      318
#define IDC_DLG_ROW_LIMIT          319
#define IDC_DLG_RETAIN_PASSPHRASE  320
#define IDC_DLG_EXIT_BY_ESCAPE     321
#define IDC_DLG_INDENT             322
#define IDC_DLG_TOOLBAR            323
#define IDC_DLG_HTTP_SERVER        324
#define IDC_DLG_HTTP_SERVER_PORT   325
#define IDC_DLG_STARTUP            326
#define IDC_DLG_MULTIPLE_INSERT    327
#define IDC_DLG_EDIT_VALUE         328
#define IDC_DLG_TAB_AUTOCOMPLETE   329

#define IDC_DLG_QUERYADD           330
#define IDC_DLG_QUERYFILTER        331
#define IDC_DLG_QUERYLIST          332
#define IDC_DLG_USERDATA           333
#define IDC_DLG_ROWS               334

#define IDC_DLG_IDXNAME            335
#define IDC_DLG_IDXWHERE           336
#define IDC_DLG_INDEXED_COLUMNS    337

#define IDC_DLG_COLNAME            340
#define IDC_DLG_COLTYPE            341
#define IDC_DLG_DEFVALUE           342
#define IDC_DLG_CHECK              343
#define IDC_DLG_ISUNIQUE           344
#define IDC_DLG_ISNOTNULL          345
#define IDC_DLG_EXAMPLE            346
#define IDC_DLG_CLI_ROW_LIMIT      347
#define IDC_DLG_BEEP_ON_QUERY_END  348
#define IDC_DLG_SYNC_OFF           349

#define IDC_DLG_ROW_ADD            350
#define IDC_DLG_ROW_DEL            351
#define IDC_DLG_ROW_UP             352
#define IDC_DLG_ROW_DOWN           353
#define IDC_DLG_ISWITHOUT_ROWID    354
#define IDC_DLG_MORE               355
#define IDC_DLG_STRICT             356

#define IDC_DLG_REFRESH            357
#define IDC_DLG_CONNECTION_STRING  358
#define IDC_DLG_ISTABLE            359
#define IDC_DLG_ISQUERY            360
#define IDC_DLG_HELP               363
#define IDC_DLG_ODBC_STRATEGY      364
#define IDC_DLG_ODBC_SCHEMA        365
#define IDC_DLG_ODBC_SCHEMA_LABEL  366
#define IDC_DLG_DATABASE           370
#define IDC_DLG_DATABASE_SELECTOR  371
#define IDC_DLG_COMPARE            372
#define IDC_DLG_COMPARE_SCHEMA     373
#define IDC_DLG_COMPARE_DATA       374
#define IDC_DLG_SCHEMA_DIFF        375
#define IDC_DLG_DATA_DIFF          376
#define IDC_DLG_ORIGINAL           377
#define IDC_DLG_COMPARED           378
#define IDC_DLG_ORIGINAL_DDL       379
#define IDC_DLG_COMPARED_DDL       380
#define IDC_DLG_ORIGINAL_LABEL     381
#define IDC_DLG_COMPARED_LABEL     382
#define IDC_DLG_ORIGINAL_COUNT     383
#define IDC_DLG_COMPARED_COUNT     384
#define IDC_DLG_DIFF_ROWS          385
#define IDC_DLG_SEARCH_TEXT        386
#define IDC_DLG_SEARCH             387
#define IDC_DLG_PATTERN            388
#define IDC_DLG_TABLENAMES         389
#define IDC_DLG_ISTRUNCATE         390
#define IDC_DLG_ISREPLACE          391
#define IDC_DLG_IMPORT_ACTION      392
#define IDC_DLG_IMPORT_ACTION2     393
#define IDC_DLG_TABLES             394
#define IDC_DLG_SEARCH_QUERY       395
#define IDC_DLG_SEARCH_QUERY_TEXT  396
#define IDC_DLG_SEARCH_RESULT      397
#define IDC_DLG_SEARCH_ROWS        398
#define IDC_DLG_STATISTICS         399
#define IDC_DLG_FILTER             400
#define IDC_DLG_ODBC_MANAGER       401
#define IDC_DLG_ASK_DELETE         402
#define IDC_DLG_WORD_WRAP          403
#define IDC_DLG_CLEAR_VALUES       404
#define IDC_DLG_COLOR         410 // + 11 next
#define IDC_DLG_VALUE_SELECTOR     430
#define IDC_DLG_TYPE               431
#define IDC_DLG_FOREIGN_KEY_CHECK  432
#define IDC_DLG_TRIM_VALUES        433
#define IDC_DLG_SKIP_EMPTY         434
#define IDC_DLG_ANORT_ON_ERROR     435
#define IDC_DLG_LINK_NAME          436

#define IDC_DLG_FONT_LABEL         440
#define IDC_DLG_ESCAPE_LABEL       441
#define IDC_DLG_INDENT_LABEL       442
#define IDC_DLG_ROW_LIMIT_LABEL    443
#define IDC_DLG_CLI_ROW_LIMIT_LABEL 444
#define IDC_DLG_BEEP_LABEL         445
#define IDC_DLG_EDITOR_LABEL       446
#define IDC_DLG_GRID_LABEL         447
#define IDC_DLG_STARTUP_LABEL      448

#define IDC_DLG_FIND_STRING        450
#define IDC_DLG_REPLACE_STRING     451
#define IDC_DLG_FIND               452
#define IDC_DLG_REPLACE            453
#define IDC_DLG_REPLACE_ALL        454
#define IDC_DLG_CASE_SENSITIVE     455

#define IDC_DLG_SHORTCUTS          470
#define IDC_TAB_EDIT               471
#define IDC_DLG_FUNCTIONS          472
#define IDC_DLG_NAME               473
#define IDC_DLG_NAME_LABEL         474
#define IDC_DLG_CODE_LABEL         475

#define IDC_DLG_CIPHER_KEY                  601
#define IDC_DLG_CIPHER_STORE_KEY            602
#define IDC_DLG_CIPHER                      603
#define IDC_DLG_CIPHER_LEGACY               610 // iterable!
#define IDC_DLG_CIPHER_PAGESIZE_LABEL       630
#define IDC_DLG_CIPHER_PAGESIZE             631
#define IDC_DLG_CIPHER_PROFILE_LABEL        632
#define IDC_DLG_CIPHER_PROFILE              633
#define IDC_DLG_CIPHER_KDF_ITER_LABEL       634
#define IDC_DLG_CIPHER_KDF_ITER             635
#define IDC_DLG_CIPHER_KDF_ALGORITHM_LABEL  636
#define IDC_DLG_CIPHER_KDF_ALGORITHM        637
#define IDC_DLG_CIPHER_HMAC_USE_LABEL       638
#define IDC_DLG_CIPHER_HMAC_USE             639
#define IDC_DLG_CIPHER_HMAC_ALGORITHM_LABEL 640
#define IDC_DLG_CIPHER_HMAC_ALGORITHM       641
#define IDC_DLG_CIPHER_FAST_KDF_ITER_LABEL  642
#define IDC_DLG_CIPHER_FAST_KDF_ITER        643
#define IDC_DLG_CIPHER_HMAC_SALT_LABEL      644
#define IDC_DLG_CIPHER_HMAC_SALT            645
#define IDC_DLG_CIPHER_HMAC_PGNO_LABEL      646
#define IDC_DLG_CIPHER_HMAC_PGNO            647
#define IDC_DLG_CIPHER_HEADER_SIZE_LABEL    648
#define IDC_DLG_CIPHER_HEADER_SIZE          649

#define IDC_DLG_GEN_ISTRUNCATE     701
#define IDC_DLG_GEN_ROW_COUNT      702
#define IDC_DLG_GEN_COLUMNS        703
#define IDC_DLG_GEN_COLUMN         704
#define IDC_DLG_GEN_COLUMN_NAME    705
#define IDC_DLG_GEN_COLUMN_TYPE    706
#define IDC_DLG_GEN_OPTION         707
#define IDC_DLG_GEN_OPTION_LABEL   708
#define IDC_DLG_GEN_OPTION_START   709
#define IDC_DLG_GEN_OPTION_END     710
#define IDC_DLG_GEN_OPTION_TABLE   711
#define IDC_DLG_GEN_OPTION_COLUMN  712
#define IDC_DLG_GEN_OPTION_MULTIPLIER  713
#define IDC_DLG_GEN_OPTION_EXPR    714

#define IDC_DLG_CHART              730
#define IDC_DLG_CHART_OPTIONS      731
#define IDC_DLG_CHART_TYPE         732
#define IDC_DLG_CHART_BASE_LABEL   733
#define IDC_DLG_CHART_BASE         734
#define IDC_DLG_CHART_COLUMN       740 // iterable

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

#define IDR_INIT                   9000
#define IDR_HELP                   9001
#define IDR_EXCEL                  9002

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
