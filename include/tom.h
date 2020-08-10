/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the w64 mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error This stub requires an updated version of <rpcndr.h>
#endif

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif

#ifndef __tomtmp_h__
#define __tomtmp_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ITextDocument_FWD_DEFINED__
#define __ITextDocument_FWD_DEFINED__
  typedef struct ITextDocument ITextDocument;
#endif

#ifndef __ITextRange_FWD_DEFINED__
#define __ITextRange_FWD_DEFINED__
  typedef struct ITextRange ITextRange;
#endif

#ifndef __ITextSelection_FWD_DEFINED__
#define __ITextSelection_FWD_DEFINED__
  typedef struct ITextSelection ITextSelection;
#endif

#ifndef __ITextFont_FWD_DEFINED__
#define __ITextFont_FWD_DEFINED__
  typedef struct ITextFont ITextFont;
#endif

#ifndef __ITextPara_FWD_DEFINED__
#define __ITextPara_FWD_DEFINED__
  typedef struct ITextPara ITextPara;
#endif

#ifndef __ITextStoryRanges_FWD_DEFINED__
#define __ITextStoryRanges_FWD_DEFINED__
  typedef struct ITextStoryRanges ITextStoryRanges;
#endif

#ifndef __ITextDocument2_FWD_DEFINED__
#define __ITextDocument2_FWD_DEFINED__
  typedef struct ITextDocument2 ITextDocument2;
#endif

#ifndef __ITextMsgFilter_FWD_DEFINED__
#define __ITextMsgFilter_FWD_DEFINED__
  typedef struct ITextMsgFilter ITextMsgFilter;
#endif

#ifndef __ITextDocument_FWD_DEFINED__
#define __ITextDocument_FWD_DEFINED__
  typedef struct ITextDocument ITextDocument;
#endif

#ifndef __ITextRange_FWD_DEFINED__
#define __ITextRange_FWD_DEFINED__
  typedef struct ITextRange ITextRange;
#endif

#ifndef __ITextSelection_FWD_DEFINED__
#define __ITextSelection_FWD_DEFINED__
  typedef struct ITextSelection ITextSelection;
#endif

#ifndef __ITextFont_FWD_DEFINED__
#define __ITextFont_FWD_DEFINED__
  typedef struct ITextFont ITextFont;
#endif

#ifndef __ITextPara_FWD_DEFINED__
#define __ITextPara_FWD_DEFINED__
  typedef struct ITextPara ITextPara;
#endif

#ifndef __ITextStoryRanges_FWD_DEFINED__
#define __ITextStoryRanges_FWD_DEFINED__
  typedef struct ITextStoryRanges ITextStoryRanges;
#endif

#ifndef __ITextDocument2_FWD_DEFINED__
#define __ITextDocument2_FWD_DEFINED__
  typedef struct ITextDocument2 ITextDocument2;
#endif

#ifndef __ITextMsgFilter_FWD_DEFINED__
#define __ITextMsgFilter_FWD_DEFINED__
  typedef struct ITextMsgFilter ITextMsgFilter;
#endif

#include "oaidl.h"
#include "ocidl.h"

#ifndef __MIDL_user_allocate_free_DEFINED__
#define __MIDL_user_allocate_free_DEFINED__
  void *__RPC_API MIDL_user_allocate(size_t);
  void __RPC_API MIDL_user_free(void *);
#endif

  extern RPC_IF_HANDLE __MIDL_itf_tom_0000_v0_0_c_ifspec;
  extern RPC_IF_HANDLE __MIDL_itf_tom_0000_v0_0_s_ifspec;

#ifndef __tom_LIBRARY_DEFINED__
#define __tom_LIBRARY_DEFINED__
  typedef enum __MIDL___MIDL_itf_tom_0000_0001 {
    tomFalse = 0,tomTrue = -1,tomUndefined = -9999999,tomToggle = -9999998,tomAutoColor = -9999997,tomDefault = -9999996,tomSuspend = -9999995,
    tomResume = -9999994,tomApplyNow = 0,tomApplyLater = 1,tomTrackParms = 2,tomCacheParms = 3,tomBackward = 0xc0000001,tomForward = 0x3fffffff,
    tomMove = 0,tomExtend = 1,tomNoSelection = 0,tomSelectionIP = 1,tomSelectionNormal = 2,tomSelectionFrame = 3,tomSelectionColumn = 4,
    tomSelectionRow = 5,tomSelectionBlock = 6,tomSelectionInlineShape = 7,tomSelectionShape = 8,tomSelStartActive = 1,tomSelAtEOL = 2,
    tomSelOvertype = 4,tomSelActive = 8,tomSelReplace = 16,tomEnd = 0,tomStart = 32,tomCollapseEnd = 0,tomCollapseStart = 1,tomClientCoord = 256,
    tomNone = 0,tomSingle = 1,tomWords = 2,tomDouble = 3,tomDotted = 4,tomDash = 5,tomDashDot = 6,tomDashDotDot = 7,tomWave = 8,tomThick = 9,
    tomHair = 10,tomLineSpaceSingle = 0,tomLineSpace1pt5 = 1,tomLineSpaceDouble = 2,tomLineSpaceAtLeast = 3,tomLineSpaceExactly = 4,
    tomLineSpaceMultiple = 5,tomAlignLeft = 0,tomAlignCenter = 1,tomAlignRight = 2,tomAlignJustify = 3,tomAlignDecimal = 3,tomAlignBar = 4,
    tomAlignInterWord = 3,tomAlignInterLetter = 4,tomAlignScaled = 5,tomAlignGlyphs = 6,tomAlignSnapGrid = 7,tomSpaces = 0,tomDots = 1,tomDashes = 2,
    tomLines = 3,tomThickLines = 4,tomEquals = 5,tomTabBack = -3,tomTabNext = -2,tomTabHere = -1,tomListNone = 0,tomListBullet = 1,
    tomListNumberAsArabic = 2,tomListNumberAsLCLetter = 3,tomListNumberAsUCLetter = 4,tomListNumberAsLCRoman = 5,tomListNumberAsUCRoman = 6,
    tomListNumberAsSequence = 7,tomListParentheses = 0x10000,tomListPeriod = 0x20000,tomListPlain = 0x30000,tomCharacter = 1,tomWord = 2,
    tomSentence = 3,tomParagraph = 4,tomLine = 5,tomStory = 6,tomScreen = 7,tomSection = 8,tomColumn = 9,tomRow = 10,tomWindow = 11,tomCell = 12,
    tomCharFormat = 13,tomParaFormat = 14,tomTable = 15,tomObject = 16,tomMatchWord = 2,tomMatchCase = 4,tomMatchPattern = 8,tomUnknownStory = 0,
    tomMainTextStory = 1,tomFootnotesStory = 2,tomEndnotesStory = 3,tomCommentsStory = 4,tomTextFrameStory = 5,tomEvenPagesHeaderStory = 6,
    tomPrimaryHeaderStory = 7,tomEvenPagesFooterStory = 8,tomPrimaryFooterStory = 9,tomFirstPageHeaderStory = 10,tomFirstPageFooterStory = 11,
    tomNoAnimation = 0,tomLasVegasLights = 1,tomBlinkingBackground = 2,tomSparkleText = 3,tomMarchingBlackAnts = 4,tomMarchingRedAnts = 5,
    tomShimmer = 6,tomWipeDown = 7,tomWipeRight = 8,tomAnimationMax = 8,tomLowerCase = 0,tomUpperCase = 1,tomTitleCase = 2,tomSentenceCase = 4,
    tomToggleCase = 5,tomReadOnly = 0x100,tomShareDenyRead = 0x200,tomShareDenyWrite = 0x400,tomPasteFile = 0x1000,tomCreateNew = 0x10,
    tomCreateAlways = 0x20,tomOpenExisting = 0x30,tomOpenAlways = 0x40,tomTruncateExisting = 0x50,tomRTF = 0x1,tomText = 0x2,tomHTML = 0x3,
    tomWordDocument = 0x4,tomBold = 0x80000001,tomItalic = 0x80000002,tomUnderline = 0x80000004,tomStrikeout = 0x80000008,tomProtected = 0x80000010,
    tomLink = 0x80000020,tomSmallCaps = 0x80000040,tomAllCaps = 0x80000080,tomHidden = 0x80000100,tomOutline = 0x80000200,tomShadow = 0x80000400,
    tomEmboss = 0x80000800,tomImprint = 0x80001000,tomDisabled = 0x80002000,tomRevised = 0x80004000,tomNormalCaret = 0,tomKoreanBlockCaret = 0x1,
    tomIncludeInset = 0x1,tomIgnoreCurrentFont = 0,tomMatchFontCharset = 0x1,tomMatchFontSignature = 0x2,tomCharset = 0x80000000,tomRE10Mode = 0x1,
    tomNoIME = 0x80000,tomSelfIME = 0x40000
  } tomConstants;

  EXTERN_C const IID LIBID_tom;
#ifndef __ITextDocument_INTERFACE_DEFINED__
#define __ITextDocument_INTERFACE_DEFINED__
  EXTERN_C const IID IID_ITextDocument;
#if defined(__cplusplus) && !defined(CINTERFACE)
  struct ITextDocument : public IDispatch {
  public:
    virtual HRESULT WINAPI GetName(BSTR *pName) = 0;
    virtual HRESULT WINAPI GetSelection(ITextSelection **ppSel) = 0;
    virtual HRESULT WINAPI GetStoryCount(long *pCount) = 0;
    virtual HRESULT WINAPI GetStoryRanges(ITextStoryRanges **ppStories) = 0;
    virtual HRESULT WINAPI GetSaved(long *pValue) = 0;
    virtual HRESULT WINAPI SetSaved(long Value) = 0;
    virtual HRESULT WINAPI GetDefaultTabStop(float *pValue) = 0;
    virtual HRESULT WINAPI SetDefaultTabStop(float Value) = 0;
    virtual HRESULT WINAPI New(void) = 0;
    virtual HRESULT WINAPI Open(VARIANT *pVar,long Flags,long CodePage) = 0;
    virtual HRESULT WINAPI Save(VARIANT *pVar,long Flags,long CodePage) = 0;
    virtual HRESULT WINAPI Freeze(long *pCount) = 0;
    virtual HRESULT WINAPI Unfreeze(long *pCount) = 0;
    virtual HRESULT WINAPI BeginEditCollection(void) = 0;
    virtual HRESULT WINAPI EndEditCollection(void) = 0;
    virtual HRESULT WINAPI Undo(long Count,long *prop) = 0;
    virtual HRESULT WINAPI Redo(long Count,long *prop) = 0;
    virtual HRESULT WINAPI Range(long cp1,long cp2,ITextRange **ppRange) = 0;
    virtual HRESULT WINAPI RangeFromPoint(long x,long y,ITextRange **ppRange) = 0;
  };
#else
  typedef struct ITextDocumentVtbl {
    BEGIN_INTERFACE
      HRESULT (WINAPI *QueryInterface)(ITextDocument *This,REFIID riid,void **ppvObject);
      ULONG (WINAPI *AddRef)(ITextDocument *This);
      ULONG (WINAPI *Release)(ITextDocument *This);
      HRESULT (WINAPI *GetTypeInfoCount)(ITextDocument *This,UINT *pctinfo);
      HRESULT (WINAPI *GetTypeInfo)(ITextDocument *This,UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo);
      HRESULT (WINAPI *GetIDsOfNames)(ITextDocument *This,REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId);
      HRESULT (WINAPI *Invoke)(ITextDocument *This,DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr);
      HRESULT (WINAPI *GetName)(ITextDocument *This,BSTR *pName);
      HRESULT (WINAPI *GetSelection)(ITextDocument *This,ITextSelection **ppSel);
      HRESULT (WINAPI *GetStoryCount)(ITextDocument *This,long *pCount);
      HRESULT (WINAPI *GetStoryRanges)(ITextDocument *This,ITextStoryRanges **ppStories);
      HRESULT (WINAPI *GetSaved)(ITextDocument *This,long *pValue);
      HRESULT (WINAPI *SetSaved)(ITextDocument *This,long Value);
      HRESULT (WINAPI *GetDefaultTabStop)(ITextDocument *This,float *pValue);
      HRESULT (WINAPI *SetDefaultTabStop)(ITextDocument *This,float Value);
      HRESULT (WINAPI *New)(ITextDocument *This);
      HRESULT (WINAPI *Open)(ITextDocument *This,VARIANT *pVar,long Flags,long CodePage);
      HRESULT (WINAPI *Save)(ITextDocument *This,VARIANT *pVar,long Flags,long CodePage);
      HRESULT (WINAPI *Freeze)(ITextDocument *This,long *pCount);
      HRESULT (WINAPI *Unfreeze)(ITextDocument *This,long *pCount);
      HRESULT (WINAPI *BeginEditCollection)(ITextDocument *This);
      HRESULT (WINAPI *EndEditCollection)(ITextDocument *This);
      HRESULT (WINAPI *Undo)(ITextDocument *This,long Count,long *prop);
      HRESULT (WINAPI *Redo)(ITextDocument *This,long Count,long *prop);
      HRESULT (WINAPI *Range)(ITextDocument *This,long cp1,long cp2,ITextRange **ppRange);
      HRESULT (WINAPI *RangeFromPoint)(ITextDocument *This,long x,long y,ITextRange **ppRange);
    END_INTERFACE
  } ITextDocumentVtbl;
  struct ITextDocument {
    CONST_VTBL struct ITextDocumentVtbl *lpVtbl;
  };
