#ifndef __MISSING_H__
#define __MISSING_H__

typedef struct tagNMCHAR {
  NMHDR hdr;
  UINT  ch;
  DWORD dwItemPrev;
  DWORD dwItemNext;
} NMCHAR, *LPNMCHAR;

typedef struct tagTVKEYDOWN {
  NMHDR hdr;
  WORD  wVKey;
  UINT  flags;
} NMTVKEYDOWN, *LPNMTVKEYDOWN;

#define MAPVK_VSC_TO_VK 1

#define CFM_BACKCOLOR 0x04000000

#define HDN_FILTERCHANGE                (HDN_FIRST - 12)
#define HDN_FILTERBTNCLICK              (HDN_FIRST - 13)
#define HDN_BEGINFILTEREDIT             (HDN_FIRST - 14)
#define HDN_ENDFILTEREDIT               (HDN_FIRST - 15)

#define HDM_SETFILTERCHANGETIMEOUT      (HDM_FIRST + 22)
#define HDM_CLEARFILTER                 (HDM_FIRST + 24)
#define LVN_INCREMENTALSEARCH           (LVN_FIRST - 63)

#define LVS_EX_AUTOSIZECOLUMNS           0x10000000
#define SS_REALSIZECONTROL               0x00000040L

#endif
