#define GUI_VERSION                "1.9.3"
#define GUI_VERSION2               1, 9, 3, 0
#ifdef __MINGW64__
#define GUI_PLATFORM               64
#else
#define GUI_PLATFORM               32
#endif
#define HELP_VERSION               8
#define EXTENSION_REPOSITORY       "little-brother/sqlite-extensions"
#define EXTENSION_DIRECTORY        "\\extensions\\"
#define VIEWER_REPOSITORY          "little-brother/sqlite-gui-value-viewers"
#define MODIFIER_REPOSITORY        "little-brother/sqlite-gui-column-modifiers"
#define PLUGIN_DIRECTORY           "\\plugins\\"

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
#define IDD_BIND_PARAMETERS        23
#define IDD_ENCRYPTION             24
#define IDD_DROP                   25
#define IDD_TABLENAME              26
#define IDD_RESULTS_COMPARISON     27
#define IDD_SHORTCUTS              28
#define IDD_ADD_INDEX              29
#define IDD_CUSTOM_FUNCTIONS       30
#define IDD_RESULT_FIND            31
#define IDD_FK_SELECTOR            33
#define IDD_COLOR_PICKER           34
#define IDD_URI_DB_PATH            35
#define IDD_ADDON_MANAGER          36
#define IDD_INFO                   37
#define IDD_ATTACH_ODBC            38
#define IDD_VALUE_VIEWER           39
#define IDD_VALUE_EDITOR           40
#define IDD_REFERENCES             41

#define IDD_TOOL_IMPORT_CSV        55
#define IDD_TOOL_IMPORT_JSON       56
#define IDD_TOOL_IMPORT_SHEET      57
#define IDD_TOOL_EXPORT_CSV        60
#define IDD_TOOL_EXPORT_JSON       61
#define IDD_TOOL_EXPORT_SQL        62
#define IDD_TOOL_EXPORT_IMPORT_ODBC 63
#define IDD_TOOL_TEXT_COMPARISON   70
#define IDD_TOOL_GENERATE_DATA     71
#define IDD_TOOL_GENERATE_DATA_SET 72
#define IDD_TOOL_DATABASE_DIAGRAM  73
#define IDD_TOOL_COMPARE_DATABASE  74
#define IDD_TOOL_DATABASE_SEARCH   75
#define IDD_TOOL_STATISTICS        76
#define IDD_TOOL_FOREIGN_KEY_CHECK 77
#define IDD_TOOL_DESKTOP_SHORTCUT  78
#define IDD_TOOL_TRANSFORM_DATA    79

#define IDC_MENU_MAIN              100
#define IDC_MENU_EDITOR            101
#define IDC_MENU_RESULT            102
#define IDC_MENU_TAB_RESULT        103
#define IDC_MENU_EDIT_DATA         104
#define IDC_MENU_VIEW_DATA         105
#define IDC_MENU_DIAGRAM           106
#define IDC_MENU_DIAGRAM_TABLE     107
#define IDC_MENU_QUERYLIST         108
#define IDC_MENU_CHART             109
#define IDC_MENU_CLI               110
#define IDC_MENU_PREVIEW           111

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
#define IDC_TREE_SEARCH            266
#define IDC_SCHEMA                 267
#define IDC_EDITOR_SEARCH          270
#define IDC_EDITOR_SEARCH_STRING   271
#define IDC_EDITOR_SEARCH_PREV     272
#define IDC_EDITOR_SEARCH_NEXT     273
#define IDC_EDITOR_SEARCH_CLOSE    274
#define IDC_CLI_EDITOR             275
#define IDC_CLI_RESULT             276
#define IDC_CLI_RAWDATA            277
#define IDC_QUERYLIST              280
#define IDC_REFLIST                281
#define IDC_PREVIEW                282
#define IDC_PREVIEW_INFO           283
#define IDC_FUNCTION_CODES         290

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
#define IDC_DLG_RESTORE_DB         315
#define IDC_DLG_RESTORE_EDITOR     316
#define IDC_DLG_CHECK_UPDATES      317
#define IDC_DLG_USE_LOGGER         318
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
#define IDC_DLG_SETTING_TAB        338
#define IDC_DLG_DISABLE_HELP       339

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
#define IDC_DLG_INFO               361
#define IDC_DLG_HELP               362
#define IDC_DLG_HELP_LABEL         363
#define IDC_DLG_ODBC_STRATEGY      364
#define IDC_DLG_ODBC_SCHEMA        365
#define IDC_DLG_ODBC_SCHEMA_LABEL  366
#define IDC_DLG_ALIAS              367
#define IDC_DLG_DRIVER             368
#define IDC_DLG_OPTIONS            369
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
#define IDC_DLG_FILTER_LABEL       401
#define IDC_DLG_ODBC_MANAGER       402
#define IDC_DLG_ASK_DELETE         403
#define IDC_DLG_AUTO_FILTERS       404
#define IDC_DLG_WORD_WRAP          405
#define IDC_DLG_CLEAR_VALUES       406
#define IDC_DLG_COLOR         410 // + 11 next
#define IDC_DLG_VALUE_SELECTOR     430
#define IDC_DLG_TYPE               431
#define IDC_DLG_FOREIGN_KEY_CHECK  432
#define IDC_DLG_TRIM_VALUES        433
#define IDC_DLG_SKIP_EMPTY         434
#define IDC_DLG_ANORT_ON_ERROR     435
#define IDC_DLG_LINK_NAME          436
#define IDC_DLG_READ_ONLY          437
#define IDC_DLG_FOREIGN_KEYS       438
#define IDC_DLG_LEGACY_RENAME      439

