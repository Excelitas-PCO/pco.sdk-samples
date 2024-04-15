// Test_cvDlg.h : Hauptheaderdatei f�r die Test_cvDlg-Anwendung
//
#pragma once

#ifndef __AFXWIN_H__
#error "include \"stdafx.h\" before this file"
#endif

#include "resource.h"       // Hauptsymbole


// CTest_cvDlgApp:
// Siehe Test_cvDlg.cpp f�r die Implementierung dieser Klasse
//

class CTest_cvDlgApp : public CWinApp
{
public:
  CTest_cvDlgApp();


  // �berschreibungen
public:
  virtual BOOL InitInstance();
  virtual BOOL ExitInstance();
  int m_iNum;
  // Implementierung

public:
  afx_msg void OnAppAbout();
  DECLARE_MESSAGE_MAP()
};

extern CTest_cvDlgApp theApp;