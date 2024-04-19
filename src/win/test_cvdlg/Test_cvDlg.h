// Test_cvDlg.h : main header file for test_cvDlg application
//
#pragma once

#ifndef __AFXWIN_H__
#error "add \"stdafx.h\" before adding this to PCH"
#endif

#include "resource.h"       // symbols


// CTest_cvDlgApp:
//

class CTest_cvDlgApp : public CWinApp
{
public:
  CTest_cvDlgApp();


public:
  virtual BOOL InitInstance();
  virtual BOOL ExitInstance();
  int m_iNum;

public:
  afx_msg void OnAppAbout();
  DECLARE_MESSAGE_MAP()
};

extern CTest_cvDlgApp theApp;