#define IDC_DLG_FONT_LABEL         440
#define IDC_DLG_ESCAPE_LABEL       441
#define IDC_DLG_INDENT_LABEL       442
#define IDC_DLG_ROW_LIMIT_LABEL    443
#define IDC_DLG_CLI_ROW_LIMIT_LABEL 444
#define IDC_DLG_BEEP_LABEL         445
#define IDC_DLG_EDITOR_LABEL       446
#define IDC_DLG_GRID_LABEL         447
#define IDC_DLG_STARTUP_LABEL      448
#define IDC_DLG_STATUSBAR          449

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
#define IDC_DLG_CODE               475
#define IDC_DLG_CODE_LABEL         476
#define IDC_DLG_SHEET_ID           477
#define IDC_DLG_SHEET_NAME         478
#define IDC_DLG_SHEET_RANGE        479
#define IDC_DLG_GOOGLE_KEY         480
#define IDC_DLG_GOOGLE_KEY_LABEL   481
#define IDC_DLG_DATASET_NAME       482
#define IDC_DLG_DATASET            483
#define IDC_DLG_ADDON_LIST         484
#define IDC_DLG_DELIMITER_LABEL    485
#define IDC_DLG_EXTENSION_REPOSITORY        486
#define IDC_DLG_EXTENSION_REPOSITORY_LABEL  487
#define IDC_DLG_VIEWER_REPOSITORY           488
#define IDC_DLG_VIEWER_REPOSITORY_LABEL     489
#define IDC_DLG_MODIFIER_REPOSITORY         490
#define IDC_DLG_MODIFIER_REPOSITORY_LABEL   491
#define IDC_DLG_REFERENCES         492
#define IDC_DLG_TITLE              493
#define IDC_DLG_QUERY              494
#define IDC_DLG_TABLENAME_LABEL    495
#define IDC_DLG_COLNAME_LABEL      496
#define IDC_DLG_TITLE_LABEL        497
#define IDC_DLG_QUERY_LABEL        498
#define IDC_DLG_SRC_SCHEMA         499
#define IDC_DLG_TRG_SCHEMA         500
#define IDC_DLG_SOURCE             501
#define IDC_DLG_TARGET             502
#define IDC_DLG_ISFIRSTCOLUMN      503
#define IDC_DLG_ISROWCOLNAMES      504
#define IDC_DLG_AXIS_X             510
#define IDC_DLG_AXIS_X_LABEL       511
#define IDC_DLG_AXIS_Y             512
#define IDC_DLG_AXIS_Y_LABEL       513
#define IDC_DLG_VALUE              514
#define IDC_DLG_VALUE_LABEL        515
#define IDC_DLG_DUPLICATES_LABEL   516
#define IDC_DLG_DUPLICATES         517

#define IDC_DLG_CIPHER_KEY                  601
#define IDC_DLG_CIPHER_SHOW_KEY             602
#define IDC_DLG_CIPHER_STORE_KEY            603
#define IDC_DLG_CIPHER                      604
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
#define IDC_DLG_GEN_OPTION_ONEOF   715

