#ifndef __MISSING_H__
#define __MISSING_H__

typedef struct tagTVKEYDOWN {
  NMHDR hdr;
  WORD  wVKey;
  UINT  flags;
} NMTVKEYDOWN, *LPNMTVKEYDOWN;

#define MAPVK_VSC_TO_VK 1
#define LVS_EX_AUTOSIZECOLUMNS 0x10000000
#define CFM_BACKCOLOR 0x04000000

#endif