#ifdef COBJMACROS
#define ITextDocument_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define ITextDocument_AddRef(This) (This)->lpVtbl->AddRef(This)
#define ITextDocument_Release(This) (This)->lpVtbl->Release(This)
#define ITextDocument_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define ITextDocument_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ITextDocument_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ITextDocument_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define ITextDocument_GetName(This,pName) (This)->lpVtbl->GetName(This,pName)
#define ITextDocument_GetSelection(This,ppSel) (This)->lpVtbl->GetSelection(This,ppSel)
#define ITextDocument_GetStoryCount(This,pCount) (This)->lpVtbl->GetStoryCount(This,pCount)
#define ITextDocument_GetStoryRanges(This,ppStories) (This)->lpVtbl->GetStoryRanges(This,ppStories)
#define ITextDocument_GetSaved(This,pValue) (This)->lpVtbl->GetSaved(This,pValue)
#define ITextDocument_SetSaved(This,Value) (This)->lpVtbl->SetSaved(This,Value)
#define ITextDocument_GetDefaultTabStop(This,pValue) (This)->lpVtbl->GetDefaultTabStop(This,pValue)
#define ITextDocument_SetDefaultTabStop(This,Value) (This)->lpVtbl->SetDefaultTabStop(This,Value)
#define ITextDocument_New(This) (This)->lpVtbl->New(This)
#define ITextDocument_Open(This,pVar,Flags,CodePage) (This)->lpVtbl->Open(This,pVar,Flags,CodePage)
#define ITextDocument_Save(This,pVar,Flags,CodePage) (This)->lpVtbl->Save(This,pVar,Flags,CodePage)
#define ITextDocument_Freeze(This,pCount) (This)->lpVtbl->Freeze(This,pCount)
#define ITextDocument_Unfreeze(This,pCount) (This)->lpVtbl->Unfreeze(This,pCount)
#define ITextDocument_BeginEditCollection(This) (This)->lpVtbl->BeginEditCollection(This)
#define ITextDocument_EndEditCollection(This) (This)->lpVtbl->EndEditCollection(This)
#define ITextDocument_Undo(This,Count,prop) (This)->lpVtbl->Undo(This,Count,prop)
#define ITextDocument_Redo(This,Count,prop) (This)->lpVtbl->Redo(This,Count,prop)
#define ITextDocument_Range(This,cp1,cp2,ppRange) (This)->lpVtbl->Range(This,cp1,cp2,ppRange)
#define ITextDocument_RangeFromPoint(This,x,y,ppRange) (This)->lpVtbl->RangeFromPoint(This,x,y,ppRange)
#endif
#endif
  HRESULT WINAPI ITextDocument_GetName_Proxy(ITextDocument *This,BSTR *pName);
  void __RPC_STUB ITextDocument_GetName_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_GetSelection_Proxy(ITextDocument *This,ITextSelection **ppSel);
  void __RPC_STUB ITextDocument_GetSelection_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_GetStoryCount_Proxy(ITextDocument *This,long *pCount);
  void __RPC_STUB ITextDocument_GetStoryCount_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_GetStoryRanges_Proxy(ITextDocument *This,ITextStoryRanges **ppStories);
  void __RPC_STUB ITextDocument_GetStoryRanges_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_GetSaved_Proxy(ITextDocument *This,long *pValue);
  void __RPC_STUB ITextDocument_GetSaved_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_SetSaved_Proxy(ITextDocument *This,long Value);
  void __RPC_STUB ITextDocument_SetSaved_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_GetDefaultTabStop_Proxy(ITextDocument *This,float *pValue);
  void __RPC_STUB ITextDocument_GetDefaultTabStop_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_SetDefaultTabStop_Proxy(ITextDocument *This,float Value);
  void __RPC_STUB ITextDocument_SetDefaultTabStop_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_New_Proxy(ITextDocument *This);
  void __RPC_STUB ITextDocument_New_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_Open_Proxy(ITextDocument *This,VARIANT *pVar,long Flags,long CodePage);
  void __RPC_STUB ITextDocument_Open_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_Save_Proxy(ITextDocument *This,VARIANT *pVar,long Flags,long CodePage);
  void __RPC_STUB ITextDocument_Save_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_Freeze_Proxy(ITextDocument *This,long *pCount);
  void __RPC_STUB ITextDocument_Freeze_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_Unfreeze_Proxy(ITextDocument *This,long *pCount);
  void __RPC_STUB ITextDocument_Unfreeze_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_BeginEditCollection_Proxy(ITextDocument *This);
  void __RPC_STUB ITextDocument_BeginEditCollection_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_EndEditCollection_Proxy(ITextDocument *This);
  void __RPC_STUB ITextDocument_EndEditCollection_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_Undo_Proxy(ITextDocument *This,long Count,long *prop);
  void __RPC_STUB ITextDocument_Undo_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_Redo_Proxy(ITextDocument *This,long Count,long *prop);
  void __RPC_STUB ITextDocument_Redo_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_Range_Proxy(ITextDocument *This,long cp1,long cp2,ITextRange **ppRange);
  void __RPC_STUB ITextDocument_Range_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument_RangeFromPoint_Proxy(ITextDocument *This,long x,long y,ITextRange **ppRange);
  void __RPC_STUB ITextDocument_RangeFromPoint_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
#endif

#ifndef __ITextRange_INTERFACE_DEFINED__
#define __ITextRange_INTERFACE_DEFINED__
  EXTERN_C const IID IID_ITextRange;
#if defined(__cplusplus) && !defined(CINTERFACE)
  struct ITextRange : public IDispatch {
  public:
    virtual HRESULT WINAPI GetText(BSTR *pbstr) = 0;
    virtual HRESULT WINAPI SetText(BSTR bstr) = 0;
    virtual HRESULT WINAPI GetChar(long *pch) = 0;
    virtual HRESULT WINAPI SetChar(long ch) = 0;
    virtual HRESULT WINAPI GetDuplicate(ITextRange **ppRange) = 0;
    virtual HRESULT WINAPI GetFormattedText(ITextRange **ppRange) = 0;
    virtual HRESULT WINAPI SetFormattedText(ITextRange *pRange) = 0;
    virtual HRESULT WINAPI GetStart(long *pcpFirst) = 0;
    virtual HRESULT WINAPI SetStart(long cpFirst) = 0;
    virtual HRESULT WINAPI GetEnd(long *pcpLim) = 0;
    virtual HRESULT WINAPI SetEnd(long cpLim) = 0;
    virtual HRESULT WINAPI GetFont(ITextFont **pFont) = 0;
    virtual HRESULT WINAPI SetFont(ITextFont *pFont) = 0;
    virtual HRESULT WINAPI GetPara(ITextPara **pPara) = 0;
    virtual HRESULT WINAPI SetPara(ITextPara *pPara) = 0;
    virtual HRESULT WINAPI GetStoryLength(long *pcch) = 0;
    virtual HRESULT WINAPI GetStoryType(long *pValue) = 0;
    virtual HRESULT WINAPI Collapse(long bStart) = 0;
    virtual HRESULT WINAPI Expand(long Unit,long *pDelta) = 0;
    virtual HRESULT WINAPI GetIndex(long Unit,long *pIndex) = 0;
    virtual HRESULT WINAPI SetIndex(long Unit,long Index,long Extend) = 0;
    virtual HRESULT WINAPI SetRange(long cpActive,long cpOther) = 0;
    virtual HRESULT WINAPI InRange(ITextRange *pRange,long *pb) = 0;
    virtual HRESULT WINAPI InStory(ITextRange *pRange,long *pb) = 0;
    virtual HRESULT WINAPI IsEqual(ITextRange *pRange,long *pb) = 0;
    virtual HRESULT WINAPI Select(void) = 0;
    virtual HRESULT WINAPI StartOf(long Unit,long Extend,long *pDelta) = 0;
    virtual HRESULT WINAPI EndOf(long Unit,long Extend,long *pDelta) = 0;
    virtual HRESULT WINAPI Move(long Unit,long Count,long *pDelta) = 0;
    virtual HRESULT WINAPI MoveStart(long Unit,long Count,long *pDelta) = 0;
    virtual HRESULT WINAPI MoveEnd(long Unit,long Count,long *pDelta) = 0;
    virtual HRESULT WINAPI MoveWhile(VARIANT *Cset,long Count,long *pDelta) = 0;
    virtual HRESULT WINAPI MoveStartWhile(VARIANT *Cset,long Count,long *pDelta) = 0;
    virtual HRESULT WINAPI MoveEndWhile(VARIANT *Cset,long Count,long *pDelta) = 0;
    virtual HRESULT WINAPI MoveUntil(VARIANT *Cset,long Count,long *pDelta) = 0;
    virtual HRESULT WINAPI MoveStartUntil(VARIANT *Cset,long Count,long *pDelta) = 0;
    virtual HRESULT WINAPI MoveEndUntil(VARIANT *Cset,long Count,long *pDelta) = 0;
    virtual HRESULT WINAPI FindText(BSTR bstr,long cch,long Flags,long *pLength) = 0;
    virtual HRESULT WINAPI FindTextStart(BSTR bstr,long cch,long Flags,long *pLength) = 0;
    virtual HRESULT WINAPI FindTextEnd(BSTR bstr,long cch,long Flags,long *pLength) = 0;
    virtual HRESULT WINAPI Delete(long Unit,long Count,long *pDelta) = 0;
    virtual HRESULT WINAPI Cut(VARIANT *pVar) = 0;
    virtual HRESULT WINAPI Copy(VARIANT *pVar) = 0;
    virtual HRESULT WINAPI Paste(VARIANT *pVar,long Format) = 0;
    virtual HRESULT WINAPI CanPaste(VARIANT *pVar,long Format,long *pb) = 0;
    virtual HRESULT WINAPI CanEdit(long *pbCanEdit) = 0;
    virtual HRESULT WINAPI ChangeCase(long Type) = 0;
    virtual HRESULT WINAPI GetPoint(long Type,long *px,long *py) = 0;
    virtual HRESULT WINAPI SetPoint(long x,long y,long Type,long Extend) = 0;
    virtual HRESULT WINAPI ScrollIntoView(long Value) = 0;
    virtual HRESULT WINAPI GetEmbeddedObject(IUnknown **ppv) = 0;
  };
#else
  typedef struct ITextRangeVtbl {
    BEGIN_INTERFACE
      HRESULT (WINAPI *QueryInterface)(ITextRange *This,REFIID riid,void **ppvObject);
      ULONG (WINAPI *AddRef)(ITextRange *This);
      ULONG (WINAPI *Release)(ITextRange *This);
      HRESULT (WINAPI *GetTypeInfoCount)(ITextRange *This,UINT *pctinfo);
      HRESULT (WINAPI *GetTypeInfo)(ITextRange *This,UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo);
      HRESULT (WINAPI *GetIDsOfNames)(ITextRange *This,REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId);
      HRESULT (WINAPI *Invoke)(ITextRange *This,DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr);
      HRESULT (WINAPI *GetText)(ITextRange *This,BSTR *pbstr);
      HRESULT (WINAPI *SetText)(ITextRange *This,BSTR bstr);
      HRESULT (WINAPI *GetChar)(ITextRange *This,long *pch);
      HRESULT (WINAPI *SetChar)(ITextRange *This,long ch);
      HRESULT (WINAPI *GetDuplicate)(ITextRange *This,ITextRange **ppRange);
      HRESULT (WINAPI *GetFormattedText)(ITextRange *This,ITextRange **ppRange);
      HRESULT (WINAPI *SetFormattedText)(ITextRange *This,ITextRange *pRange);
      HRESULT (WINAPI *GetStart)(ITextRange *This,long *pcpFirst);
      HRESULT (WINAPI *SetStart)(ITextRange *This,long cpFirst);
      HRESULT (WINAPI *GetEnd)(ITextRange *This,long *pcpLim);
      HRESULT (WINAPI *SetEnd)(ITextRange *This,long cpLim);
      HRESULT (WINAPI *GetFont)(ITextRange *This,ITextFont **pFont);
      HRESULT (WINAPI *SetFont)(ITextRange *This,ITextFont *pFont);
      HRESULT (WINAPI *GetPara)(ITextRange *This,ITextPara **pPara);
      HRESULT (WINAPI *SetPara)(ITextRange *This,ITextPara *pPara);
      HRESULT (WINAPI *GetStoryLength)(ITextRange *This,long *pcch);
      HRESULT (WINAPI *GetStoryType)(ITextRange *This,long *pValue);
      HRESULT (WINAPI *Collapse)(ITextRange *This,long bStart);
      HRESULT (WINAPI *Expand)(ITextRange *This,long Unit,long *pDelta);
      HRESULT (WINAPI *GetIndex)(ITextRange *This,long Unit,long *pIndex);
      HRESULT (WINAPI *SetIndex)(ITextRange *This,long Unit,long Index,long Extend);
      HRESULT (WINAPI *SetRange)(ITextRange *This,long cpActive,long cpOther);
      HRESULT (WINAPI *InRange)(ITextRange *This,ITextRange *pRange,long *pb);
      HRESULT (WINAPI *InStory)(ITextRange *This,ITextRange *pRange,long *pb);
      HRESULT (WINAPI *IsEqual)(ITextRange *This,ITextRange *pRange,long *pb);
      HRESULT (WINAPI *Select)(ITextRange *This);
      HRESULT (WINAPI *StartOf)(ITextRange *This,long Unit,long Extend,long *pDelta);
      HRESULT (WINAPI *EndOf)(ITextRange *This,long Unit,long Extend,long *pDelta);
      HRESULT (WINAPI *Move)(ITextRange *This,long Unit,long Count,long *pDelta);
      HRESULT (WINAPI *MoveStart)(ITextRange *This,long Unit,long Count,long *pDelta);
      HRESULT (WINAPI *MoveEnd)(ITextRange *This,long Unit,long Count,long *pDelta);
      HRESULT (WINAPI *MoveWhile)(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *MoveStartWhile)(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *MoveEndWhile)(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *MoveUntil)(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *MoveStartUntil)(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *MoveEndUntil)(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *FindText)(ITextRange *This,BSTR bstr,long cch,long Flags,long *pLength);
      HRESULT (WINAPI *FindTextStart)(ITextRange *This,BSTR bstr,long cch,long Flags,long *pLength);
      HRESULT (WINAPI *FindTextEnd)(ITextRange *This,BSTR bstr,long cch,long Flags,long *pLength);
      HRESULT (WINAPI *Delete)(ITextRange *This,long Unit,long Count,long *pDelta);
      HRESULT (WINAPI *Cut)(ITextRange *This,VARIANT *pVar);
      HRESULT (WINAPI *Copy)(ITextRange *This,VARIANT *pVar);
      HRESULT (WINAPI *Paste)(ITextRange *This,VARIANT *pVar,long Format);
      HRESULT (WINAPI *CanPaste)(ITextRange *This,VARIANT *pVar,long Format,long *pb);
      HRESULT (WINAPI *CanEdit)(ITextRange *This,long *pbCanEdit);
      HRESULT (WINAPI *ChangeCase)(ITextRange *This,long Type);
      HRESULT (WINAPI *GetPoint)(ITextRange *This,long Type,long *px,long *py);
      HRESULT (WINAPI *SetPoint)(ITextRange *This,long x,long y,long Type,long Extend);
      HRESULT (WINAPI *ScrollIntoView)(ITextRange *This,long Value);
      HRESULT (WINAPI *GetEmbeddedObject)(ITextRange *This,IUnknown **ppv);
    END_INTERFACE
  } ITextRangeVtbl;
  struct ITextRange {
    CONST_VTBL struct ITextRangeVtbl *lpVtbl;
  };
