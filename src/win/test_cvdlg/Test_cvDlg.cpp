// Test_cvDlg.cpp : Defines class for application.
//

#include "stdafx.h"
#include "Test_cvDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTest_cvDlgApp

BEGIN_MESSAGE_MAP(CTest_cvDlgApp, CWinApp)
  ON_COMMAND(ID_APP_ABOUT, &CTest_cvDlgApp::OnAppAbout)
END_MESSAGE_MAP()


// Create CTest_cvDlgApp

CTest_cvDlgApp::CTest_cvDlgApp()
{
}


// Only one CTest_cvDlgApp-Objekt

CTest_cvDlgApp theApp;

int CTest_cvDlgApp::ExitInstance()
{
  // TODO: Add your specialized code here and/or call the base class
  //PCO_RemoveAppName(m_iNum);
  return CWinApp::ExitInstance();
}
// CTest_cvDlgApp-Initialisierung

BOOL CTest_cvDlgApp::InitInstance()
{
  unsigned char ucappname[20];
  sprintf_s((char*)&ucappname[0], 20, "pcotest_cvdlg");
  //m_iNum = PCO_SetAppName(ucappname);

  INITCOMMONCONTROLSEX InitCtrls;
  InitCtrls.dwSize = sizeof(InitCtrls);

  InitCtrls.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&InitCtrls);

  CWinApp::InitInstance();

  // Init OLE
  if (!AfxOleInit())
  {
    AfxMessageBox(IDP_OLE_INIT_FAILED);
    return FALSE;
  }
  AfxEnableControlContainer();

  SetRegistryKey(_T("pco_testcvdlg"));
  CMainFrame* pFrame = new CMainFrame;
  if (!pFrame)
    return FALSE;
  m_pMainWnd = pFrame;
  pFrame->LoadFrame(IDR_MAINFRAME,
    WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
    NULL);

  pFrame->ShowWindow(SW_SHOW);
  pFrame->UpdateWindow();
  return TRUE;
}

class CAboutDlg : public CDialog
{
public:
  CAboutDlg();

  // Dialog data
  enum { IDD = IDD_ABOUTBOX };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-support

  // Implementation
protected:
  DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

void CTest_cvDlgApp::OnAppAbout()
{
  CAboutDlg aboutDlg;
  aboutDlg.DoModal();
}