#define IDC_DLG_CHART              730
#define IDC_DLG_CHART_OPTIONS      731
#define IDC_DLG_CHART_TYPE         732
#define IDC_DLG_CHART_BASE_LABEL   733
#define IDC_DLG_CHART_BASE         734
#define IDC_DLG_CHART_COLUMN       740 // iterable

#define IDM_OPEN                   1001
#define IDM_CLOSE                  1002
#define IDM_SAVE_AS                1003
#define IDM_ATTACH                 1004
#define IDM_ATTACH_ODBC            1005
#define IDM_ENCRYPTION             1006
#define IDM_VIEWER_PLUGINS         1007
#define IDM_MODIFIER_PLUGINS       1008
#define IDM_EXTENSIONS             1009
#define IDM_CUSTOM_FUNCTIONS       1020
#define IDM_REFERENCES             1021
#define IDM_SETTINGS               1022
#define IDM_EXIT                   1023

#define IDM_RECENT                 1100 // iterable
#define IDM_RECENT_ATTACHED        1150 // iterable
#define IDM_RECENT_ATTACHED_ODBC   1200 // iterable
#define IDM_SCHEMA                 1250 // iterable

#define IDM_SHORTCUTS              1510
#define IDM_SAVE                   1511
#define IDM_EXECUTE                1512
#define IDM_EXECUTE_BATCH          1513
#define IDM_PLAN                   1514
#define IDM_EXECUTE_SELECTION      1515
#define IDM_PLAN_SELECTION         1516
#define IDM_EXECUTE_CURRENT_LINE   1517
#define IDM_HISTORY                1518
#define IDM_GISTS                  1519
#define IDM_INTERRUPT              1520

#define IDM_IMPORT_SQL             1525
#define IDM_IMPORT_CSV             1526
#define IDM_IMPORT_JSON            1527
#define IDM_IMPORT_GOOGLE_SHEETS   1528
#define IDM_IMPORT_ODBC            1529
#define IDM_EXPORT_SQL             1530
#define IDM_EXPORT_CSV             1531
#define IDM_EXPORT_JSON            1532
#define IDM_EXPORT_EXCEL           1533
#define IDM_EXPORT_ODBC            1534
#define IDM_CHECK_INTEGRITY        1535
#define IDM_FOREIGN_KEY_CHECK      1536
#define IDM_VACUUM                 1537
#define IDM_REINDEX                1538
#define IDM_DESKTOP_SHORTCUT       1539
#define IDM_LOCATE_FILE            1540
#define IDM_STATISTICS             1541
#define IDM_GENERATE_DATA          1542
#define IDM_COMPARE_DATABASE       1544
#define IDM_DATABASE_SEARCH        1545
#define IDM_DATABASE_DIAGRAM       1546
#define IDM_TRANSFORM_DATA         1547

#define IDM_HELP                   1550
#define IDM_ABOUT                  1551
#define IDM_HOMEPAGE               1552
#define IDM_WIKI                   1553
#define IDM_HOTKEYS                1554
#define IDM_TIPS                   1555
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

#define IDM_PREVIEW_SWITCH_PLUGIN  1616
#define IDM_PREVIEW_TO_FILE        1617
#define IDM_PREVIEW_AS_FILE        1618

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

#define IDM_VALUE_FILE_OPEN        1681
#define IDM_VALUE_FILE_SAVE        1682
#define IDM_VALUE_FILE_SET         1683
#define IDM_VALUE_FILE_EDIT        1684

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
#define IDM_DIAGRAM                1706
#define IDM_DUPLICATE              1707
#define IDM_GENERATE               1708
#define IDM_TRANSFORM              1709
#define IDM_ADD_COLUMN             1710
#define IDM_ADD_INDEX              1711
#define IDM_ERASE_DATA             1712
#define IDM_DDL                    1713
#define IDM_ENABLE                 1714
#define IDM_DISABLE                1715
#define IDM_VIEW                   1716
#define IDM_TEMP_EXPLAIN           1717
#define IDM_PIN_ON_TOP             1718

#define IDM_ADD                    1720
#define IDM_REFRESH                1721
#define IDM_ENABLE_ALL             1722
#define IDM_DISABLE_ALL            1723