#ifdef COBJMACROS
#define ITextRange_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define ITextRange_AddRef(This) (This)->lpVtbl->AddRef(This)
#define ITextRange_Release(This) (This)->lpVtbl->Release(This)
#define ITextRange_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define ITextRange_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ITextRange_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ITextRange_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define ITextRange_GetText(This,pbstr) (This)->lpVtbl->GetText(This,pbstr)
#define ITextRange_SetText(This,bstr) (This)->lpVtbl->SetText(This,bstr)
#define ITextRange_GetChar(This,pch) (This)->lpVtbl->GetChar(This,pch)
#define ITextRange_SetChar(This,ch) (This)->lpVtbl->SetChar(This,ch)
#define ITextRange_GetDuplicate(This,ppRange) (This)->lpVtbl->GetDuplicate(This,ppRange)
#define ITextRange_GetFormattedText(This,ppRange) (This)->lpVtbl->GetFormattedText(This,ppRange)
#define ITextRange_SetFormattedText(This,pRange) (This)->lpVtbl->SetFormattedText(This,pRange)
#define ITextRange_GetStart(This,pcpFirst) (This)->lpVtbl->GetStart(This,pcpFirst)
#define ITextRange_SetStart(This,cpFirst) (This)->lpVtbl->SetStart(This,cpFirst)
#define ITextRange_GetEnd(This,pcpLim) (This)->lpVtbl->GetEnd(This,pcpLim)
#define ITextRange_SetEnd(This,cpLim) (This)->lpVtbl->SetEnd(This,cpLim)
#define ITextRange_GetFont(This,pFont) (This)->lpVtbl->GetFont(This,pFont)
#define ITextRange_SetFont(This,pFont) (This)->lpVtbl->SetFont(This,pFont)
#define ITextRange_GetPara(This,pPara) (This)->lpVtbl->GetPara(This,pPara)
#define ITextRange_SetPara(This,pPara) (This)->lpVtbl->SetPara(This,pPara)
#define ITextRange_GetStoryLength(This,pcch) (This)->lpVtbl->GetStoryLength(This,pcch)
#define ITextRange_GetStoryType(This,pValue) (This)->lpVtbl->GetStoryType(This,pValue)
#define ITextRange_Collapse(This,bStart) (This)->lpVtbl->Collapse(This,bStart)
#define ITextRange_Expand(This,Unit,pDelta) (This)->lpVtbl->Expand(This,Unit,pDelta)
#define ITextRange_GetIndex(This,Unit,pIndex) (This)->lpVtbl->GetIndex(This,Unit,pIndex)
#define ITextRange_SetIndex(This,Unit,Index,Extend) (This)->lpVtbl->SetIndex(This,Unit,Index,Extend)
#define ITextRange_SetRange(This,cpActive,cpOther) (This)->lpVtbl->SetRange(This,cpActive,cpOther)
#define ITextRange_InRange(This,pRange,pb) (This)->lpVtbl->InRange(This,pRange,pb)
#define ITextRange_InStory(This,pRange,pb) (This)->lpVtbl->InStory(This,pRange,pb)
#define ITextRange_IsEqual(This,pRange,pb) (This)->lpVtbl->IsEqual(This,pRange,pb)
#define ITextRange_Select(This) (This)->lpVtbl->Select(This)
#define ITextRange_StartOf(This,Unit,Extend,pDelta) (This)->lpVtbl->StartOf(This,Unit,Extend,pDelta)
#define ITextRange_EndOf(This,Unit,Extend,pDelta) (This)->lpVtbl->EndOf(This,Unit,Extend,pDelta)
#define ITextRange_Move(This,Unit,Count,pDelta) (This)->lpVtbl->Move(This,Unit,Count,pDelta)
#define ITextRange_MoveStart(This,Unit,Count,pDelta) (This)->lpVtbl->MoveStart(This,Unit,Count,pDelta)
#define ITextRange_MoveEnd(This,Unit,Count,pDelta) (This)->lpVtbl->MoveEnd(This,Unit,Count,pDelta)
#define ITextRange_MoveWhile(This,Cset,Count,pDelta) (This)->lpVtbl->MoveWhile(This,Cset,Count,pDelta)
#define ITextRange_MoveStartWhile(This,Cset,Count,pDelta) (This)->lpVtbl->MoveStartWhile(This,Cset,Count,pDelta)
#define ITextRange_MoveEndWhile(This,Cset,Count,pDelta) (This)->lpVtbl->MoveEndWhile(This,Cset,Count,pDelta)
#define ITextRange_MoveUntil(This,Cset,Count,pDelta) (This)->lpVtbl->MoveUntil(This,Cset,Count,pDelta)
#define ITextRange_MoveStartUntil(This,Cset,Count,pDelta) (This)->lpVtbl->MoveStartUntil(This,Cset,Count,pDelta)
#define ITextRange_MoveEndUntil(This,Cset,Count,pDelta) (This)->lpVtbl->MoveEndUntil(This,Cset,Count,pDelta)
#define ITextRange_FindText(This,bstr,cch,Flags,pLength) (This)->lpVtbl->FindText(This,bstr,cch,Flags,pLength)
#define ITextRange_FindTextStart(This,bstr,cch,Flags,pLength) (This)->lpVtbl->FindTextStart(This,bstr,cch,Flags,pLength)
#define ITextRange_FindTextEnd(This,bstr,cch,Flags,pLength) (This)->lpVtbl->FindTextEnd(This,bstr,cch,Flags,pLength)
#define ITextRange_Delete(This,Unit,Count,pDelta) (This)->lpVtbl->Delete(This,Unit,Count,pDelta)
#define ITextRange_Cut(This,pVar) (This)->lpVtbl->Cut(This,pVar)
#define ITextRange_Copy(This,pVar) (This)->lpVtbl->Copy(This,pVar)
#define ITextRange_Paste(This,pVar,Format) (This)->lpVtbl->Paste(This,pVar,Format)
#define ITextRange_CanPaste(This,pVar,Format,pb) (This)->lpVtbl->CanPaste(This,pVar,Format,pb)
#define ITextRange_CanEdit(This,pbCanEdit) (This)->lpVtbl->CanEdit(This,pbCanEdit)
#define ITextRange_ChangeCase(This,Type) (This)->lpVtbl->ChangeCase(This,Type)
#define ITextRange_GetPoint(This,Type,px,py) (This)->lpVtbl->GetPoint(This,Type,px,py)
#define ITextRange_SetPoint(This,x,y,Type,Extend) (This)->lpVtbl->SetPoint(This,x,y,Type,Extend)
#define ITextRange_ScrollIntoView(This,Value) (This)->lpVtbl->ScrollIntoView(This,Value)
#define ITextRange_GetEmbeddedObject(This,ppv) (This)->lpVtbl->GetEmbeddedObject(This,ppv)
#endif
#endif
  HRESULT WINAPI ITextRange_GetText_Proxy(ITextRange *This,BSTR *pbstr);
  void __RPC_STUB ITextRange_GetText_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_SetText_Proxy(ITextRange *This,BSTR bstr);
  void __RPC_STUB ITextRange_SetText_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetChar_Proxy(ITextRange *This,long *pch);
  void __RPC_STUB ITextRange_GetChar_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_SetChar_Proxy(ITextRange *This,long ch);
  void __RPC_STUB ITextRange_SetChar_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetDuplicate_Proxy(ITextRange *This,ITextRange **ppRange);
  void __RPC_STUB ITextRange_GetDuplicate_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetFormattedText_Proxy(ITextRange *This,ITextRange **ppRange);
  void __RPC_STUB ITextRange_GetFormattedText_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_SetFormattedText_Proxy(ITextRange *This,ITextRange *pRange);
  void __RPC_STUB ITextRange_SetFormattedText_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetStart_Proxy(ITextRange *This,long *pcpFirst);
  void __RPC_STUB ITextRange_GetStart_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_SetStart_Proxy(ITextRange *This,long cpFirst);
  void __RPC_STUB ITextRange_SetStart_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetEnd_Proxy(ITextRange *This,long *pcpLim);
  void __RPC_STUB ITextRange_GetEnd_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_SetEnd_Proxy(ITextRange *This,long cpLim);
  void __RPC_STUB ITextRange_SetEnd_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetFont_Proxy(ITextRange *This,ITextFont **pFont);
  void __RPC_STUB ITextRange_GetFont_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_SetFont_Proxy(ITextRange *This,ITextFont *pFont);
  void __RPC_STUB ITextRange_SetFont_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetPara_Proxy(ITextRange *This,ITextPara **pPara);
  void __RPC_STUB ITextRange_GetPara_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_SetPara_Proxy(ITextRange *This,ITextPara *pPara);
  void __RPC_STUB ITextRange_SetPara_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetStoryLength_Proxy(ITextRange *This,long *pcch);
  void __RPC_STUB ITextRange_GetStoryLength_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetStoryType_Proxy(ITextRange *This,long *pValue);
  void __RPC_STUB ITextRange_GetStoryType_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_Collapse_Proxy(ITextRange *This,long bStart);
  void __RPC_STUB ITextRange_Collapse_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_Expand_Proxy(ITextRange *This,long Unit,long *pDelta);
  void __RPC_STUB ITextRange_Expand_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetIndex_Proxy(ITextRange *This,long Unit,long *pIndex);
  void __RPC_STUB ITextRange_GetIndex_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_SetIndex_Proxy(ITextRange *This,long Unit,long Index,long Extend);
  void __RPC_STUB ITextRange_SetIndex_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_SetRange_Proxy(ITextRange *This,long cpActive,long cpOther);
  void __RPC_STUB ITextRange_SetRange_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_InRange_Proxy(ITextRange *This,ITextRange *pRange,long *pb);
  void __RPC_STUB ITextRange_InRange_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_InStory_Proxy(ITextRange *This,ITextRange *pRange,long *pb);
  void __RPC_STUB ITextRange_InStory_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_IsEqual_Proxy(ITextRange *This,ITextRange *pRange,long *pb);
  void __RPC_STUB ITextRange_IsEqual_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_Select_Proxy(ITextRange *This);
  void __RPC_STUB ITextRange_Select_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_StartOf_Proxy(ITextRange *This,long Unit,long Extend,long *pDelta);
  void __RPC_STUB ITextRange_StartOf_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_EndOf_Proxy(ITextRange *This,long Unit,long Extend,long *pDelta);
  void __RPC_STUB ITextRange_EndOf_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_Move_Proxy(ITextRange *This,long Unit,long Count,long *pDelta);
  void __RPC_STUB ITextRange_Move_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_MoveStart_Proxy(ITextRange *This,long Unit,long Count,long *pDelta);
  void __RPC_STUB ITextRange_MoveStart_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_MoveEnd_Proxy(ITextRange *This,long Unit,long Count,long *pDelta);
  void __RPC_STUB ITextRange_MoveEnd_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_MoveWhile_Proxy(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
  void __RPC_STUB ITextRange_MoveWhile_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_MoveStartWhile_Proxy(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
  void __RPC_STUB ITextRange_MoveStartWhile_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_MoveEndWhile_Proxy(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
  void __RPC_STUB ITextRange_MoveEndWhile_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_MoveUntil_Proxy(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
  void __RPC_STUB ITextRange_MoveUntil_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_MoveStartUntil_Proxy(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
  void __RPC_STUB ITextRange_MoveStartUntil_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_MoveEndUntil_Proxy(ITextRange *This,VARIANT *Cset,long Count,long *pDelta);
  void __RPC_STUB ITextRange_MoveEndUntil_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_FindText_Proxy(ITextRange *This,BSTR bstr,long cch,long Flags,long *pLength);
  void __RPC_STUB ITextRange_FindText_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_FindTextStart_Proxy(ITextRange *This,BSTR bstr,long cch,long Flags,long *pLength);
  void __RPC_STUB ITextRange_FindTextStart_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_FindTextEnd_Proxy(ITextRange *This,BSTR bstr,long cch,long Flags,long *pLength);
  void __RPC_STUB ITextRange_FindTextEnd_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_Delete_Proxy(ITextRange *This,long Unit,long Count,long *pDelta);
  void __RPC_STUB ITextRange_Delete_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_Cut_Proxy(ITextRange *This,VARIANT *pVar);
  void __RPC_STUB ITextRange_Cut_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_Copy_Proxy(ITextRange *This,VARIANT *pVar);
  void __RPC_STUB ITextRange_Copy_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_Paste_Proxy(ITextRange *This,VARIANT *pVar,long Format);
  void __RPC_STUB ITextRange_Paste_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_CanPaste_Proxy(ITextRange *This,VARIANT *pVar,long Format,long *pb);
  void __RPC_STUB ITextRange_CanPaste_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_CanEdit_Proxy(ITextRange *This,long *pbCanEdit);
  void __RPC_STUB ITextRange_CanEdit_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_ChangeCase_Proxy(ITextRange *This,long Type);
  void __RPC_STUB ITextRange_ChangeCase_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetPoint_Proxy(ITextRange *This,long Type,long *px,long *py);
  void __RPC_STUB ITextRange_GetPoint_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_SetPoint_Proxy(ITextRange *This,long x,long y,long Type,long Extend);
  void __RPC_STUB ITextRange_SetPoint_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_ScrollIntoView_Proxy(ITextRange *This,long Value);
  void __RPC_STUB ITextRange_ScrollIntoView_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextRange_GetEmbeddedObject_Proxy(ITextRange *This,IUnknown **ppv);
  void __RPC_STUB ITextRange_GetEmbeddedObject_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
#endif

#ifndef __ITextSelection_INTERFACE_DEFINED__
#define __ITextSelection_INTERFACE_DEFINED__
  EXTERN_C const IID IID_ITextSelection;
#if defined(__cplusplus) && !defined(CINTERFACE)
  struct ITextSelection : public ITextRange {
  public:
    virtual HRESULT WINAPI GetFlags(long *pFlags) = 0;
    virtual HRESULT WINAPI SetFlags(LONG Flags) = 0;
    virtual HRESULT WINAPI GetType(long *pType) = 0;
    virtual HRESULT WINAPI MoveLeft(long Unit,long Count,long Extend,long *pDelta) = 0;
    virtual HRESULT WINAPI MoveRight(long Unit,long Count,long Extend,long *pDelta) = 0;
    virtual HRESULT WINAPI MoveUp(long Unit,long Count,long Extend,long *pDelta) = 0;
    virtual HRESULT WINAPI MoveDown(long Unit,long Count,long Extend,long *pDelta) = 0;
    virtual HRESULT WINAPI HomeKey(long Unit,long Extend,long *pDelta) = 0;
    virtual HRESULT WINAPI EndKey(long Unit,long Extend,long *pDelta) = 0;
    virtual HRESULT WINAPI TypeText(BSTR bstr) = 0;
  };
#else
  typedef struct ITextSelectionVtbl {
    BEGIN_INTERFACE
      HRESULT (WINAPI *QueryInterface)(ITextSelection *This,REFIID riid,void **ppvObject);
      ULONG (WINAPI *AddRef)(ITextSelection *This);
      ULONG (WINAPI *Release)(ITextSelection *This);
      HRESULT (WINAPI *GetTypeInfoCount)(ITextSelection *This,UINT *pctinfo);
      HRESULT (WINAPI *GetTypeInfo)(ITextSelection *This,UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo);
      HRESULT (WINAPI *GetIDsOfNames)(ITextSelection *This,REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId);
      HRESULT (WINAPI *Invoke)(ITextSelection *This,DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr);
      HRESULT (WINAPI *GetText)(ITextSelection *This,BSTR *pbstr);
      HRESULT (WINAPI *SetText)(ITextSelection *This,BSTR bstr);
      HRESULT (WINAPI *GetChar)(ITextSelection *This,long *pch);
      HRESULT (WINAPI *SetChar)(ITextSelection *This,long ch);
      HRESULT (WINAPI *GetDuplicate)(ITextSelection *This,ITextRange **ppRange);
      HRESULT (WINAPI *GetFormattedText)(ITextSelection *This,ITextRange **ppRange);
      HRESULT (WINAPI *SetFormattedText)(ITextSelection *This,ITextRange *pRange);
      HRESULT (WINAPI *GetStart)(ITextSelection *This,long *pcpFirst);
      HRESULT (WINAPI *SetStart)(ITextSelection *This,long cpFirst);
      HRESULT (WINAPI *GetEnd)(ITextSelection *This,long *pcpLim);
      HRESULT (WINAPI *SetEnd)(ITextSelection *This,long cpLim);
      HRESULT (WINAPI *GetFont)(ITextSelection *This,ITextFont **pFont);
      HRESULT (WINAPI *SetFont)(ITextSelection *This,ITextFont *pFont);
      HRESULT (WINAPI *GetPara)(ITextSelection *This,ITextPara **pPara);
      HRESULT (WINAPI *SetPara)(ITextSelection *This,ITextPara *pPara);
      HRESULT (WINAPI *GetStoryLength)(ITextSelection *This,long *pcch);
      HRESULT (WINAPI *GetStoryType)(ITextSelection *This,long *pValue);
      HRESULT (WINAPI *Collapse)(ITextSelection *This,long bStart);
      HRESULT (WINAPI *Expand)(ITextSelection *This,long Unit,long *pDelta);
      HRESULT (WINAPI *GetIndex)(ITextSelection *This,long Unit,long *pIndex);
      HRESULT (WINAPI *SetIndex)(ITextSelection *This,long Unit,long Index,long Extend);
      HRESULT (WINAPI *SetRange)(ITextSelection *This,long cpActive,long cpOther);
      HRESULT (WINAPI *InRange)(ITextSelection *This,ITextRange *pRange,long *pb);
      HRESULT (WINAPI *InStory)(ITextSelection *This,ITextRange *pRange,long *pb);
      HRESULT (WINAPI *IsEqual)(ITextSelection *This,ITextRange *pRange,long *pb);
      HRESULT (WINAPI *Select)(ITextSelection *This);
      HRESULT (WINAPI *StartOf)(ITextSelection *This,long Unit,long Extend,long *pDelta);
      HRESULT (WINAPI *EndOf)(ITextSelection *This,long Unit,long Extend,long *pDelta);
      HRESULT (WINAPI *Move)(ITextSelection *This,long Unit,long Count,long *pDelta);
      HRESULT (WINAPI *MoveStart)(ITextSelection *This,long Unit,long Count,long *pDelta);
      HRESULT (WINAPI *MoveEnd)(ITextSelection *This,long Unit,long Count,long *pDelta);
      HRESULT (WINAPI *MoveWhile)(ITextSelection *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *MoveStartWhile)(ITextSelection *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *MoveEndWhile)(ITextSelection *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *MoveUntil)(ITextSelection *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *MoveStartUntil)(ITextSelection *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *MoveEndUntil)(ITextSelection *This,VARIANT *Cset,long Count,long *pDelta);
      HRESULT (WINAPI *FindText)(ITextSelection *This,BSTR bstr,long cch,long Flags,long *pLength);
      HRESULT (WINAPI *FindTextStart)(ITextSelection *This,BSTR bstr,long cch,long Flags,long *pLength);
      HRESULT (WINAPI *FindTextEnd)(ITextSelection *This,BSTR bstr,long cch,long Flags,long *pLength);
      HRESULT (WINAPI *Delete)(ITextSelection *This,long Unit,long Count,long *pDelta);
      HRESULT (WINAPI *Cut)(ITextSelection *This,VARIANT *pVar);
      HRESULT (WINAPI *Copy)(ITextSelection *This,VARIANT *pVar);
      HRESULT (WINAPI *Paste)(ITextSelection *This,VARIANT *pVar,long Format);
      HRESULT (WINAPI *CanPaste)(ITextSelection *This,VARIANT *pVar,long Format,long *pb);
      HRESULT (WINAPI *CanEdit)(ITextSelection *This,long *pbCanEdit);
      HRESULT (WINAPI *ChangeCase)(ITextSelection *This,long Type);
      HRESULT (WINAPI *GetPoint)(ITextSelection *This,long Type,long *px,long *py);
      HRESULT (WINAPI *SetPoint)(ITextSelection *This,long x,long y,long Type,long Extend);
      HRESULT (WINAPI *ScrollIntoView)(ITextSelection *This,long Value);
      HRESULT (WINAPI *GetEmbeddedObject)(ITextSelection *This,IUnknown **ppv);
      HRESULT (WINAPI *GetFlags)(ITextSelection *This,long *pFlags);
      HRESULT (WINAPI *SetFlags)(ITextSelection *This,LONG Flags);
      HRESULT (WINAPI *GetType)(ITextSelection *This,long *pType);
      HRESULT (WINAPI *MoveLeft)(ITextSelection *This,long Unit,long Count,long Extend,long *pDelta);
      HRESULT (WINAPI *MoveRight)(ITextSelection *This,long Unit,long Count,long Extend,long *pDelta);
      HRESULT (WINAPI *MoveUp)(ITextSelection *This,long Unit,long Count,long Extend,long *pDelta);
      HRESULT (WINAPI *MoveDown)(ITextSelection *This,long Unit,long Count,long Extend,long *pDelta);
      HRESULT (WINAPI *HomeKey)(ITextSelection *This,long Unit,long Extend,long *pDelta);
      HRESULT (WINAPI *EndKey)(ITextSelection *This,long Unit,long Extend,long *pDelta);
      HRESULT (WINAPI *TypeText)(ITextSelection *This,BSTR bstr);
    END_INTERFACE
  } ITextSelectionVtbl;

  struct ITextSelection {
    CONST_VTBL struct ITextSelectionVtbl *lpVtbl;
  };
#ifdef COBJMACROS
#define ITextSelection_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define ITextSelection_AddRef(This) (This)->lpVtbl->AddRef(This)
#define ITextSelection_Release(This) (This)->lpVtbl->Release(This)
#define ITextSelection_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define ITextSelection_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ITextSelection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ITextSelection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define ITextSelection_GetText(This,pbstr) (This)->lpVtbl->GetText(This,pbstr)
#define ITextSelection_SetText(This,bstr) (This)->lpVtbl->SetText(This,bstr)
#define ITextSelection_GetChar(This,pch) (This)->lpVtbl->GetChar(This,pch)
#define ITextSelection_SetChar(This,ch) (This)->lpVtbl->SetChar(This,ch)
#define ITextSelection_GetDuplicate(This,ppRange) (This)->lpVtbl->GetDuplicate(This,ppRange)
#define ITextSelection_GetFormattedText(This,ppRange) (This)->lpVtbl->GetFormattedText(This,ppRange)
#define ITextSelection_SetFormattedText(This,pRange) (This)->lpVtbl->SetFormattedText(This,pRange)
#define ITextSelection_GetStart(This,pcpFirst) (This)->lpVtbl->GetStart(This,pcpFirst)
#define ITextSelection_SetStart(This,cpFirst) (This)->lpVtbl->SetStart(This,cpFirst)
#define ITextSelection_GetEnd(This,pcpLim) (This)->lpVtbl->GetEnd(This,pcpLim)
#define ITextSelection_SetEnd(This,cpLim) (This)->lpVtbl->SetEnd(This,cpLim)
#define ITextSelection_GetFont(This,pFont) (This)->lpVtbl->GetFont(This,pFont)
#define ITextSelection_SetFont(This,pFont) (This)->lpVtbl->SetFont(This,pFont)
#define ITextSelection_GetPara(This,pPara) (This)->lpVtbl->GetPara(This,pPara)
#define ITextSelection_SetPara(This,pPara) (This)->lpVtbl->SetPara(This,pPara)
#define ITextSelection_GetStoryLength(This,pcch) (This)->lpVtbl->GetStoryLength(This,pcch)
#define ITextSelection_GetStoryType(This,pValue) (This)->lpVtbl->GetStoryType(This,pValue)
#define ITextSelection_Collapse(This,bStart) (This)->lpVtbl->Collapse(This,bStart)
#define ITextSelection_Expand(This,Unit,pDelta) (This)->lpVtbl->Expand(This,Unit,pDelta)
#define ITextSelection_GetIndex(This,Unit,pIndex) (This)->lpVtbl->GetIndex(This,Unit,pIndex)
#define ITextSelection_SetIndex(This,Unit,Index,Extend) (This)->lpVtbl->SetIndex(This,Unit,Index,Extend)
#define ITextSelection_SetRange(This,cpActive,cpOther) (This)->lpVtbl->SetRange(This,cpActive,cpOther)
#define ITextSelection_InRange(This,pRange,pb) (This)->lpVtbl->InRange(This,pRange,pb)
#define ITextSelection_InStory(This,pRange,pb) (This)->lpVtbl->InStory(This,pRange,pb)
#define ITextSelection_IsEqual(This,pRange,pb) (This)->lpVtbl->IsEqual(This,pRange,pb)
#define ITextSelection_Select(This) (This)->lpVtbl->Select(This)
#define ITextSelection_StartOf(This,Unit,Extend,pDelta) (This)->lpVtbl->StartOf(This,Unit,Extend,pDelta)
#define ITextSelection_EndOf(This,Unit,Extend,pDelta) (This)->lpVtbl->EndOf(This,Unit,Extend,pDelta)
#define ITextSelection_Move(This,Unit,Count,pDelta) (This)->lpVtbl->Move(This,Unit,Count,pDelta)
#define ITextSelection_MoveStart(This,Unit,Count,pDelta) (This)->lpVtbl->MoveStart(This,Unit,Count,pDelta)
#define ITextSelection_MoveEnd(This,Unit,Count,pDelta) (This)->lpVtbl->MoveEnd(This,Unit,Count,pDelta)
#define ITextSelection_MoveWhile(This,Cset,Count,pDelta) (This)->lpVtbl->MoveWhile(This,Cset,Count,pDelta)
#define ITextSelection_MoveStartWhile(This,Cset,Count,pDelta) (This)->lpVtbl->MoveStartWhile(This,Cset,Count,pDelta)
#define ITextSelection_MoveEndWhile(This,Cset,Count,pDelta) (This)->lpVtbl->MoveEndWhile(This,Cset,Count,pDelta)
#define ITextSelection_MoveUntil(This,Cset,Count,pDelta) (This)->lpVtbl->MoveUntil(This,Cset,Count,pDelta)
#define ITextSelection_MoveStartUntil(This,Cset,Count,pDelta) (This)->lpVtbl->MoveStartUntil(This,Cset,Count,pDelta)
#define ITextSelection_MoveEndUntil(This,Cset,Count,pDelta) (This)->lpVtbl->MoveEndUntil(This,Cset,Count,pDelta)
#define ITextSelection_FindText(This,bstr,cch,Flags,pLength) (This)->lpVtbl->FindText(This,bstr,cch,Flags,pLength)
#define ITextSelection_FindTextStart(This,bstr,cch,Flags,pLength) (This)->lpVtbl->FindTextStart(This,bstr,cch,Flags,pLength)
#define ITextSelection_FindTextEnd(This,bstr,cch,Flags,pLength) (This)->lpVtbl->FindTextEnd(This,bstr,cch,Flags,pLength)
#define ITextSelection_Delete(This,Unit,Count,pDelta) (This)->lpVtbl->Delete(This,Unit,Count,pDelta)
#define ITextSelection_Cut(This,pVar) (This)->lpVtbl->Cut(This,pVar)
#define ITextSelection_Copy(This,pVar) (This)->lpVtbl->Copy(This,pVar)
#define ITextSelection_Paste(This,pVar,Format) (This)->lpVtbl->Paste(This,pVar,Format)
#define ITextSelection_CanPaste(This,pVar,Format,pb) (This)->lpVtbl->CanPaste(This,pVar,Format,pb)
#define ITextSelection_CanEdit(This,pbCanEdit) (This)->lpVtbl->CanEdit(This,pbCanEdit)
#define ITextSelection_ChangeCase(This,Type) (This)->lpVtbl->ChangeCase(This,Type)
#define ITextSelection_GetPoint(This,Type,px,py) (This)->lpVtbl->GetPoint(This,Type,px,py)
#define ITextSelection_SetPoint(This,x,y,Type,Extend) (This)->lpVtbl->SetPoint(This,x,y,Type,Extend)
#define ITextSelection_ScrollIntoView(This,Value) (This)->lpVtbl->ScrollIntoView(This,Value)
#define ITextSelection_GetEmbeddedObject(This,ppv) (This)->lpVtbl->GetEmbeddedObject(This,ppv)
#define ITextSelection_GetFlags(This,pFlags) (This)->lpVtbl->GetFlags(This,pFlags)
#define ITextSelection_SetFlags(This,Flags) (This)->lpVtbl->SetFlags(This,Flags)
#define ITextSelection_GetType(This,pType) (This)->lpVtbl->GetType(This,pType)
#define ITextSelection_MoveLeft(This,Unit,Count,Extend,pDelta) (This)->lpVtbl->MoveLeft(This,Unit,Count,Extend,pDelta)
#define ITextSelection_MoveRight(This,Unit,Count,Extend,pDelta) (This)->lpVtbl->MoveRight(This,Unit,Count,Extend,pDelta)
#define ITextSelection_MoveUp(This,Unit,Count,Extend,pDelta) (This)->lpVtbl->MoveUp(This,Unit,Count,Extend,pDelta)
#define ITextSelection_MoveDown(This,Unit,Count,Extend,pDelta) (This)->lpVtbl->MoveDown(This,Unit,Count,Extend,pDelta)
#define ITextSelection_HomeKey(This,Unit,Extend,pDelta) (This)->lpVtbl->HomeKey(This,Unit,Extend,pDelta)
#define ITextSelection_EndKey(This,Unit,Extend,pDelta) (This)->lpVtbl->EndKey(This,Unit,Extend,pDelta)
#define ITextSelection_TypeText(This,bstr) (This)->lpVtbl->TypeText(This,bstr)
#endif
#endif
  HRESULT WINAPI ITextSelection_GetFlags_Proxy(ITextSelection *This,long *pFlags);
  void __RPC_STUB ITextSelection_GetFlags_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextSelection_SetFlags_Proxy(ITextSelection *This,LONG Flags);
  void __RPC_STUB ITextSelection_SetFlags_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextSelection_GetType_Proxy(ITextSelection *This,long *pType);
  void __RPC_STUB ITextSelection_GetType_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextSelection_MoveLeft_Proxy(ITextSelection *This,long Unit,long Count,long Extend,long *pDelta);
  void __RPC_STUB ITextSelection_MoveLeft_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextSelection_MoveRight_Proxy(ITextSelection *This,long Unit,long Count,long Extend,long *pDelta);
  void __RPC_STUB ITextSelection_MoveRight_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextSelection_MoveUp_Proxy(ITextSelection *This,long Unit,long Count,long Extend,long *pDelta);
  void __RPC_STUB ITextSelection_MoveUp_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextSelection_MoveDown_Proxy(ITextSelection *This,long Unit,long Count,long Extend,long *pDelta);
  void __RPC_STUB ITextSelection_MoveDown_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextSelection_HomeKey_Proxy(ITextSelection *This,long Unit,long Extend,long *pDelta);
  void __RPC_STUB ITextSelection_HomeKey_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextSelection_EndKey_Proxy(ITextSelection *This,long Unit,long Extend,long *pDelta);
  void __RPC_STUB ITextSelection_EndKey_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextSelection_TypeText_Proxy(ITextSelection *This,BSTR bstr);
  void __RPC_STUB ITextSelection_TypeText_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
#endif

#ifndef __ITextFont_INTERFACE_DEFINED__
#define __ITextFont_INTERFACE_DEFINED__
  EXTERN_C const IID IID_ITextFont;
#if defined(__cplusplus) && !defined(CINTERFACE)
  struct ITextFont : public IDispatch {
  public:
    virtual HRESULT WINAPI GetDuplicate(ITextFont **ppFont) = 0;
    virtual HRESULT WINAPI SetDuplicate(ITextFont *pFont) = 0;
    virtual HRESULT WINAPI CanChange(long *pB) = 0;
    virtual HRESULT WINAPI IsEqual(ITextFont *pFont,long *pB) = 0;
    virtual HRESULT WINAPI Reset(long Value) = 0;
    virtual HRESULT WINAPI GetStyle(long *pValue) = 0;
    virtual HRESULT WINAPI SetStyle(long Value) = 0;
    virtual HRESULT WINAPI GetAllCaps(long *pValue) = 0;
    virtual HRESULT WINAPI SetAllCaps(long Value) = 0;
    virtual HRESULT WINAPI GetAnimation(long *pValue) = 0;
    virtual HRESULT WINAPI SetAnimation(long Value) = 0;
    virtual HRESULT WINAPI GetBackColor(long *pValue) = 0;
    virtual HRESULT WINAPI SetBackColor(long Value) = 0;
    virtual HRESULT WINAPI GetBold(long *pValue) = 0;
    virtual HRESULT WINAPI SetBold(long Value) = 0;
    virtual HRESULT WINAPI GetEmboss(long *pValue) = 0;
    virtual HRESULT WINAPI SetEmboss(long Value) = 0;
    virtual HRESULT WINAPI GetForeColor(long *pValue) = 0;
    virtual HRESULT WINAPI SetForeColor(long Value) = 0;
    virtual HRESULT WINAPI GetHidden(long *pValue) = 0;
    virtual HRESULT WINAPI SetHidden(long Value) = 0;
    virtual HRESULT WINAPI GetEngrave(long *pValue) = 0;
    virtual HRESULT WINAPI SetEngrave(long Value) = 0;
    virtual HRESULT WINAPI GetItalic(long *pValue) = 0;
    virtual HRESULT WINAPI SetItalic(long Value) = 0;
    virtual HRESULT WINAPI GetKerning(float *pValue) = 0;
    virtual HRESULT WINAPI SetKerning(float Value) = 0;
    virtual HRESULT WINAPI GetLanguageID(long *pValue) = 0;
    virtual HRESULT WINAPI SetLanguageID(long Value) = 0;
    virtual HRESULT WINAPI GetName(BSTR *pbstr) = 0;
    virtual HRESULT WINAPI SetName(BSTR bstr) = 0;
    virtual HRESULT WINAPI GetOutline(long *pValue) = 0;
    virtual HRESULT WINAPI SetOutline(long Value) = 0;
    virtual HRESULT WINAPI GetPosition(float *pValue) = 0;
    virtual HRESULT WINAPI SetPosition(float Value) = 0;
    virtual HRESULT WINAPI GetProtected(long *pValue) = 0;
    virtual HRESULT WINAPI SetProtected(long Value) = 0;
    virtual HRESULT WINAPI GetShadow(long *pValue) = 0;
    virtual HRESULT WINAPI SetShadow(long Value) = 0;
    virtual HRESULT WINAPI GetSize(float *pValue) = 0;
    virtual HRESULT WINAPI SetSize(float Value) = 0;
    virtual HRESULT WINAPI GetSmallCaps(long *pValue) = 0;
    virtual HRESULT WINAPI SetSmallCaps(long Value) = 0;
    virtual HRESULT WINAPI GetSpacing(float *pValue) = 0;
    virtual HRESULT WINAPI SetSpacing(float Value) = 0;
    virtual HRESULT WINAPI GetStrikeThrough(long *pValue) = 0;
    virtual HRESULT WINAPI SetStrikeThrough(long Value) = 0;
    virtual HRESULT WINAPI GetSubscript(long *pValue) = 0;
    virtual HRESULT WINAPI SetSubscript(long Value) = 0;
    virtual HRESULT WINAPI GetSuperscript(long *pValue) = 0;
    virtual HRESULT WINAPI SetSuperscript(long Value) = 0;
    virtual HRESULT WINAPI GetUnderline(long *pValue) = 0;
    virtual HRESULT WINAPI SetUnderline(long Value) = 0;
    virtual HRESULT WINAPI GetWeight(long *pValue) = 0;
    virtual HRESULT WINAPI SetWeight(long Value) = 0;
  };
#else
  typedef struct ITextFontVtbl {
    BEGIN_INTERFACE
      HRESULT (WINAPI *QueryInterface)(ITextFont *This,REFIID riid,void **ppvObject);
      ULONG (WINAPI *AddRef)(ITextFont *This);
      ULONG (WINAPI *Release)(ITextFont *This);
      HRESULT (WINAPI *GetTypeInfoCount)(ITextFont *This,UINT *pctinfo);
      HRESULT (WINAPI *GetTypeInfo)(ITextFont *This,UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo);
      HRESULT (WINAPI *GetIDsOfNames)(ITextFont *This,REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId);
      HRESULT (WINAPI *Invoke)(ITextFont *This,DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr);
      HRESULT (WINAPI *GetDuplicate)(ITextFont *This,ITextFont **ppFont);
      HRESULT (WINAPI *SetDuplicate)(ITextFont *This,ITextFont *pFont);
      HRESULT (WINAPI *CanChange)(ITextFont *This,long *pB);
      HRESULT (WINAPI *IsEqual)(ITextFont *This,ITextFont *pFont,long *pB);
      HRESULT (WINAPI *Reset)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetStyle)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetStyle)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetAllCaps)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetAllCaps)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetAnimation)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetAnimation)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetBackColor)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetBackColor)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetBold)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetBold)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetEmboss)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetEmboss)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetForeColor)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetForeColor)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetHidden)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetHidden)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetEngrave)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetEngrave)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetItalic)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetItalic)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetKerning)(ITextFont *This,float *pValue);
      HRESULT (WINAPI *SetKerning)(ITextFont *This,float Value);
      HRESULT (WINAPI *GetLanguageID)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetLanguageID)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetName)(ITextFont *This,BSTR *pbstr);
      HRESULT (WINAPI *SetName)(ITextFont *This,BSTR bstr);
      HRESULT (WINAPI *GetOutline)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetOutline)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetPosition)(ITextFont *This,float *pValue);
      HRESULT (WINAPI *SetPosition)(ITextFont *This,float Value);
      HRESULT (WINAPI *GetProtected)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetProtected)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetShadow)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetShadow)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetSize)(ITextFont *This,float *pValue);
      HRESULT (WINAPI *SetSize)(ITextFont *This,float Value);
      HRESULT (WINAPI *GetSmallCaps)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetSmallCaps)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetSpacing)(ITextFont *This,float *pValue);
      HRESULT (WINAPI *SetSpacing)(ITextFont *This,float Value);
      HRESULT (WINAPI *GetStrikeThrough)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetStrikeThrough)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetSubscript)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetSubscript)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetSuperscript)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetSuperscript)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetUnderline)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetUnderline)(ITextFont *This,long Value);
      HRESULT (WINAPI *GetWeight)(ITextFont *This,long *pValue);
      HRESULT (WINAPI *SetWeight)(ITextFont *This,long Value);
    END_INTERFACE
  } ITextFontVtbl;
  struct ITextFont {
    CONST_VTBL struct ITextFontVtbl *lpVtbl;
  };
