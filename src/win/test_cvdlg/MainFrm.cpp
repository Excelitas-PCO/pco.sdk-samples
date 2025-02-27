// MainFrm.cpp : Implementation of classe CMainFrame
//

/*

ATTENTION: It is not possible to use the pco_conv.dll without a connected camera
from pco. Using this conversion software is restricted to PCO cameras only!

However, this application is enabled to work without a camera, but transferring the code to
your application will not work, due to the restriction.

*/

#include "stdafx.h"
#include "Test_cvDlg.h"

#include "FILE12.H"
#include "MainFrm.h"
#include "PCO_cDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)
BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
  ON_MESSAGE(WM_APP+1011, OnProcessDialog)
  ON_WM_CREATE()
  ON_WM_SETFOCUS()
  ON_COMMAND(ID_OPEN_CONVERTDIALOG, OnOpenConvertDialog)
  ON_WM_CLOSE()
  ON_COMMAND(ID_FILE_OPENFILE, OnFileOpenfile)
  ON_COMMAND(ID_BAYER0, OnBayer0)
  ON_UPDATE_COMMAND_UI(ID_BAYER0, OnUpdateBayer0)
  ON_COMMAND(ID_BAYER1, OnBayer1)
  ON_UPDATE_COMMAND_UI(ID_BAYER1, OnUpdateBayer1)
  ON_COMMAND(ID_BAYER2, OnBayer2)
  ON_UPDATE_COMMAND_UI(ID_BAYER2, OnUpdateBayer2)
  ON_COMMAND(ID_BAYER3, OnBayer3)
  ON_UPDATE_COMMAND_UI(ID_BAYER3, OnUpdateBayer3)
  ON_COMMAND(ID_FPSTEST, OnFPSTest)
  ON_UPDATE_COMMAND_UI(ID_FPSTEST, OnUpdateFPSTest)
  ON_COMMAND(ID_EDIT_BWCONVERT, &CMainFrame::OnEditBwconvert)
  ON_COMMAND(ID_EDIT_FLIPIMAGE, &CMainFrame::OnEditFlipimage)
  ON_COMMAND(ID_EDIT_MIRRORIMAGE, &CMainFrame::OnEditMirrorimage)
  ON_COMMAND(ID_EDIT_PSEUDOCONVERT, &CMainFrame::OnEditPseudoconvert)
  ON_COMMAND(ID_BITMAP_32BPP, &CMainFrame::OnBitmap32bpp)
  ON_COMMAND(ID_BITMAP_24BPP, &CMainFrame::OnBitmap24bpp)
  ON_COMMAND(ID_DIALOG_FPSTESTWITHDISPLAY, &CMainFrame::OnDialogFpstestwithdisplay)
END_MESSAGE_MAP()

CMainFrame::CMainFrame()
{
  input_image = NULL;
  output_image = NULL;
  m_ibpp=24;
  m_iMode = 0;
  m_iColMode=0;
  col_lut = NULL;
  m_hLut=NULL;
  m_hLutDialog = NULL;
  m_hConvertThread = NULL;
  m_iConvertType=PCO_COLOR_CONVERT;
  m_Flip=m_Mirror=FALSE;
  m_alloc_output=TRUE;
}

