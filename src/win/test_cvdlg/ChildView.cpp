// ChildView.cpp : Implementation of class CChildView
//

#include "stdafx.h"
#include "Test_cvDlg.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView
CChildView::CChildView()
{
  m_iXRes = -1;
  m_iYRes = -1;
  m_Pix_byte=0;
  m_handleBitmap = NULL;
}

CChildView::~CChildView()
{
  ::DeleteObject(m_handleBitmap);
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
  ON_WM_CREATE()
  ON_WM_SIZE()
  ON_WM_PAINT()
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

int CChildView::SetBitmap(byte* pdata, int iwidth, int iheight)
{
  BITMAPINFO bmi;
  HDC hchelper;

  ZeroMemory(&bmi, sizeof(bmi));

  // Fill out the fields you care about.
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = iwidth;
  bmi.bmiHeader.biHeight =-iheight;
  bmi.bmiHeader.biSizeImage = iwidth * iheight * 3;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 24;
  bmi.bmiHeader.biCompression = BI_RGB;
  hchelper = ::GetDC(NULL);

  m_iXRes = iwidth;
  m_iYRes = iheight;
  m_Pix_byte=bmi.bmiHeader.biBitCount/8;
  // Create the surface.
  ::DeleteObject(m_handleBitmap);
  m_handleBitmap = CreateDIBSection(hchelper, &bmi, DIB_RGB_COLORS,(void **)&pic8, NULL, 0);
  memcpy(pic8, pdata, m_iXRes * m_iYRes * m_Pix_byte);
  return 0;
}

int CChildView::CreateBitmap(int ibpp, void** pdata, int iwidth, int iheight)
{
  HDC hchelper;

  hchelper = ::GetDC(NULL);
  if(ibpp==8)
  {
   int q;
   struct
   {
    BITMAPINFOHEADER bmiHeader;
    unsigned short Colors[256];
   }
   bmi;

   ZeroMemory(&bmi, sizeof(bmi));
   for(q=0;q<256;q++)
    bmi.Colors[q] = (short)(q/2);

   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = iwidth;
   bmi.bmiHeader.biHeight =-iheight;
   bmi.bmiHeader.biBitCount = ibpp;
   bmi.bmiHeader.biSizeImage = iwidth * iheight * bmi.bmiHeader.biBitCount/8;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biCompression = BI_RGB;

   if(m_handleBitmap)
    ::DeleteObject(m_handleBitmap);
   m_handleBitmap = CreateDIBSection(hchelper,(BITMAPINFO *)&bmi, DIB_PAL_COLORS,(void **)&pic8, NULL, 0);
  }
  else
  {
   BITMAPINFO bmi;
   ZeroMemory(&bmi, sizeof(bmi));

   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = iwidth;
   bmi.bmiHeader.biHeight =-iheight;
   bmi.bmiHeader.biBitCount = ibpp;
   bmi.bmiHeader.biSizeImage = iwidth * iheight * bmi.bmiHeader.biBitCount/8;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biCompression = BI_RGB;

   if(m_handleBitmap)
    ::DeleteObject(m_handleBitmap);
   m_handleBitmap = CreateDIBSection(hchelper, &bmi, DIB_RGB_COLORS,(void **)&pic8, NULL, 0);
  }

  m_iXRes = iwidth;
  m_iYRes = iheight;
  m_Pix_byte=ibpp/8;
  if(*pdata==NULL)
   *pdata=pic8;

  ::ReleaseDC(NULL, hchelper);
  return 0;
}


void CChildView::SetData(byte* pdata)
{
  if(pdata==pic8)
   return;
  else
  {
   memcpy(pic8, pdata, m_iXRes * m_iYRes * m_Pix_byte);
   return;
  }
}

void CChildView::SetData(byte* pdata, int iwidth, int iheight,int x_off,int y_off)
{
  unsigned char* adr_out;
  unsigned char* adr_in;
  for(int y=y_off;y<iheight;y++)
  {
   adr_out=pic8+x_off*m_Pix_byte+m_iXRes*m_Pix_byte*y;
   adr_in=pdata+x_off*m_Pix_byte+iwidth*m_Pix_byte*y;
   memcpy(adr_out,adr_in,(iwidth-x_off)*m_Pix_byte);
  }
  RECT rect;
  rect.top=0;
  rect.bottom=iheight;
  rect.left=0;
  rect.right=iwidth;

  InvalidateRect(&rect);
  OnPaint();
}


BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
  if (!CWnd::PreCreateWindow(cs))
    return FALSE;

  cs.dwExStyle |= WS_EX_CLIENTEDGE;
  cs.style &= ~WS_BORDER;
  cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
    ::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

  return TRUE;
}

void CChildView::OnPaint() 
{
  CPaintDC dc(this);
  CDC memdc;
  HBITMAP* pold;
  CRect rect;

  dc.SetStretchBltMode(HALFTONE);

  GetClientRect(&rect);

  if(m_handleBitmap)
  {
    memdc.CreateCompatibleDC(&dc);
    pold = (HBITMAP*)memdc.SelectObject(m_handleBitmap);
    dc.StretchBlt(0,0, rect.Width(), rect.Height(), &memdc, 0, 0, m_iXRes, m_iYRes, SRCCOPY);
//    dc.BitBlt(0,0, rect.Width(), rect.Height(), &memdc, 0, 0, SRCCOPY);
    memdc.SelectObject(pold);
    memdc.DeleteDC();
  }
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;//CWnd::OnEraseBkgnd(pDC);
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;
  return 0;
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
  CWnd::OnSize(nType, cx, cy);
  CRect rect;

  GetClientRect(&rect);
}