#ifdef COBJMACROS
#define ITextFont_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define ITextFont_AddRef(This) (This)->lpVtbl->AddRef(This)
#define ITextFont_Release(This) (This)->lpVtbl->Release(This)
#define ITextFont_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define ITextFont_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ITextFont_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ITextFont_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define ITextFont_GetDuplicate(This,ppFont) (This)->lpVtbl->GetDuplicate(This,ppFont)
#define ITextFont_SetDuplicate(This,pFont) (This)->lpVtbl->SetDuplicate(This,pFont)
#define ITextFont_CanChange(This,pB) (This)->lpVtbl->CanChange(This,pB)
#define ITextFont_IsEqual(This,pFont,pB) (This)->lpVtbl->IsEqual(This,pFont,pB)
#define ITextFont_Reset(This,Value) (This)->lpVtbl->Reset(This,Value)
#define ITextFont_GetStyle(This,pValue) (This)->lpVtbl->GetStyle(This,pValue)
#define ITextFont_SetStyle(This,Value) (This)->lpVtbl->SetStyle(This,Value)
#define ITextFont_GetAllCaps(This,pValue) (This)->lpVtbl->GetAllCaps(This,pValue)
#define ITextFont_SetAllCaps(This,Value) (This)->lpVtbl->SetAllCaps(This,Value)
#define ITextFont_GetAnimation(This,pValue) (This)->lpVtbl->GetAnimation(This,pValue)
#define ITextFont_SetAnimation(This,Value) (This)->lpVtbl->SetAnimation(This,Value)
#define ITextFont_GetBackColor(This,pValue) (This)->lpVtbl->GetBackColor(This,pValue)
#define ITextFont_SetBackColor(This,Value) (This)->lpVtbl->SetBackColor(This,Value)
#define ITextFont_GetBold(This,pValue) (This)->lpVtbl->GetBold(This,pValue)
#define ITextFont_SetBold(This,Value) (This)->lpVtbl->SetBold(This,Value)
#define ITextFont_GetEmboss(This,pValue) (This)->lpVtbl->GetEmboss(This,pValue)
#define ITextFont_SetEmboss(This,Value) (This)->lpVtbl->SetEmboss(This,Value)
#define ITextFont_GetForeColor(This,pValue) (This)->lpVtbl->GetForeColor(This,pValue)
#define ITextFont_SetForeColor(This,Value) (This)->lpVtbl->SetForeColor(This,Value)
#define ITextFont_GetHidden(This,pValue) (This)->lpVtbl->GetHidden(This,pValue)
#define ITextFont_SetHidden(This,Value) (This)->lpVtbl->SetHidden(This,Value)
#define ITextFont_GetEngrave(This,pValue) (This)->lpVtbl->GetEngrave(This,pValue)
#define ITextFont_SetEngrave(This,Value) (This)->lpVtbl->SetEngrave(This,Value)
#define ITextFont_GetItalic(This,pValue) (This)->lpVtbl->GetItalic(This,pValue)
#define ITextFont_SetItalic(This,Value) (This)->lpVtbl->SetItalic(This,Value)
#define ITextFont_GetKerning(This,pValue) (This)->lpVtbl->GetKerning(This,pValue)
#define ITextFont_SetKerning(This,Value) (This)->lpVtbl->SetKerning(This,Value)
#define ITextFont_GetLanguageID(This,pValue) (This)->lpVtbl->GetLanguageID(This,pValue)
#define ITextFont_SetLanguageID(This,Value) (This)->lpVtbl->SetLanguageID(This,Value)
#define ITextFont_GetName(This,pbstr) (This)->lpVtbl->GetName(This,pbstr)
#define ITextFont_SetName(This,bstr) (This)->lpVtbl->SetName(This,bstr)
#define ITextFont_GetOutline(This,pValue) (This)->lpVtbl->GetOutline(This,pValue)
#define ITextFont_SetOutline(This,Value) (This)->lpVtbl->SetOutline(This,Value)
#define ITextFont_GetPosition(This,pValue) (This)->lpVtbl->GetPosition(This,pValue)
#define ITextFont_SetPosition(This,Value) (This)->lpVtbl->SetPosition(This,Value)
#define ITextFont_GetProtected(This,pValue) (This)->lpVtbl->GetProtected(This,pValue)
#define ITextFont_SetProtected(This,Value) (This)->lpVtbl->SetProtected(This,Value)
#define ITextFont_GetShadow(This,pValue) (This)->lpVtbl->GetShadow(This,pValue)
#define ITextFont_SetShadow(This,Value) (This)->lpVtbl->SetShadow(This,Value)
#define ITextFont_GetSize(This,pValue) (This)->lpVtbl->GetSize(This,pValue)
#define ITextFont_SetSize(This,Value) (This)->lpVtbl->SetSize(This,Value)
#define ITextFont_GetSmallCaps(This,pValue) (This)->lpVtbl->GetSmallCaps(This,pValue)
#define ITextFont_SetSmallCaps(This,Value) (This)->lpVtbl->SetSmallCaps(This,Value)
#define ITextFont_GetSpacing(This,pValue) (This)->lpVtbl->GetSpacing(This,pValue)
#define ITextFont_SetSpacing(This,Value) (This)->lpVtbl->SetSpacing(This,Value)
#define ITextFont_GetStrikeThrough(This,pValue) (This)->lpVtbl->GetStrikeThrough(This,pValue)
#define ITextFont_SetStrikeThrough(This,Value) (This)->lpVtbl->SetStrikeThrough(This,Value)
#define ITextFont_GetSubscript(This,pValue) (This)->lpVtbl->GetSubscript(This,pValue)
#define ITextFont_SetSubscript(This,Value) (This)->lpVtbl->SetSubscript(This,Value)
#define ITextFont_GetSuperscript(This,pValue) (This)->lpVtbl->GetSuperscript(This,pValue)
#define ITextFont_SetSuperscript(This,Value) (This)->lpVtbl->SetSuperscript(This,Value)
#define ITextFont_GetUnderline(This,pValue) (This)->lpVtbl->GetUnderline(This,pValue)
#define ITextFont_SetUnderline(This,Value) (This)->lpVtbl->SetUnderline(This,Value)
#define ITextFont_GetWeight(This,pValue) (This)->lpVtbl->GetWeight(This,pValue)
#define ITextFont_SetWeight(This,Value) (This)->lpVtbl->SetWeight(This,Value)
#endif
#endif
  HRESULT WINAPI ITextFont_GetDuplicate_Proxy(ITextFont *This,ITextFont **ppFont);
  void __RPC_STUB ITextFont_GetDuplicate_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetDuplicate_Proxy(ITextFont *This,ITextFont *pFont);
  void __RPC_STUB ITextFont_SetDuplicate_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_CanChange_Proxy(ITextFont *This,long *pB);
  void __RPC_STUB ITextFont_CanChange_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_IsEqual_Proxy(ITextFont *This,ITextFont *pFont,long *pB);
  void __RPC_STUB ITextFont_IsEqual_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_Reset_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_Reset_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetStyle_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetStyle_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetStyle_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetStyle_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetAllCaps_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetAllCaps_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetAllCaps_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetAllCaps_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetAnimation_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetAnimation_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetAnimation_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetAnimation_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetBackColor_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetBackColor_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetBackColor_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetBackColor_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetBold_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetBold_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetBold_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetBold_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetEmboss_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetEmboss_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetEmboss_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetEmboss_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetForeColor_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetForeColor_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetForeColor_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetForeColor_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetHidden_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetHidden_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetHidden_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetHidden_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetEngrave_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetEngrave_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetEngrave_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetEngrave_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetItalic_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetItalic_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetItalic_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetItalic_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetKerning_Proxy(ITextFont *This,float *pValue);
  void __RPC_STUB ITextFont_GetKerning_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetKerning_Proxy(ITextFont *This,float Value);
  void __RPC_STUB ITextFont_SetKerning_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetLanguageID_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetLanguageID_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetLanguageID_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetLanguageID_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetName_Proxy(ITextFont *This,BSTR *pbstr);
  void __RPC_STUB ITextFont_GetName_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetName_Proxy(ITextFont *This,BSTR bstr);
  void __RPC_STUB ITextFont_SetName_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetOutline_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetOutline_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetOutline_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetOutline_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetPosition_Proxy(ITextFont *This,float *pValue);
  void __RPC_STUB ITextFont_GetPosition_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetPosition_Proxy(ITextFont *This,float Value);
  void __RPC_STUB ITextFont_SetPosition_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetProtected_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetProtected_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetProtected_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetProtected_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetShadow_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetShadow_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetShadow_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetShadow_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetSize_Proxy(ITextFont *This,float *pValue);
  void __RPC_STUB ITextFont_GetSize_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetSize_Proxy(ITextFont *This,float Value);
  void __RPC_STUB ITextFont_SetSize_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetSmallCaps_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetSmallCaps_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetSmallCaps_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetSmallCaps_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetSpacing_Proxy(ITextFont *This,float *pValue);
  void __RPC_STUB ITextFont_GetSpacing_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetSpacing_Proxy(ITextFont *This,float Value);
  void __RPC_STUB ITextFont_SetSpacing_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetStrikeThrough_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetStrikeThrough_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetStrikeThrough_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetStrikeThrough_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetSubscript_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetSubscript_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetSubscript_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetSubscript_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetSuperscript_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetSuperscript_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetSuperscript_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetSuperscript_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetUnderline_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetUnderline_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetUnderline_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetUnderline_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_GetWeight_Proxy(ITextFont *This,long *pValue);
  void __RPC_STUB ITextFont_GetWeight_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextFont_SetWeight_Proxy(ITextFont *This,long Value);
  void __RPC_STUB ITextFont_SetWeight_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