CMainFrame::~CMainFrame()
{
  if(input_image)
    free(input_image);
  if((m_alloc_output)&&(output_image))
    free(output_image);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  hwnd_frame = GetSafeHwnd();
  //::MessageBox(NULL, "4 attach", MB_OK, 0);
  if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
    CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
  {
    return -1;
  }


  int width; 
  int height;
  bool mode_sp;

  ////load test picture 
  printf("Loading TIF-file test\n");	 

  if((getsize_tif("test.tif", &width, &height, &mode_sp))!=0)
  {
    printf("Error loading file test.tif.\n"); 
    ::AfxMessageBox("Test.tif not found!", MB_ICONERROR, 0);
    exit(-1);
  }

  pic_width  = width; 
  pic_height = height;

  int size = width*height;

  // Allocate 16bit buffer
  input_image = (WORD *)malloc(size*sizeof(WORD));
  if(input_image==NULL) 
  {
    AfxMessageBox("Error allocating buffer", MB_ICONERROR, 0);
    exit(-1);
  };

  Bild bild;
  memset(&bild,0,sizeof(Bild));
  bild.bAlignUpper = TRUE;
  bild.iBitRes = 12;
  bild.pic12 = input_image;            // Set image pointer to Bild struct

  if(read_tif("test.tif", &bild, 0) != 0)
  {
    AfxMessageBox("Error reading tif", MB_ICONERROR, 0);
    exit(-1);
  };

// Allocate RGB buffer
  if(m_alloc_output)
  {
    int imod = (pic_width * 3) % 4;
    if(imod > 0)
      imod = 4 - imod;
    output_image = (byte *) malloc((imod + m_ibpp / 8 * pic_width)*pic_height*sizeof(byte));
  }
//create according drawing object in Childview
  m_wndView.CreateBitmap(m_ibpp,(void**)&output_image, pic_width , pic_height);

  if(output_image==NULL)
  {
    AfxMessageBox("Error allocating buffer", MB_ICONERROR, 0);
    exit(-1);
  };


  int iMajor, iMinor, iPatch, iBuild;
  char szInfo[50] = "printmodules";
  int iLen = 50;
  char szPath[_MAX_PATH];
  int iLenP = _MAX_PATH;
  int err = PCO_GetVersionInfoPCO_CONV(szInfo, iLen, szPath, iLenP, &iMajor, &iMinor, &iPatch, &iBuild);

  // This structure holds all the information necessary to setup a convert object.
  strsensorinf = { 0 };
  strsensorinf.wSize = sizeof(PCO_SensorInfo);// Set size of structure
  strsensorinf.iConversionFactor = 8;  // Set conversion factor of camera
  strsensorinf.iDarkOffset = 32;       // Set dark offset of camera
  strsensorinf.iDataBits = bild.iBitRes;         // Set bit resolution of camera
// Set color type and alignment of image data.
  strsensorinf.iSensorInfoBits = CONVERT_SENSOR_COLORIMAGE;// Set color camera type; 

  if(bild.bAlignUpper)
    strsensorinf.iSensorInfoBits |= CONVERT_SENSOR_UPPERALIGNED;

  // Set the color correction matrix information of the camera.
  strsensorinf.strColorCoeff.da11 = 1.4474; strsensorinf.strColorCoeff.da12 = -0.5856; strsensorinf.strColorCoeff.da13 = 0.1382;
  strsensorinf.strColorCoeff.da21 = -0.1961; strsensorinf.strColorCoeff.da22 = 1.4444; strsensorinf.strColorCoeff.da23 = -0.2401;
  strsensorinf.strColorCoeff.da31 = 0.1027; strsensorinf.strColorCoeff.da32 = -0.6059; strsensorinf.strColorCoeff.da33 = 1.5350;
  m_hLut = NULL;
  // PCO_BW_CONVERT (1)     -> Creates a bw convert object
  // PCO_COLOR_CONVERT (2)  -> Creates a color convert object
  // PCO_PSEUDO_CONVERT (3) -> Creates a pseudo convert object
  // PCO_COLOR16_CONVERT (4)-> Creates a 16bit color convert object

  // Create a color convert object
  err = PCO_ConvertCreate(&m_hLut, (PCO_SensorInfo*)&strsensorinf.wSize, m_iConvertType);

  PCO_Display strDisplay = {0};

  strDisplay.wSize = sizeof(PCO_Display);
  // Gets the PCO_Display structure
  err = PCO_ConvertGetDisplay(m_hLut, (PCO_Display*) &strDisplay.wSize);
  strDisplay.iScale_min = 64;          // min value for convert. This makes the test.tif looking good!
  strDisplay.iScale_max = 410;         // max value for convert.  
  // Sets the PCO_Display structure
  err = PCO_ConvertSetDisplay(m_hLut, (PCO_Display*) &strDisplay.wSize);

  SetMode(0, m_Flip, m_Mirror);
//SetMode is calling DoConvert()
//DoConvert is calling the appropriate convert_to_xxx function from the SDK

  int color_temp,tint;
  PCO_GetWhiteBalance(m_hLut,&color_temp,&tint,0,width,height,input_image,50,50,width-50,height-50);


  GetMenu()->CheckMenuItem(ID_BITMAP_24BPP,MF_CHECKED);
  GetMenu()->CheckMenuItem(ID_BITMAP_32BPP,MF_UNCHECKED);

  GetMenu()->GetSubMenu(1)->CheckMenuItem(0,MF_CHECKED|MF_BYPOSITION);
  GetMenu()->GetSubMenu(1)->CheckMenuItem(1,MF_UNCHECKED|MF_BYPOSITION);
  GetMenu()->GetSubMenu(1)->CheckMenuItem(2,MF_UNCHECKED|MF_BYPOSITION);

  GetMenu()->CheckMenuItem(ID_OPEN_CONVERTDIALOG,MF_UNCHECKED);
  return 0;
}