#define IDM_VALUE_VIEW             1730
#define IDM_VALUE_EDIT             1731
#define IDM_VALUE_NULL             1732
#define IDM_ROW_ADD                1733
#define IDM_ROW_EDIT               1734
#define IDM_ROW_DELETE             1735
#define IDM_ROW_REFRESH            1736
#define IDM_ROW_DUPLICATE          1737
#define IDM_LAST_SEPARATOR         1738
#define IDM_EXPORT_PNG             1739
#define IDM_EXPORT_CLIPBOARD       1740
#define IDM_EXPORT_FILE            1741
#define IDM_FILTER_TYPE            1742
#define IDM_COMPARE_TEXTS          1743
#define IDM_CHART_RESET            1744
#define IDM_ISOLATE                1745

#define IDM_CLI_COPY               1750
#define IDM_CLI_CUT                1751
#define IDM_CLI_CLEAR_ALL          1752

#define IDM_SHOW_FKLINKS           1765
#define IDM_SHOW_COLTYPES          1766
#define IDM_SHOW_REASONS           1767

#define IDM_PREV_DIALOG            1770
#define IDM_NEXT_DIALOG            1771

#define IDM_TEST                   1780

#define IDM_RESULT_COMPARE         1800 // iterable, 100
#define IDM_RESULT_MODIFIER        1900 // iterable, MAX_PLUGIN_COUNT
#define IDM_MENU_TRIGGER           2200 // iterable, 50
#define IDM_MENU_INDEX             2250 // iterable, 50

// Iterable. Should have a gap.
#define IDC_HEADER_EDIT            2500
#define IDC_TAB_ROWS               2600
#define IDC_TAB_MESSAGE            2700
#define IDC_TAB_PREVIEW            2800

#define IDC_ROW_LABEL              3000
#define IDC_ROW_EDIT               3500
#define IDC_ROW_SWITCH             4000

#define IDI_LOGO                   6000
#define IDI_LOGO2                  6001
#define IDB_TAB                    6005
#define IDB_TOOLBAR16              6010
#define IDB_TOOLBAR24              6011
#define IDB_TOOLBAR_DATA16         6030
#define IDB_TOOLBAR_DATA24         6031
#define IDB_TOOLBAR_FUNCTIONS16    6041
#define IDB_TOOLBAR_FUNCTIONS24    6042
#define IDB_ICONS                  6043

#define IDA_ACCEL                  6100
#define IDA_ACCEL2                 6101

#define IDT_EDIT_DATA              7000
#define IDT_HIGHLIGHT              7001
#define IDT_REFERENCE              7002
#define IDT_EDITOR                 7003
#define IDT_DIAGRAM_INPUT          7004

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
#define IDS_ATTACH_ODBC_HELP       10017
#define IDS_CLI_HELP               10018
#define IDS_FUNCTIONS_HELP         10019
#define IDS_REFERENCES_HELP        10020
#define IDS_TEMP_EXPLAIN           10021
#define IDS_TOOLTIP_OPEN           IDM_OPEN
#define IDS_TOOLTIP_CLOSE          IDM_CLOSE
#define IDS_TOOLTIP_SAVE           IDM_SAVE
#define IDS_TOOLTIP_PLAN           IDM_PLAN
#define IDS_TOOLTIP_EXECUTE        IDM_EXECUTE
#define IDS_TOOLTIP_INTERRUPT      IDM_INTERRUPT
#define IDS_TOOLTIP_ROW_REFRESH    IDM_ROW_REFRESH
#define IDS_TOOLTIP_ROW_ADD        IDM_ROW_ADD
#define IDS_TOOLTIP_ROW_DELETE     IDM_ROW_DELETE
#define IDS_TOOLTIP_FILTER_TYPE    IDM_FILTER_TYPE
#define IDS_DIAGRAM_LINKS_ERROR    10025
#define IDS_TRANSPOSE_ERROR           10030
#define IDS_CONVERT_TO_MATRIX_ERROR   10031
#define IDS_CONVERT_TO_3COLUMNS_ERROR 10032