#endif

#ifndef __ITextPara_INTERFACE_DEFINED__
#define __ITextPara_INTERFACE_DEFINED__
  EXTERN_C const IID IID_ITextPara;
#if defined(__cplusplus) && !defined(CINTERFACE)
  struct ITextPara : public IDispatch {
  public:
    virtual HRESULT WINAPI GetDuplicate(ITextPara **ppPara) = 0;
    virtual HRESULT WINAPI SetDuplicate(ITextPara *pPara) = 0;
    virtual HRESULT WINAPI CanChange(long *pB) = 0;
    virtual HRESULT WINAPI IsEqual(ITextPara *pPara,long *pB) = 0;
    virtual HRESULT WINAPI Reset(long Value) = 0;
    virtual HRESULT WINAPI GetStyle(long *pValue) = 0;
    virtual HRESULT WINAPI SetStyle(long Value) = 0;
    virtual HRESULT WINAPI GetAlignment(long *pValue) = 0;
    virtual HRESULT WINAPI SetAlignment(long Value) = 0;
    virtual HRESULT WINAPI GetHyphenation(long *pValue) = 0;
    virtual HRESULT WINAPI SetHyphenation(long Value) = 0;
    virtual HRESULT WINAPI GetFirstLineIndent(float *pValue) = 0;
    virtual HRESULT WINAPI GetKeepTogether(long *pValue) = 0;
    virtual HRESULT WINAPI SetKeepTogether(long Value) = 0;
    virtual HRESULT WINAPI GetKeepWithNext(long *pValue) = 0;
    virtual HRESULT WINAPI SetKeepWithNext(long Value) = 0;
    virtual HRESULT WINAPI GetLeftIndent(float *pValue) = 0;
    virtual HRESULT WINAPI GetLineSpacing(float *pValue) = 0;
    virtual HRESULT WINAPI GetLineSpacingRule(long *pValue) = 0;
    virtual HRESULT WINAPI GetListAlignment(long *pValue) = 0;
    virtual HRESULT WINAPI SetListAlignment(long Value) = 0;
    virtual HRESULT WINAPI GetListLevelIndex(long *pValue) = 0;
    virtual HRESULT WINAPI SetListLevelIndex(long Value) = 0;
    virtual HRESULT WINAPI GetListStart(long *pValue) = 0;
    virtual HRESULT WINAPI SetListStart(long Value) = 0;
    virtual HRESULT WINAPI GetListTab(float *pValue) = 0;
    virtual HRESULT WINAPI SetListTab(float Value) = 0;
    virtual HRESULT WINAPI GetListType(long *pValue) = 0;
    virtual HRESULT WINAPI SetListType(long Value) = 0;
    virtual HRESULT WINAPI GetNoLineNumber(long *pValue) = 0;
    virtual HRESULT WINAPI SetNoLineNumber(long Value) = 0;
    virtual HRESULT WINAPI GetPageBreakBefore(long *pValue) = 0;
    virtual HRESULT WINAPI SetPageBreakBefore(long Value) = 0;
    virtual HRESULT WINAPI GetRightIndent(float *pValue) = 0;
    virtual HRESULT WINAPI SetRightIndent(float Value) = 0;
    virtual HRESULT WINAPI SetIndents(float StartIndent,float LeftIndent,float RightIndent) = 0;
    virtual HRESULT WINAPI SetLineSpacing(long LineSpacingRule,float LineSpacing) = 0;
    virtual HRESULT WINAPI GetSpaceAfter(float *pValue) = 0;
    virtual HRESULT WINAPI SetSpaceAfter(float Value) = 0;
    virtual HRESULT WINAPI GetSpaceBefore(float *pValue) = 0;
    virtual HRESULT WINAPI SetSpaceBefore(float Value) = 0;
    virtual HRESULT WINAPI GetWidowControl(long *pValue) = 0;
    virtual HRESULT WINAPI SetWidowControl(long Value) = 0;
    virtual HRESULT WINAPI GetTabCount(long *pCount) = 0;
    virtual HRESULT WINAPI AddTab(float tbPos,long tbAlign,long tbLeader) = 0;
    virtual HRESULT WINAPI ClearAllTabs(void) = 0;
    virtual HRESULT WINAPI DeleteTab(float tbPos) = 0;
    virtual HRESULT WINAPI GetTab(long iTab,float *ptbPos,long *ptbAlign,long *ptbLeader) = 0;
  };