//#define DO_BAYER_CALL
void CMainFrame::SetMode(int bayer, bool bflip, bool bmirror)
{
  int imosaiker[4] = {BAYER_UPPER_LEFT_IS_RED, BAYER_UPPER_LEFT_IS_GREEN_RED,
                      BAYER_UPPER_LEFT_IS_GREEN_BLUE, BAYER_UPPER_LEFT_IS_BLUE};
  m_iColMode = imosaiker[bayer];
  // pco.camera series depend on the descriptor and ROI
  m_iMode = 0;
  if(bflip)
    m_iMode |= CONVERT_MODE_OUT_FLIPIMAGE;
  if(bmirror)
    m_iMode |= CONVERT_MODE_OUT_MIRRORIMAGE;
#if defined DO_BAYER_CALL
  PCO_Bayer strBayer;
  strBayer.wSize = sizeof(PCO_Bayer);
  strBayer.iColorMode = 0;
  strBayer.iKernel = imosaiker[bayer];
  PCO_ConvertSetBayer(m_hLut, (PCO_Bayer*)&strBayer.wSize);
  if (m_hLutDialog)
  {
    OnOpenConvertDialog();
    OnOpenConvertDialog();
  }
#else
  if (m_hLutDialog)
  {
    m_iMode |= CONVERT_MODE_OUT_DOHIST;
  }
#endif
  DoConvert();
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  if( !CFrameWnd::PreCreateWindow(cs) )
    return FALSE;

  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
  cs.lpszClass = AfxRegisterWndClass(0);
  return TRUE;
}

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
  CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
  CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame Event handler

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
  m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
    return TRUE;

  return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnOpenConvertDialog()
{
  if(m_hLutDialog != NULL)             // Already open -> close dialog
  {
    PCO_CloseConvertDialog(m_hLutDialog);
    m_hLutDialog = NULL;
    m_iMode = 0;
    GetMenu()->CheckMenuItem(ID_OPEN_CONVERTDIALOG,MF_UNCHECKED);
    return;
  }

  int width  = pic_width;
  int height = pic_height;

  m_iMode = CONVERT_MODE_OUT_DOHIST;
  m_hLutDialog = NULL;                 // Set dialog handle to NULL
  // Open a new dialog with WM_APP+1011 as message
  // and the previously created convert object for control.
  int err = PCO_OpenConvertDialog(&m_hLutDialog, GetSafeHwnd(), "Convert Dialog", WM_APP+1011, m_hLut, 410, 252);
  GetMenu()->CheckMenuItem(ID_OPEN_CONVERTDIALOG,MF_CHECKED);
  DoConvert();
}

void CMainFrame::OnClose()
{
  if(m_hLutDialog != NULL)             // Close dialog in case one is opened.
  {
    PCO_CloseConvertDialog(m_hLutDialog);
    GetMenu()->CheckMenuItem(ID_OPEN_CONVERTDIALOG,MF_UNCHECKED);
    m_hLutDialog = NULL;
  }
  PCO_ConvertDelete(m_hLut);           // Delete convert object
  CFrameWnd::OnClose();
}