#define WMU_SET_DLG_ROW_DATA       WM_USER + 1
#define WMU_UPDATE_DATA            WM_USER + 2
#define WMU_SOURCE_UPDATED         WM_USER + 3
#define WMU_ADD_ROW                WM_USER + 4
#define WMU_UPDATE_ROWNO           WM_USER + 5
#define WMU_TARGET_CHANGED         WM_USER + 6
#define WMU_TYPE_CHANGED           WM_USER + 7
#define WMU_REFTABLE_CHANGED       WM_USER + 8
#define WMU_UPDATE_EXTENSIONS_UI   WM_USER + 9
#define WMU_SHOW_INFO              WM_USER + 10
#define WMU_RESET_LISTVIEW         WM_USER + 11
#define WMU_EDIT_VALUE             WM_USER + 12
#define WMU_UPDATE_COLSIZE         WM_USER + 13
#define WMU_SET_CURRENT_CELL       WM_USER + 14
#define WMU_SET_CURRENT_CELL_BLOB  WM_USER + 15
#define WMU_SET_CURRENT_CELL_VALUE WM_USER + 16
#define WMU_SYNC_CURRENT_CELL      WM_USER + 17
#define WMU_UNREGISTER_DIALOG      WM_USER + 18
#define WMU_OBJECT_CREATED         WM_USER + 19
#define WMU_APPEND_TEXT            WM_USER + 20
#define WMU_CIPHER_CHANGED         WM_USER + 21
#define WMU_SET_VALUE              WM_USER + 22
#define WMU_GET_VALUE              WM_USER + 23
#define WMU_SET_ECRYPT_FLAG        WM_USER + 24
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
#define WMU_RESET_PREVIEW          WM_USER + 36
#define WMU_RESET_CACHE            WM_USER + 37
#define WMU_FUNCTION_SAVE          WM_USER + 39
#define WMU_SAVE_REFERENCE         WM_USER + 40
#define WMU_UPDATE_REFERENCES      WM_USER + 41
#define WMU_UPDATE_RESULTSET       WM_USER + 43
#define WMU_UPDATE_FILTER_SIZE     WM_USER + 44
#define WMU_SET_HEADER_FILTERS     WM_USER + 45
#define WMU_AUTO_COLUMN_SIZE       WM_USER + 46
#define WMU_UPDATE_SB_RESULTSET    WM_USER + 50
#define WMU_RESULT_SEARCH          WM_USER + 51
#define WMU_COMPARE                WM_USER + 52
#define WMU_UPDATE_SHEET_IDS       WM_USER + 54
#define WMU_UPDATE_SHEET_PREVIEW   WM_USER + 55
#define WMU_SET_THEME              WM_USER + 56
#define WMU_DATASET_CHANGED        WM_USER + 57
#define WMU_TAB_CHANGED            WM_USER + 58
#define WMU_UPDATE_META            WM_USER + 59
#define WMU_SET_SCROLL_HEIGHT      WM_USER + 60
#define WMU_ADD_EMPTY_ROW          WM_USER + 61
#define WMU_TEST                   WM_USER + 62
#define WMU_RESET_MODIFIER         WM_USER + 63
#define WMU_GET_TABLE              WM_USER + 64
#define WMU_RECALC_WIDTH           WM_USER + 65
#define WMU_GET_DIAGRAM_RECT       WM_USER + 66
#define WMU_TRANSFORM_TRANSPOSE    WM_USER + 67
#define WMU_TRANSFORM_TO_MATRIX    WM_USER + 68
#define WMU_TRANSFORM_TO_3COLUMNS  WM_USER + 69


// ricedit.h has own WM_USER + N message, but N less 210
#define WMU_HIGHLIGHT              WM_USER + 260
#define WMU_SELECTION_CHANGED      WM_USER + 261
#define WMU_TEXT_CHANGED           WM_USER + 262

#define WMU_TAB_ADD                WM_USER + 340
#define WMU_TAB_DELETE             WM_USER + 341
#define WMU_TAB_SET_CURRENT        WM_USER + 343
#define WMU_TAB_SET_TEXT           WM_USER + 344
#define WMU_TAB_GET_TEXT           WM_USER + 345
#define WMU_TAB_GET_COUNT          WM_USER + 346
#define WMU_TAB_GET_CURRENT        WM_USER + 347
#define WMU_TAB_SET_STYLE          WM_USER + 348
#define WMU_TAB_GET_STYLE          WM_USER + 349

#define NM_TAB_ADD                 WM_USER + 550
#define NM_TAB_DELETE              WM_USER + 551
#define NM_TAB_REQUEST_DELETE      WM_USER + 552
#define NM_TAB_CHANGE              WM_USER + 553