#else
  typedef struct ITextParaVtbl {
    BEGIN_INTERFACE
      HRESULT (WINAPI *QueryInterface)(ITextPara *This,REFIID riid,void **ppvObject);
      ULONG (WINAPI *AddRef)(ITextPara *This);
      ULONG (WINAPI *Release)(ITextPara *This);
      HRESULT (WINAPI *GetTypeInfoCount)(ITextPara *This,UINT *pctinfo);
      HRESULT (WINAPI *GetTypeInfo)(ITextPara *This,UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo);
      HRESULT (WINAPI *GetIDsOfNames)(ITextPara *This,REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId);
      HRESULT (WINAPI *Invoke)(ITextPara *This,DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr);
      HRESULT (WINAPI *GetDuplicate)(ITextPara *This,ITextPara **ppPara);
      HRESULT (WINAPI *SetDuplicate)(ITextPara *This,ITextPara *pPara);
      HRESULT (WINAPI *CanChange)(ITextPara *This,long *pB);
      HRESULT (WINAPI *IsEqual)(ITextPara *This,ITextPara *pPara,long *pB);
      HRESULT (WINAPI *Reset)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetStyle)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetStyle)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetAlignment)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetAlignment)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetHyphenation)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetHyphenation)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetFirstLineIndent)(ITextPara *This,float *pValue);
      HRESULT (WINAPI *GetKeepTogether)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetKeepTogether)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetKeepWithNext)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetKeepWithNext)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetLeftIndent)(ITextPara *This,float *pValue);
      HRESULT (WINAPI *GetLineSpacing)(ITextPara *This,float *pValue);
      HRESULT (WINAPI *GetLineSpacingRule)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *GetListAlignment)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetListAlignment)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetListLevelIndex)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetListLevelIndex)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetListStart)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetListStart)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetListTab)(ITextPara *This,float *pValue);
      HRESULT (WINAPI *SetListTab)(ITextPara *This,float Value);
      HRESULT (WINAPI *GetListType)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetListType)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetNoLineNumber)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetNoLineNumber)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetPageBreakBefore)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetPageBreakBefore)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetRightIndent)(ITextPara *This,float *pValue);
      HRESULT (WINAPI *SetRightIndent)(ITextPara *This,float Value);
      HRESULT (WINAPI *SetIndents)(ITextPara *This,float StartIndent,float LeftIndent,float RightIndent);
      HRESULT (WINAPI *SetLineSpacing)(ITextPara *This,long LineSpacingRule,float LineSpacing);
      HRESULT (WINAPI *GetSpaceAfter)(ITextPara *This,float *pValue);
      HRESULT (WINAPI *SetSpaceAfter)(ITextPara *This,float Value);
      HRESULT (WINAPI *GetSpaceBefore)(ITextPara *This,float *pValue);
      HRESULT (WINAPI *SetSpaceBefore)(ITextPara *This,float Value);
      HRESULT (WINAPI *GetWidowControl)(ITextPara *This,long *pValue);
      HRESULT (WINAPI *SetWidowControl)(ITextPara *This,long Value);
      HRESULT (WINAPI *GetTabCount)(ITextPara *This,long *pCount);
      HRESULT (WINAPI *AddTab)(ITextPara *This,float tbPos,long tbAlign,long tbLeader);
      HRESULT (WINAPI *ClearAllTabs)(ITextPara *This);
      HRESULT (WINAPI *DeleteTab)(ITextPara *This,float tbPos);
      HRESULT (WINAPI *GetTab)(ITextPara *This,long iTab,float *ptbPos,long *ptbAlign,long *ptbLeader);
    END_INTERFACE
  } ITextParaVtbl;
  struct ITextPara {
    CONST_VTBL struct ITextParaVtbl *lpVtbl;
  };
