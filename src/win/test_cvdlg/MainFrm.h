// MainFrm.h : taraget class CMainFrame
//

#pragma once

#include "ChildView.h"

class CMainFrame : public CFrameWnd
{
public:
  HWND hwnd_frame;
  BYTE *output_image;
  WORD *input_image;
  int pic_width;
  int pic_height;
  int m_iMode;
  int m_iColMode;
  PCO_Convert *col_lut;
  int m_iConvertType;
  int m_ibpp;
  bool m_Flip,m_Mirror;
  bool m_alloc_output;

  PCO_SensorInfo strsensorinf;

  HANDLE m_hLut;
  HANDLE m_hLutDialog;
//  HANDLE m_hLutDialog16;

public:
  CMainFrame();
protected: 
  DECLARE_DYNAMIC(CMainFrame)

public:

public:

public:
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
  void SetMode(int bayer, bool bflip, bool bmirror);
  void SetConvert(int convert);
  void DoConvert();
  double TT();
public:
  virtual ~CMainFrame();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

protected:
  HANDLE      m_hConvertThread;

public:
  CChildView    m_wndView;

protected:
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSetFocus(CWnd *pOldWnd);
  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnOpenConvertDialog();
  afx_msg void OnClose();
  afx_msg LRESULT OnProcessDialog(WPARAM a, LPARAM b);
  afx_msg void OnFileOpenfile();
  afx_msg void OnBayer0();
  afx_msg void OnUpdateBayer0(CCmdUI *pCmdUI);
  afx_msg void OnBayer1();
  afx_msg void OnUpdateBayer1(CCmdUI *pCmdUI);
  afx_msg void OnBayer2();
  afx_msg void OnUpdateBayer2(CCmdUI *pCmdUI);
  afx_msg void OnBayer3();
  afx_msg void OnUpdateBayer3(CCmdUI *pCmdUI);
  afx_msg void OnFPSTest();
  afx_msg void OnUpdateFPSTest(CCmdUI *pCmdUI);
  afx_msg void OnEditBwconvert();
  afx_msg void OnEditFlipimage();
  afx_msg void OnEditMirrorimage();
  afx_msg void OnEditPseudoconvert();
  afx_msg void OnBitmap32bpp();
  afx_msg void OnBitmap24bpp();
  afx_msg void OnDialogFpstestwithdisplay();
};