LRESULT CMainFrame::OnProcessDialog(WPARAM a, LPARAM b)// All convert actions will run up here
{
  int color_temp;
  int tint;
  PCO_ConvDlg_Message *gl_strMessageInfo;

  gl_strMessageInfo = (PCO_ConvDlg_Message*)b;

  if(gl_strMessageInfo->wCommand == PCO_CNV_DLG_CMD_CLOSING)// Dialog is closing
  {
    GetMenu()->CheckMenuItem(ID_OPEN_CONVERTDIALOG,MF_UNCHECKED);
    m_hLutDialog = NULL;
    m_iMode = 0;
    return 0;
  }
  if(gl_strMessageInfo->wCommand == PCO_CNV_DLG_CMD_WHITEBALANCE)// White balance button pressed
  {
    if(m_iConvertType==PCO_COLOR_CONVERT)
    {
    // Get color_temp and tint in order to get a white balanced image.
     PCO_GetWhiteBalance(m_hLut, &color_temp, &tint, 0, pic_width, pic_height, input_image, 0, 0, pic_width, pic_height );

     PCO_Display strDisplay;
     strDisplay.wSize = sizeof(PCO_Display);
     PCO_ConvertGetDisplay(m_hLut, (PCO_Display*)&strDisplay.wSize);
     strDisplay.iColor_temp = color_temp;
     strDisplay.iColor_tint = tint;
     PCO_ConvertSetDisplay(m_hLut, (PCO_Display*)&strDisplay.wSize);



     PCO_SetConvertDialog(m_hLutDialog, NULL);// Reload actual data
    }
  }

  if(  (gl_strMessageInfo->wCommand == PCO_CNV_DLG_CMD_UPDATE)
     ||(gl_strMessageInfo->wCommand == PCO_CNV_DLG_CMD_MINMAX)
     ||(gl_strMessageInfo->wCommand == PCO_CNV_DLG_CMD_MINMAXSMALL)
    )
  {
   PCO_Display strDisplay;

   PCO_GetConvertDialog(m_hLutDialog,m_hLut);
   strDisplay.wSize = sizeof(PCO_Display);
   PCO_ConvertGetDisplay(m_hLut,&strDisplay);
   PCO_ConvertSetDisplay(m_hLut,&strDisplay);
  }
  DoConvert();
  return 0;
}