#ifdef COBJMACROS
#define ITextPara_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define ITextPara_AddRef(This) (This)->lpVtbl->AddRef(This)
#define ITextPara_Release(This) (This)->lpVtbl->Release(This)
#define ITextPara_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define ITextPara_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ITextPara_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ITextPara_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define ITextPara_GetDuplicate(This,ppPara) (This)->lpVtbl->GetDuplicate(This,ppPara)
#define ITextPara_SetDuplicate(This,pPara) (This)->lpVtbl->SetDuplicate(This,pPara)
#define ITextPara_CanChange(This,pB) (This)->lpVtbl->CanChange(This,pB)
#define ITextPara_IsEqual(This,pPara,pB) (This)->lpVtbl->IsEqual(This,pPara,pB)
#define ITextPara_Reset(This,Value) (This)->lpVtbl->Reset(This,Value)
#define ITextPara_GetStyle(This,pValue) (This)->lpVtbl->GetStyle(This,pValue)
#define ITextPara_SetStyle(This,Value) (This)->lpVtbl->SetStyle(This,Value)
#define ITextPara_GetAlignment(This,pValue) (This)->lpVtbl->GetAlignment(This,pValue)
#define ITextPara_SetAlignment(This,Value) (This)->lpVtbl->SetAlignment(This,Value)
#define ITextPara_GetHyphenation(This,pValue) (This)->lpVtbl->GetHyphenation(This,pValue)
#define ITextPara_SetHyphenation(This,Value) (This)->lpVtbl->SetHyphenation(This,Value)
#define ITextPara_GetFirstLineIndent(This,pValue) (This)->lpVtbl->GetFirstLineIndent(This,pValue)
#define ITextPara_GetKeepTogether(This,pValue) (This)->lpVtbl->GetKeepTogether(This,pValue)
#define ITextPara_SetKeepTogether(This,Value) (This)->lpVtbl->SetKeepTogether(This,Value)
#define ITextPara_GetKeepWithNext(This,pValue) (This)->lpVtbl->GetKeepWithNext(This,pValue)
#define ITextPara_SetKeepWithNext(This,Value) (This)->lpVtbl->SetKeepWithNext(This,Value)
#define ITextPara_GetLeftIndent(This,pValue) (This)->lpVtbl->GetLeftIndent(This,pValue)
#define ITextPara_GetLineSpacing(This,pValue) (This)->lpVtbl->GetLineSpacing(This,pValue)
#define ITextPara_GetLineSpacingRule(This,pValue) (This)->lpVtbl->GetLineSpacingRule(This,pValue)
#define ITextPara_GetListAlignment(This,pValue) (This)->lpVtbl->GetListAlignment(This,pValue)
#define ITextPara_SetListAlignment(This,Value) (This)->lpVtbl->SetListAlignment(This,Value)
#define ITextPara_GetListLevelIndex(This,pValue) (This)->lpVtbl->GetListLevelIndex(This,pValue)
#define ITextPara_SetListLevelIndex(This,Value) (This)->lpVtbl->SetListLevelIndex(This,Value)
#define ITextPara_GetListStart(This,pValue) (This)->lpVtbl->GetListStart(This,pValue)
#define ITextPara_SetListStart(This,Value) (This)->lpVtbl->SetListStart(This,Value)
#define ITextPara_GetListTab(This,pValue) (This)->lpVtbl->GetListTab(This,pValue)
#define ITextPara_SetListTab(This,Value) (This)->lpVtbl->SetListTab(This,Value)
#define ITextPara_GetListType(This,pValue) (This)->lpVtbl->GetListType(This,pValue)
#define ITextPara_SetListType(This,Value) (This)->lpVtbl->SetListType(This,Value)
#define ITextPara_GetNoLineNumber(This,pValue) (This)->lpVtbl->GetNoLineNumber(This,pValue)
#define ITextPara_SetNoLineNumber(This,Value) (This)->lpVtbl->SetNoLineNumber(This,Value)
#define ITextPara_GetPageBreakBefore(This,pValue) (This)->lpVtbl->GetPageBreakBefore(This,pValue)
#define ITextPara_SetPageBreakBefore(This,Value) (This)->lpVtbl->SetPageBreakBefore(This,Value)
#define ITextPara_GetRightIndent(This,pValue) (This)->lpVtbl->GetRightIndent(This,pValue)
#define ITextPara_SetRightIndent(This,Value) (This)->lpVtbl->SetRightIndent(This,Value)
#define ITextPara_SetIndents(This,StartIndent,LeftIndent,RightIndent) (This)->lpVtbl->SetIndents(This,StartIndent,LeftIndent,RightIndent)
#define ITextPara_SetLineSpacing(This,LineSpacingRule,LineSpacing) (This)->lpVtbl->SetLineSpacing(This,LineSpacingRule,LineSpacing)
#define ITextPara_GetSpaceAfter(This,pValue) (This)->lpVtbl->GetSpaceAfter(This,pValue)
#define ITextPara_SetSpaceAfter(This,Value) (This)->lpVtbl->SetSpaceAfter(This,Value)
#define ITextPara_GetSpaceBefore(This,pValue) (This)->lpVtbl->GetSpaceBefore(This,pValue)
#define ITextPara_SetSpaceBefore(This,Value) (This)->lpVtbl->SetSpaceBefore(This,Value)
#define ITextPara_GetWidowControl(This,pValue) (This)->lpVtbl->GetWidowControl(This,pValue)
#define ITextPara_SetWidowControl(This,Value) (This)->lpVtbl->SetWidowControl(This,Value)
#define ITextPara_GetTabCount(This,pCount) (This)->lpVtbl->GetTabCount(This,pCount)
#define ITextPara_AddTab(This,tbPos,tbAlign,tbLeader) (This)->lpVtbl->AddTab(This,tbPos,tbAlign,tbLeader)
#define ITextPara_ClearAllTabs(This) (This)->lpVtbl->ClearAllTabs(This)
#define ITextPara_DeleteTab(This,tbPos) (This)->lpVtbl->DeleteTab(This,tbPos)
#define ITextPara_GetTab(This,iTab,ptbPos,ptbAlign,ptbLeader) (This)->lpVtbl->GetTab(This,iTab,ptbPos,ptbAlign,ptbLeader)
#endif
#endif
  HRESULT WINAPI ITextPara_GetDuplicate_Proxy(ITextPara *This,ITextPara **ppPara);
  void __RPC_STUB ITextPara_GetDuplicate_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetDuplicate_Proxy(ITextPara *This,ITextPara *pPara);
  void __RPC_STUB ITextPara_SetDuplicate_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_CanChange_Proxy(ITextPara *This,long *pB);
  void __RPC_STUB ITextPara_CanChange_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_IsEqual_Proxy(ITextPara *This,ITextPara *pPara,long *pB);
  void __RPC_STUB ITextPara_IsEqual_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_Reset_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_Reset_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetStyle_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetStyle_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetStyle_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetStyle_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetAlignment_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetAlignment_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetAlignment_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetAlignment_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetHyphenation_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetHyphenation_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetHyphenation_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetHyphenation_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetFirstLineIndent_Proxy(ITextPara *This,float *pValue);
  void __RPC_STUB ITextPara_GetFirstLineIndent_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetKeepTogether_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetKeepTogether_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetKeepTogether_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetKeepTogether_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetKeepWithNext_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetKeepWithNext_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetKeepWithNext_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetKeepWithNext_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetLeftIndent_Proxy(ITextPara *This,float *pValue);
  void __RPC_STUB ITextPara_GetLeftIndent_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetLineSpacing_Proxy(ITextPara *This,float *pValue);
  void __RPC_STUB ITextPara_GetLineSpacing_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetLineSpacingRule_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetLineSpacingRule_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetListAlignment_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetListAlignment_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetListAlignment_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetListAlignment_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetListLevelIndex_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetListLevelIndex_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetListLevelIndex_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetListLevelIndex_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetListStart_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetListStart_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetListStart_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetListStart_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetListTab_Proxy(ITextPara *This,float *pValue);
  void __RPC_STUB ITextPara_GetListTab_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetListTab_Proxy(ITextPara *This,float Value);
  void __RPC_STUB ITextPara_SetListTab_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetListType_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetListType_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetListType_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetListType_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetNoLineNumber_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetNoLineNumber_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetNoLineNumber_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetNoLineNumber_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetPageBreakBefore_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetPageBreakBefore_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetPageBreakBefore_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetPageBreakBefore_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetRightIndent_Proxy(ITextPara *This,float *pValue);
  void __RPC_STUB ITextPara_GetRightIndent_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetRightIndent_Proxy(ITextPara *This,float Value);
  void __RPC_STUB ITextPara_SetRightIndent_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetIndents_Proxy(ITextPara *This,float StartIndent,float LeftIndent,float RightIndent);
  void __RPC_STUB ITextPara_SetIndents_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetLineSpacing_Proxy(ITextPara *This,long LineSpacingRule,float LineSpacing);
  void __RPC_STUB ITextPara_SetLineSpacing_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetSpaceAfter_Proxy(ITextPara *This,float *pValue);
  void __RPC_STUB ITextPara_GetSpaceAfter_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetSpaceAfter_Proxy(ITextPara *This,float Value);
  void __RPC_STUB ITextPara_SetSpaceAfter_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetSpaceBefore_Proxy(ITextPara *This,float *pValue);
  void __RPC_STUB ITextPara_GetSpaceBefore_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetSpaceBefore_Proxy(ITextPara *This,float Value);
  void __RPC_STUB ITextPara_SetSpaceBefore_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetWidowControl_Proxy(ITextPara *This,long *pValue);
  void __RPC_STUB ITextPara_GetWidowControl_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_SetWidowControl_Proxy(ITextPara *This,long Value);
  void __RPC_STUB ITextPara_SetWidowControl_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetTabCount_Proxy(ITextPara *This,long *pCount);
  void __RPC_STUB ITextPara_GetTabCount_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_AddTab_Proxy(ITextPara *This,float tbPos,long tbAlign,long tbLeader);
  void __RPC_STUB ITextPara_AddTab_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_ClearAllTabs_Proxy(ITextPara *This);
  void __RPC_STUB ITextPara_ClearAllTabs_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_DeleteTab_Proxy(ITextPara *This,float tbPos);
  void __RPC_STUB ITextPara_DeleteTab_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextPara_GetTab_Proxy(ITextPara *This,long iTab,float *ptbPos,long *ptbAlign,long *ptbLeader);
  void __RPC_STUB ITextPara_GetTab_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
#endif

#ifndef __ITextStoryRanges_INTERFACE_DEFINED__
#define __ITextStoryRanges_INTERFACE_DEFINED__
  EXTERN_C const IID IID_ITextStoryRanges;
#if defined(__cplusplus) && !defined(CINTERFACE)
  struct ITextStoryRanges : public IDispatch {
  public:
    virtual HRESULT WINAPI _NewEnum(IUnknown **ppunkEnum) = 0;
    virtual HRESULT WINAPI Item(long Index,ITextRange **ppRange) = 0;
    virtual HRESULT WINAPI GetCount(long *pCount) = 0;
  };
#else
  typedef struct ITextStoryRangesVtbl {
    BEGIN_INTERFACE
      HRESULT (WINAPI *QueryInterface)(ITextStoryRanges *This,REFIID riid,void **ppvObject);
      ULONG (WINAPI *AddRef)(ITextStoryRanges *This);
      ULONG (WINAPI *Release)(ITextStoryRanges *This);
      HRESULT (WINAPI *GetTypeInfoCount)(ITextStoryRanges *This,UINT *pctinfo);
      HRESULT (WINAPI *GetTypeInfo)(ITextStoryRanges *This,UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo);
      HRESULT (WINAPI *GetIDsOfNames)(ITextStoryRanges *This,REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId);
      HRESULT (WINAPI *Invoke)(ITextStoryRanges *This,DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr);
      HRESULT (WINAPI *_NewEnum)(ITextStoryRanges *This,IUnknown **ppunkEnum);
      HRESULT (WINAPI *Item)(ITextStoryRanges *This,long Index,ITextRange **ppRange);
      HRESULT (WINAPI *GetCount)(ITextStoryRanges *This,long *pCount);
    END_INTERFACE
  } ITextStoryRangesVtbl;
  struct ITextStoryRanges {
    CONST_VTBL struct ITextStoryRangesVtbl *lpVtbl;
  };
#ifdef COBJMACROS
#define ITextStoryRanges_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define ITextStoryRanges_AddRef(This) (This)->lpVtbl->AddRef(This)
#define ITextStoryRanges_Release(This) (This)->lpVtbl->Release(This)
#define ITextStoryRanges_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define ITextStoryRanges_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ITextStoryRanges_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ITextStoryRanges_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define ITextStoryRanges__NewEnum(This,ppunkEnum) (This)->lpVtbl->_NewEnum(This,ppunkEnum)
#define ITextStoryRanges_Item(This,Index,ppRange) (This)->lpVtbl->Item(This,Index,ppRange)
#define ITextStoryRanges_GetCount(This,pCount) (This)->lpVtbl->GetCount(This,pCount)
#endif
#endif
  HRESULT WINAPI ITextStoryRanges__NewEnum_Proxy(ITextStoryRanges *This,IUnknown **ppunkEnum);
  void __RPC_STUB ITextStoryRanges__NewEnum_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextStoryRanges_Item_Proxy(ITextStoryRanges *This,long Index,ITextRange **ppRange);
  void __RPC_STUB ITextStoryRanges_Item_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextStoryRanges_GetCount_Proxy(ITextStoryRanges *This,long *pCount);
  void __RPC_STUB ITextStoryRanges_GetCount_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
#endif

#ifndef __ITextDocument2_INTERFACE_DEFINED__
#define __ITextDocument2_INTERFACE_DEFINED__
  EXTERN_C const IID IID_ITextDocument2;
#if defined(__cplusplus) && !defined(CINTERFACE)
  struct ITextDocument2 : public ITextDocument {
  public:
    virtual HRESULT WINAPI AttachMsgFilter(IUnknown *pFilter) = 0;
    virtual HRESULT WINAPI SetEffectColor(long Index,COLORREF cr) = 0;
    virtual HRESULT WINAPI GetEffectColor(long Index,COLORREF *pcr) = 0;
    virtual HRESULT WINAPI GetCaretType(long *pCaretType) = 0;
    virtual HRESULT WINAPI SetCaretType(long CaretType) = 0;
    virtual HRESULT WINAPI GetImmContext(long *pContext) = 0;
    virtual HRESULT WINAPI ReleaseImmContext(long Context) = 0;
    virtual HRESULT WINAPI GetPreferredFont(long cp,long CodePage,long Option,long curCodepage,long curFontSize,BSTR *pbstr,long *pPitchAndFamily,long *pNewFontSize) = 0;
    virtual HRESULT WINAPI GetNotificationMode(long *pMode) = 0;
    virtual HRESULT WINAPI SetNotificationMode(long Mode) = 0;
    virtual HRESULT WINAPI GetClientRect(long Type,long *pLeft,long *pTop,long *pRight,long *pBottom) = 0;
    virtual HRESULT WINAPI GetSelectionEx(ITextSelection **ppSel) = 0;
    virtual HRESULT WINAPI GetWindow(long *phWnd) = 0;
    virtual HRESULT WINAPI GetFEFlags(long *pFlags) = 0;
    virtual HRESULT WINAPI UpdateWindow(void) = 0;
    virtual HRESULT WINAPI CheckTextLimit(long cch,long *pcch) = 0;
    virtual HRESULT WINAPI IMEInProgress(long Mode) = 0;
    virtual HRESULT WINAPI SysBeep(void) = 0;
    virtual HRESULT WINAPI Update(long Mode) = 0;
    virtual HRESULT WINAPI Notify(long Notify) = 0;
  };
#else
  typedef struct ITextDocument2Vtbl {
    BEGIN_INTERFACE
      HRESULT (WINAPI *QueryInterface)(ITextDocument2 *This,REFIID riid,void **ppvObject);
      ULONG (WINAPI *AddRef)(ITextDocument2 *This);
      ULONG (WINAPI *Release)(ITextDocument2 *This);
      HRESULT (WINAPI *GetTypeInfoCount)(ITextDocument2 *This,UINT *pctinfo);
      HRESULT (WINAPI *GetTypeInfo)(ITextDocument2 *This,UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo);
      HRESULT (WINAPI *GetIDsOfNames)(ITextDocument2 *This,REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId);
      HRESULT (WINAPI *Invoke)(ITextDocument2 *This,DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr);
      HRESULT (WINAPI *GetName)(ITextDocument2 *This,BSTR *pName);
      HRESULT (WINAPI *GetSelection)(ITextDocument2 *This,ITextSelection **ppSel);
      HRESULT (WINAPI *GetStoryCount)(ITextDocument2 *This,long *pCount);
      HRESULT (WINAPI *GetStoryRanges)(ITextDocument2 *This,ITextStoryRanges **ppStories);
      HRESULT (WINAPI *GetSaved)(ITextDocument2 *This,long *pValue);
      HRESULT (WINAPI *SetSaved)(ITextDocument2 *This,long Value);
      HRESULT (WINAPI *GetDefaultTabStop)(ITextDocument2 *This,float *pValue);
      HRESULT (WINAPI *SetDefaultTabStop)(ITextDocument2 *This,float Value);
      HRESULT (WINAPI *New)(ITextDocument2 *This);
      HRESULT (WINAPI *Open)(ITextDocument2 *This,VARIANT *pVar,long Flags,long CodePage);
      HRESULT (WINAPI *Save)(ITextDocument2 *This,VARIANT *pVar,long Flags,long CodePage);
      HRESULT (WINAPI *Freeze)(ITextDocument2 *This,long *pCount);
      HRESULT (WINAPI *Unfreeze)(ITextDocument2 *This,long *pCount);
      HRESULT (WINAPI *BeginEditCollection)(ITextDocument2 *This);
      HRESULT (WINAPI *EndEditCollection)(ITextDocument2 *This);
      HRESULT (WINAPI *Undo)(ITextDocument2 *This,long Count,long *prop);
      HRESULT (WINAPI *Redo)(ITextDocument2 *This,long Count,long *prop);
      HRESULT (WINAPI *Range)(ITextDocument2 *This,long cp1,long cp2,ITextRange **ppRange);
      HRESULT (WINAPI *RangeFromPoint)(ITextDocument2 *This,long x,long y,ITextRange **ppRange);
      HRESULT (WINAPI *AttachMsgFilter)(ITextDocument2 *This,IUnknown *pFilter);
      HRESULT (WINAPI *SetEffectColor)(ITextDocument2 *This,long Index,COLORREF cr);
      HRESULT (WINAPI *GetEffectColor)(ITextDocument2 *This,long Index,COLORREF *pcr);
      HRESULT (WINAPI *GetCaretType)(ITextDocument2 *This,long *pCaretType);
      HRESULT (WINAPI *SetCaretType)(ITextDocument2 *This,long CaretType);
      HRESULT (WINAPI *GetImmContext)(ITextDocument2 *This,long *pContext);
      HRESULT (WINAPI *ReleaseImmContext)(ITextDocument2 *This,long Context);
      HRESULT (WINAPI *GetPreferredFont)(ITextDocument2 *This,long cp,long CodePage,long Option,long curCodepage,long curFontSize,BSTR *pbstr,long *pPitchAndFamily,long *pNewFontSize);
      HRESULT (WINAPI *GetNotificationMode)(ITextDocument2 *This,long *pMode);
      HRESULT (WINAPI *SetNotificationMode)(ITextDocument2 *This,long Mode);
      HRESULT (WINAPI *GetClientRect)(ITextDocument2 *This,long Type,long *pLeft,long *pTop,long *pRight,long *pBottom);
      HRESULT (WINAPI *GetSelectionEx)(ITextDocument2 *This,ITextSelection **ppSel);
      HRESULT (WINAPI *GetWindow)(ITextDocument2 *This,long *phWnd);
      HRESULT (WINAPI *GetFEFlags)(ITextDocument2 *This,long *pFlags);
      HRESULT (WINAPI *UpdateWindow)(ITextDocument2 *This);
      HRESULT (WINAPI *CheckTextLimit)(ITextDocument2 *This,long cch,long *pcch);
      HRESULT (WINAPI *IMEInProgress)(ITextDocument2 *This,long Mode);
      HRESULT (WINAPI *SysBeep)(ITextDocument2 *This);
      HRESULT (WINAPI *Update)(ITextDocument2 *This,long Mode);
      HRESULT (WINAPI *Notify)(ITextDocument2 *This,long Notify);
    END_INTERFACE
  } ITextDocument2Vtbl;
  struct ITextDocument2 {
    CONST_VTBL struct ITextDocument2Vtbl *lpVtbl;
  };
