// ChildView.h : Target class CChildView
//

#pragma once

// CChildView-Fenster

class CChildView : public CWnd
{
  // Constructor
public:

  CChildView();

  // Attributes
public:

public:
  int SetBitmap(byte* pdata, int iwidth, int iheight);
  int CreateBitmap(int ibpp, void** pdata, int iwidth, int iheight);
  void SetData(byte* pdata);
  void SetData(byte* pdata, int iwidth, int iheight,int x_off,int y_off);

protected:
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
  virtual ~CChildView();
protected:
  HBITMAP m_handleBitmap;
  int m_iXRes, m_iYRes,m_Pix_byte;
  unsigned char *pic8;

protected:

  DECLARE_MESSAGE_MAP()
  afx_msg void OnPaint();
public:
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