void CMainFrame::DoConvert()
{
  switch(m_iConvertType)
  {
   case PCO_BW_CONVERT:
    if(m_ibpp==8)
     PCO_Convert16TO8(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
    else if(m_ibpp==24)
    {
     m_iMode&=~CONVERT_MODE_OUT_RGB32;
     PCO_Convert16TO24(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
    }
    else if(m_ibpp==32)
    {
     m_iMode|=CONVERT_MODE_OUT_RGB32;
     PCO_Convert16TO24(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
    }
    break;

   case PCO_COLOR_CONVERT:
    if(m_ibpp==24)
    {
     m_iMode&=~CONVERT_MODE_OUT_RGB32;
     PCO_Convert16TOCOL(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
    }
    else if(m_ibpp==32)
    {
     m_iMode|=CONVERT_MODE_OUT_RGB32;
     PCO_Convert16TOCOL(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
    }
    break;

   case PCO_PSEUDO_CONVERT:
    if(m_ibpp==24)
    {
     m_iMode&=~CONVERT_MODE_OUT_RGB32;
     PCO_Convert16TOPSEUDO(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
    }
    else if(m_ibpp==32)
    {
     m_iMode|=CONVERT_MODE_OUT_RGB32;
     PCO_Convert16TOPSEUDO(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
    }
    break;

   default:
    break;
  }
// Display conversion result as histogram
  if(m_hLutDialog)
   //PCO_SetDataToDialog(m_hLutDialog, pic_width, pic_height, input_image, output_image);
   PCO_UpdateHistData(m_hLutDialog, pic_width, pic_height);

  m_wndView.SetData(output_image);
//  m_wndView.SetBitmap(output_image, pic_width ,pic_height);
  m_wndView.Invalidate(0);
  GdiFlush();
}

void CMainFrame::OnFileOpenfile()
{
  int width, height;
  bool mode_sp;
  WORD* input_ima=NULL;
  BYTE* output_ima=NULL;

  CFileDialog fo(TRUE, "tif-file|*.tif", "tif", 0, 0, this, 0, 1);
  if(fo.DoModal() != IDOK)
   return;

  if((getsize_tif((char*)(LPCTSTR)fo.GetPathName(), &width, &height, &mode_sp))!=0)
  {
   AfxMessageBox("Error loading image", MB_ICONERROR, 0);
   return;
  }

  int size = width*height;
  Bild bild;
  memset(&bild,0,sizeof(Bild));

  input_ima = (WORD *)malloc(size*sizeof(WORD));
  if(input_ima==NULL)
  {
   AfxMessageBox("Error allocating buffer", MB_ICONERROR, 0);
   return;
  }

  bild.pic12 = input_ima;
  if(read_tif((char*)(LPCTSTR)fo.GetPathName(), &bild, 0) != 0)
  {
   AfxMessageBox("Error reading image", MB_ICONERROR, 0);
   return;
  };

  pic_width  = width; 
  pic_height = height;

  if(m_alloc_output)
  {
    int imod = (pic_width * 3) % 4;
    if(imod > 0)
      imod = 4 - imod;
    output_ima = (byte *) malloc((imod + m_ibpp / 8 * pic_width)*pic_height*sizeof(byte));

   if(output_ima==NULL)
   {
    AfxMessageBox("Error allocating buffer", MB_ICONERROR, 0);
    return;
   }
  }

  if(input_image != NULL)
   free(input_image);
  if((m_alloc_output)&&(output_image != NULL))
   free(output_image);

  input_image=input_ima;
  output_image=output_ima;
  m_wndView.CreateBitmap(m_ibpp,(void**)&output_image,pic_width,pic_height);

  DoConvert();
}

void CMainFrame::OnBayer0()
{
  SetConvert(PCO_COLOR_CONVERT);
  SetMode(0, m_Flip, m_Mirror);
}

void CMainFrame::OnUpdateBayer0(CCmdUI *pCmdUI)
{
  if((m_iColMode & 0xFF) == BAYER_UPPER_LEFT_IS_RED)
    pCmdUI->SetCheck(1);
  else
    pCmdUI->SetCheck(0);
}

void CMainFrame::OnBayer1()
{
  SetConvert(PCO_COLOR_CONVERT);
  SetMode(1, m_Flip, m_Mirror);
}

void CMainFrame::OnUpdateBayer1(CCmdUI *pCmdUI)
{
  if((m_iColMode & 0xFF) == BAYER_UPPER_LEFT_IS_GREEN_RED)
    pCmdUI->SetCheck(1);
  else
    pCmdUI->SetCheck(0);
}

void CMainFrame::OnBayer2()
{
  SetConvert(PCO_COLOR_CONVERT);
  SetMode(2, m_Flip, m_Mirror);
}

void CMainFrame::OnUpdateBayer2(CCmdUI *pCmdUI)
{
  if((m_iColMode & 0xFF) == BAYER_UPPER_LEFT_IS_GREEN_BLUE)
    pCmdUI->SetCheck(1);
  else
    pCmdUI->SetCheck(0);
}

void CMainFrame::OnBayer3()
{
  SetConvert(PCO_COLOR_CONVERT);
  SetMode(3, m_Flip, m_Mirror);
}

void CMainFrame::OnUpdateBayer3(CCmdUI *pCmdUI)
{
  if((m_iColMode & 0xFF) == BAYER_UPPER_LEFT_IS_BLUE)
    pCmdUI->SetCheck(1);
  else
    pCmdUI->SetCheck(0);
}

void CMainFrame::OnFPSTest()
{
  CWaitCursor wait;
  double dt = 0.0;
  CString csh;
  MSG msg;
  int count;
  char text[100];
  char textc[200];

  if(m_iConvertType==PCO_COLOR_CONVERT)
   count=1000;
  else
   count=10000;

  GetWindowTextA(text,sizeof(text));
  TT();
  for(int i = 0; i < count; i++)
  {
   switch(m_iConvertType)
   {
    case PCO_BW_CONVERT:
     if(m_ibpp==8)
     PCO_Convert16TO8(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
     else if(m_ibpp==24)
     {
      m_iMode&=~CONVERT_MODE_OUT_RGB32;
      PCO_Convert16TO24(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
     }
     else if(m_ibpp==32)
     {
      m_iMode|=CONVERT_MODE_OUT_RGB32;
      PCO_Convert16TO24(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
     }
     break;

    case PCO_COLOR_CONVERT:
     if(m_ibpp==24)
     {
      m_iMode&=~CONVERT_MODE_OUT_RGB32;
      PCO_Convert16TOCOL(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
     }
     else if(m_ibpp==32)
     {
      m_iMode|=CONVERT_MODE_OUT_RGB32;
      PCO_Convert16TOCOL(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
     }
     break;

    case PCO_PSEUDO_CONVERT:
     if(m_ibpp==24)
     {
      m_iMode&=~CONVERT_MODE_OUT_RGB32;
      PCO_Convert16TOPSEUDO(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
     }
     else if(m_ibpp==32)
     {
      m_iMode|=CONVERT_MODE_OUT_RGB32;
      PCO_Convert16TOPSEUDO(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);
     }
     break;

    default:
     break;
   }
//   PCO_Convert16TOCOL(m_hLut, m_iMode, m_iColMode, pic_width, pic_height, input_image, output_image);

   sprintf_s(textc,sizeof(textc),"%s -- %03d of %d loops done --",text,i+1,count);
   SetWindowTextA(textc);
   while(PeekMessage(&msg,GetSafeHwnd(),0,0,PM_REMOVE))
   {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
   }

  }
  dt = TT();
  csh.Format("Done %d conversions in %4.2f sec. %4.2f fps",count, dt, (float)count/dt);
  AfxMessageBox((LPCTSTR)csh, MB_ICONINFORMATION | MB_OK, 0);
  SetWindowTextA(text);
  DoConvert();
}

void CMainFrame::OnUpdateFPSTest(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(0);
}


void CMainFrame::OnDialogFpstestwithdisplay()
{
  CWaitCursor wait;
  double dt = 0.0;
  CString csh;
  MSG msg;
  int count,err;
  char text[100];
  char textc[200];
  int iScale_max;

  PCO_Display strDisplay = {0};

  strDisplay.wSize = sizeof(PCO_Display);
  err = PCO_ConvertGetDisplay(m_hLut,&strDisplay);
  iScale_max=strDisplay.iScale_max;
  err = PCO_ConvertSetDisplay(m_hLut,&strDisplay);

  if(m_iConvertType==PCO_COLOR_CONVERT)
   count=1000;
  else
   count=10000;

  GetWindowTextA(text,sizeof(text));
  TT();
  for(int i = 0; i < count; i++)
  {
   if(m_iConvertType!=PCO_COLOR_CONVERT)
   {
    strDisplay.iScale_max=iScale_max+(i%100)*4;
    err = PCO_ConvertSetDisplay(m_hLut,&strDisplay);
   }
   else
   {
    strDisplay.iScale_max=iScale_max+(i%20)*20;
    err = PCO_ConvertSetDisplay(m_hLut,&strDisplay);
   }

   DoConvert();
   sprintf_s(textc,sizeof(textc),"%s -- %03d of %d loops done --",text,i+1,count);
   SetWindowTextA(textc);
   while(PeekMessage(&msg,GetSafeHwnd(),0,0,PM_REMOVE))
   {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
   }
  }
  dt = TT();
  csh.Format("Done %d conversions in %4.2f sec. %4.2f fps",count, dt, (float)count/dt);
  AfxMessageBox((LPCTSTR)csh, MB_ICONINFORMATION | MB_OK, 0);
  SetWindowTextA(text);
  strDisplay.iScale_max=iScale_max;
  err = PCO_ConvertSetDisplay(m_hLut,&strDisplay);
  DoConvert();
}



double CMainFrame::TT()
{
  __int64 T1, T2;
  double dT;
  static double dTTF = 0.0;// Float for frequ.
  static LARGE_INTEGER LITime1, LITime2;// Time buffer

  if(dTTF == 0.0)
  {
    LARGE_INTEGER LIFrequ;

    QueryPerformanceFrequency(&LIFrequ);
    QueryPerformanceCounter(&LITime1);
    dTTF = (double)LIFrequ.QuadPart;
    return 0.0;
  }

  QueryPerformanceCounter(&LITime2);
  T1 = LITime1.QuadPart;
  T2 = LITime2.QuadPart;

  dT = (double)T2 - (double)T1;
  dT /= dTTF;

  LITime1.QuadPart = LITime2.QuadPart;
  return dT;
}

void CMainFrame::OnEditBwconvert()
{
  SetConvert(PCO_BW_CONVERT);
  SetMode(0, m_Flip, m_Mirror);
}

void CMainFrame::OnEditPseudoconvert()
{
  SetConvert(PCO_PSEUDO_CONVERT);
  SetMode(0, m_Flip, m_Mirror);
}


void CMainFrame::SetConvert(int convert)
{
  if(m_iConvertType!=convert)
  {
   m_iConvertType=convert;
   if(m_hLutDialog)
   {
    PCO_CloseConvertDialog(m_hLutDialog);
    GetMenu()->CheckMenuItem(ID_OPEN_CONVERTDIALOG,MF_UNCHECKED);
    m_hLutDialog=NULL;
   }

   if(m_hLut)
   {
    PCO_ConvertDelete(m_hLut);
    m_hLut=NULL;
   }
   if(m_iConvertType==PCO_COLOR_CONVERT)
    strsensorinf.iSensorInfoBits |= CONVERT_SENSOR_COLORIMAGE;// Set color camera type; 
   else
    strsensorinf.iSensorInfoBits&=~CONVERT_SENSOR_COLORIMAGE;

   int err = PCO_ConvertCreate(&m_hLut,&strsensorinf, m_iConvertType);
   PCO_Display strDisplay = {0};

   strDisplay.wSize = sizeof(PCO_Display);
  // Gets the PCO_Display structure
   err = PCO_ConvertGetDisplay(m_hLut, (PCO_Display*) &strDisplay.wSize);
   strDisplay.iScale_min = 64;          // min value for convert. This makes the test.tif looking good!
   strDisplay.iScale_max = 410;         // max value for convert.  
  // Sets the PCO_Display structure
   err = PCO_ConvertSetDisplay(m_hLut, (PCO_Display*) &strDisplay.wSize);
  }


  switch(m_iConvertType)
  {
   case PCO_BW_CONVERT:
    GetMenu()->GetSubMenu(1)->CheckMenuItem(0,MF_UNCHECKED|MF_BYPOSITION);
    GetMenu()->GetSubMenu(1)->CheckMenuItem(1,MF_CHECKED|MF_BYPOSITION);
    GetMenu()->GetSubMenu(1)->CheckMenuItem(2,MF_UNCHECKED|MF_BYPOSITION);
    break;

   case PCO_COLOR_CONVERT:
    GetMenu()->GetSubMenu(1)->CheckMenuItem(0,MF_CHECKED|MF_BYPOSITION);
    GetMenu()->GetSubMenu(1)->CheckMenuItem(1,MF_UNCHECKED|MF_BYPOSITION);
    GetMenu()->GetSubMenu(1)->CheckMenuItem(2,MF_UNCHECKED|MF_BYPOSITION);
    break;

   case PCO_PSEUDO_CONVERT:
    GetMenu()->GetSubMenu(1)->CheckMenuItem(0,MF_UNCHECKED|MF_BYPOSITION);
    GetMenu()->GetSubMenu(1)->CheckMenuItem(1,MF_UNCHECKED|MF_BYPOSITION);
    GetMenu()->GetSubMenu(1)->CheckMenuItem(2,MF_CHECKED|MF_BYPOSITION);
    break;

   default:
    break;
  }
}


void CMainFrame::OnEditFlipimage()
{
  if(m_Flip)
  {
   m_Flip=FALSE;
   GetMenu()->CheckMenuItem(ID_EDIT_FLIPIMAGE,MF_UNCHECKED);
  }
  else
  {
   m_Flip=TRUE;
   GetMenu()->CheckMenuItem(ID_EDIT_FLIPIMAGE,MF_CHECKED);
  }
  switch(m_iColMode & 0xFF)
  {
   case BAYER_UPPER_LEFT_IS_RED:
    SetMode(0, m_Flip, m_Mirror);
    break;

   case BAYER_UPPER_LEFT_IS_GREEN_RED:
    SetMode(1, m_Flip, m_Mirror);
    break;

   case BAYER_UPPER_LEFT_IS_GREEN_BLUE:
    SetMode(2, m_Flip, m_Mirror);
    break;

   case BAYER_UPPER_LEFT_IS_BLUE:
    SetMode(3, m_Flip, m_Mirror);
    break;
  }
}

void CMainFrame::OnEditMirrorimage()
{
  if(m_Mirror)
  {
   m_Mirror=FALSE;
   GetMenu()->CheckMenuItem(ID_EDIT_MIRRORIMAGE,MF_UNCHECKED);
  }
  else
  {
   m_Mirror=TRUE;
   GetMenu()->CheckMenuItem(ID_EDIT_MIRRORIMAGE,MF_CHECKED);
  }

  switch(m_iColMode & 0xFF)
  {
   case BAYER_UPPER_LEFT_IS_RED:
    SetMode(0, m_Flip, m_Mirror);
    break;

   case BAYER_UPPER_LEFT_IS_GREEN_RED:
    SetMode(1, m_Flip, m_Mirror);
    break;

   case BAYER_UPPER_LEFT_IS_GREEN_BLUE:
    SetMode(2, m_Flip, m_Mirror);
    break;

   case BAYER_UPPER_LEFT_IS_BLUE:
    SetMode(3, m_Flip, m_Mirror);
    break;
  }
}



void CMainFrame::OnBitmap32bpp()
{
  if(m_ibpp!=32)
  {
   GetMenu()->CheckMenuItem(ID_BITMAP_24BPP,MF_UNCHECKED);
   GetMenu()->CheckMenuItem(ID_BITMAP_32BPP,MF_CHECKED);
   m_ibpp=32;
   if(m_alloc_output)
   {
    if(output_image != NULL)
     free(output_image);
    output_image=(BYTE*)malloc(m_ibpp/8*pic_width*pic_height);
   }
   else
    output_image = NULL;
   m_wndView.CreateBitmap(m_ibpp,(void**)&output_image,pic_width,pic_height);
  }
  DoConvert();
}

void CMainFrame::OnBitmap24bpp()
{
  if(m_ibpp!=24)
  {
   GetMenu()->CheckMenuItem(ID_BITMAP_24BPP,MF_CHECKED);
   GetMenu()->CheckMenuItem(ID_BITMAP_32BPP,MF_UNCHECKED);
   m_ibpp=24;
   int imod = (pic_width * 3) % 4;
   if(imod > 0)
     imod = 4 - imod;
   if(m_alloc_output)
   {
    if(output_image != NULL)
     free(output_image);
    output_image=(BYTE*)malloc((imod + m_ibpp/8*pic_width)*pic_height);
   }
   else
    output_image = NULL;
   m_wndView.CreateBitmap(m_ibpp,(void**)&output_image,pic_width,pic_height);
  }
  DoConvert();
}