#ifdef COBJMACROS
#define ITextDocument2_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define ITextDocument2_AddRef(This) (This)->lpVtbl->AddRef(This)
#define ITextDocument2_Release(This) (This)->lpVtbl->Release(This)
#define ITextDocument2_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define ITextDocument2_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ITextDocument2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ITextDocument2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define ITextDocument2_GetName(This,pName) (This)->lpVtbl->GetName(This,pName)
#define ITextDocument2_GetSelection(This,ppSel) (This)->lpVtbl->GetSelection(This,ppSel)
#define ITextDocument2_GetStoryCount(This,pCount) (This)->lpVtbl->GetStoryCount(This,pCount)
#define ITextDocument2_GetStoryRanges(This,ppStories) (This)->lpVtbl->GetStoryRanges(This,ppStories)
#define ITextDocument2_GetSaved(This,pValue) (This)->lpVtbl->GetSaved(This,pValue)
#define ITextDocument2_SetSaved(This,Value) (This)->lpVtbl->SetSaved(This,Value)
#define ITextDocument2_GetDefaultTabStop(This,pValue) (This)->lpVtbl->GetDefaultTabStop(This,pValue)
#define ITextDocument2_SetDefaultTabStop(This,Value) (This)->lpVtbl->SetDefaultTabStop(This,Value)
#define ITextDocument2_New(This) (This)->lpVtbl->New(This)
#define ITextDocument2_Open(This,pVar,Flags,CodePage) (This)->lpVtbl->Open(This,pVar,Flags,CodePage)
#define ITextDocument2_Save(This,pVar,Flags,CodePage) (This)->lpVtbl->Save(This,pVar,Flags,CodePage)
#define ITextDocument2_Freeze(This,pCount) (This)->lpVtbl->Freeze(This,pCount)
#define ITextDocument2_Unfreeze(This,pCount) (This)->lpVtbl->Unfreeze(This,pCount)
#define ITextDocument2_BeginEditCollection(This) (This)->lpVtbl->BeginEditCollection(This)
#define ITextDocument2_EndEditCollection(This) (This)->lpVtbl->EndEditCollection(This)
#define ITextDocument2_Undo(This,Count,prop) (This)->lpVtbl->Undo(This,Count,prop)
#define ITextDocument2_Redo(This,Count,prop) (This)->lpVtbl->Redo(This,Count,prop)
#define ITextDocument2_Range(This,cp1,cp2,ppRange) (This)->lpVtbl->Range(This,cp1,cp2,ppRange)
#define ITextDocument2_RangeFromPoint(This,x,y,ppRange) (This)->lpVtbl->RangeFromPoint(This,x,y,ppRange)
#define ITextDocument2_AttachMsgFilter(This,pFilter) (This)->lpVtbl->AttachMsgFilter(This,pFilter)
#define ITextDocument2_SetEffectColor(This,Index,cr) (This)->lpVtbl->SetEffectColor(This,Index,cr)
#define ITextDocument2_GetEffectColor(This,Index,pcr) (This)->lpVtbl->GetEffectColor(This,Index,pcr)
#define ITextDocument2_GetCaretType(This,pCaretType) (This)->lpVtbl->GetCaretType(This,pCaretType)
#define ITextDocument2_SetCaretType(This,CaretType) (This)->lpVtbl->SetCaretType(This,CaretType)
#define ITextDocument2_GetImmContext(This,pContext) (This)->lpVtbl->GetImmContext(This,pContext)
#define ITextDocument2_ReleaseImmContext(This,Context) (This)->lpVtbl->ReleaseImmContext(This,Context)
#define ITextDocument2_GetPreferredFont(This,cp,CodePage,Option,curCodepage,curFontSize,pbstr,pPitchAndFamily,pNewFontSize) (This)->lpVtbl->GetPreferredFont(This,cp,CodePage,Option,curCodepage,curFontSize,pbstr,pPitchAndFamily,pNewFontSize)
#define ITextDocument2_GetNotificationMode(This,pMode) (This)->lpVtbl->GetNotificationMode(This,pMode)
#define ITextDocument2_SetNotificationMode(This,Mode) (This)->lpVtbl->SetNotificationMode(This,Mode)
#define ITextDocument2_GetClientRect(This,Type,pLeft,pTop,pRight,pBottom) (This)->lpVtbl->GetClientRect(This,Type,pLeft,pTop,pRight,pBottom)
#define ITextDocument2_GetSelectionEx(This,ppSel) (This)->lpVtbl->GetSelectionEx(This,ppSel)
#define ITextDocument2_GetWindow(This,phWnd) (This)->lpVtbl->GetWindow(This,phWnd)
#define ITextDocument2_GetFEFlags(This,pFlags) (This)->lpVtbl->GetFEFlags(This,pFlags)
#define ITextDocument2_UpdateWindow(This) (This)->lpVtbl->UpdateWindow(This)
#define ITextDocument2_CheckTextLimit(This,cch,pcch) (This)->lpVtbl->CheckTextLimit(This,cch,pcch)
#define ITextDocument2_IMEInProgress(This,Mode) (This)->lpVtbl->IMEInProgress(This,Mode)
#define ITextDocument2_SysBeep(This) (This)->lpVtbl->SysBeep(This)
#define ITextDocument2_Update(This,Mode) (This)->lpVtbl->Update(This,Mode)
#define ITextDocument2_Notify(This,Notify) (This)->lpVtbl->Notify(This,Notify)
#endif
#endif
  HRESULT WINAPI ITextDocument2_AttachMsgFilter_Proxy(ITextDocument2 *This,IUnknown *pFilter);
  void __RPC_STUB ITextDocument2_AttachMsgFilter_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_SetEffectColor_Proxy(ITextDocument2 *This,long Index,COLORREF cr);
  void __RPC_STUB ITextDocument2_SetEffectColor_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_GetEffectColor_Proxy(ITextDocument2 *This,long Index,COLORREF *pcr);
  void __RPC_STUB ITextDocument2_GetEffectColor_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_GetCaretType_Proxy(ITextDocument2 *This,long *pCaretType);
  void __RPC_STUB ITextDocument2_GetCaretType_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_SetCaretType_Proxy(ITextDocument2 *This,long CaretType);
  void __RPC_STUB ITextDocument2_SetCaretType_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_GetImmContext_Proxy(ITextDocument2 *This,long *pContext);
  void __RPC_STUB ITextDocument2_GetImmContext_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_ReleaseImmContext_Proxy(ITextDocument2 *This,long Context);
  void __RPC_STUB ITextDocument2_ReleaseImmContext_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_GetPreferredFont_Proxy(ITextDocument2 *This,long cp,long CodePage,long Option,long curCodepage,long curFontSize,BSTR *pbstr,long *pPitchAndFamily,long *pNewFontSize);
  void __RPC_STUB ITextDocument2_GetPreferredFont_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_GetNotificationMode_Proxy(ITextDocument2 *This,long *pMode);
  void __RPC_STUB ITextDocument2_GetNotificationMode_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_SetNotificationMode_Proxy(ITextDocument2 *This,long Mode);
  void __RPC_STUB ITextDocument2_SetNotificationMode_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_GetClientRect_Proxy(ITextDocument2 *This,long Type,long *pLeft,long *pTop,long *pRight,long *pBottom);
  void __RPC_STUB ITextDocument2_GetClientRect_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_GetSelectionEx_Proxy(ITextDocument2 *This,ITextSelection **ppSel);
  void __RPC_STUB ITextDocument2_GetSelectionEx_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_GetWindow_Proxy(ITextDocument2 *This,long *phWnd);
  void __RPC_STUB ITextDocument2_GetWindow_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_GetFEFlags_Proxy(ITextDocument2 *This,long *pFlags);
  void __RPC_STUB ITextDocument2_GetFEFlags_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_UpdateWindow_Proxy(ITextDocument2 *This);
  void __RPC_STUB ITextDocument2_UpdateWindow_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_CheckTextLimit_Proxy(ITextDocument2 *This,long cch,long *pcch);
  void __RPC_STUB ITextDocument2_CheckTextLimit_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_IMEInProgress_Proxy(ITextDocument2 *This,long Mode);
  void __RPC_STUB ITextDocument2_IMEInProgress_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_SysBeep_Proxy(ITextDocument2 *This);
  void __RPC_STUB ITextDocument2_SysBeep_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_Update_Proxy(ITextDocument2 *This,long Mode);
  void __RPC_STUB ITextDocument2_Update_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextDocument2_Notify_Proxy(ITextDocument2 *This,long Notify);
  void __RPC_STUB ITextDocument2_Notify_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
#endif

#ifndef __ITextMsgFilter_INTERFACE_DEFINED__
#define __ITextMsgFilter_INTERFACE_DEFINED__
  EXTERN_C const IID IID_ITextMsgFilter;
#if defined(__cplusplus) && !defined(CINTERFACE)
  struct ITextMsgFilter : public IUnknown {
  public:
    virtual HRESULT WINAPI AttachDocument(HWND hwnd,ITextDocument2 *pTextDoc) = 0;
    virtual HRESULT WINAPI HandleMessage(UINT *pmsg,WPARAM *pwparam,LPARAM *plparam,LRESULT *plres) = 0;
    virtual HRESULT WINAPI AttachMsgFilter(ITextMsgFilter *pMsgFilter) = 0;
  };
#else
  typedef struct ITextMsgFilterVtbl {
    BEGIN_INTERFACE
      HRESULT (WINAPI *QueryInterface)(ITextMsgFilter *This,REFIID riid,void **ppvObject);
      ULONG (WINAPI *AddRef)(ITextMsgFilter *This);
      ULONG (WINAPI *Release)(ITextMsgFilter *This);
      HRESULT (WINAPI *AttachDocument)(ITextMsgFilter *This,HWND hwnd,ITextDocument2 *pTextDoc);
      HRESULT (WINAPI *HandleMessage)(ITextMsgFilter *This,UINT *pmsg,WPARAM *pwparam,LPARAM *plparam,LRESULT *plres);
      HRESULT (WINAPI *AttachMsgFilter)(ITextMsgFilter *This,ITextMsgFilter *pMsgFilter);
    END_INTERFACE
  } ITextMsgFilterVtbl;
  struct ITextMsgFilter {
    CONST_VTBL struct ITextMsgFilterVtbl *lpVtbl;
  };
#ifdef COBJMACROS
#define ITextMsgFilter_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define ITextMsgFilter_AddRef(This) (This)->lpVtbl->AddRef(This)
#define ITextMsgFilter_Release(This) (This)->lpVtbl->Release(This)
#define ITextMsgFilter_AttachDocument(This,hwnd,pTextDoc) (This)->lpVtbl->AttachDocument(This,hwnd,pTextDoc)
#define ITextMsgFilter_HandleMessage(This,pmsg,pwparam,plparam,plres) (This)->lpVtbl->HandleMessage(This,pmsg,pwparam,plparam,plres)
#define ITextMsgFilter_AttachMsgFilter(This,pMsgFilter) (This)->lpVtbl->AttachMsgFilter(This,pMsgFilter)
#endif
#endif
  HRESULT WINAPI ITextMsgFilter_AttachDocument_Proxy(ITextMsgFilter *This,HWND hwnd,ITextDocument2 *pTextDoc);
  void __RPC_STUB ITextMsgFilter_AttachDocument_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextMsgFilter_HandleMessage_Proxy(ITextMsgFilter *This,UINT *pmsg,WPARAM *pwparam,LPARAM *plparam,LRESULT *plres);
  void __RPC_STUB ITextMsgFilter_HandleMessage_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
  HRESULT WINAPI ITextMsgFilter_AttachMsgFilter_Proxy(ITextMsgFilter *This,ITextMsgFilter *pMsgFilter);
  void __RPC_STUB ITextMsgFilter_AttachMsgFilter_Stub(IRpcStubBuffer *This,IRpcChannelBuffer *_pRpcChannelBuffer,PRPC_MESSAGE _pRpcMessage,DWORD *_pdwStubPhase);
#endif
#endif

  unsigned long __RPC_API BSTR_UserSize(unsigned long *,unsigned long,BSTR *);
  unsigned char *__RPC_API BSTR_UserMarshal(unsigned long *,unsigned char *,BSTR *);
  unsigned char *__RPC_API BSTR_UserUnmarshal(unsigned long *,unsigned char *,BSTR *);
  void __RPC_API BSTR_UserFree(unsigned long *,BSTR *);
  unsigned long __RPC_API HWND_UserSize(unsigned long *,unsigned long,HWND *);
  unsigned char *__RPC_API HWND_UserMarshal(unsigned long *,unsigned char *,HWND *);
  unsigned char *__RPC_API HWND_UserUnmarshal(unsigned long *,unsigned char *,HWND *);
  void __RPC_API HWND_UserFree(unsigned long *,HWND *);
  unsigned long __RPC_API VARIANT_UserSize(unsigned long *,unsigned long,VARIANT *);
  unsigned char *__RPC_API VARIANT_UserMarshal(unsigned long *,unsigned char *,VARIANT *);
  unsigned char *__RPC_API VARIANT_UserUnmarshal(unsigned long *,unsigned char *,VARIANT *);
  void __RPC_API VARIANT_UserFree(unsigned long *,VARIANT *);

#ifdef __cplusplus
}
#endif
#endif
