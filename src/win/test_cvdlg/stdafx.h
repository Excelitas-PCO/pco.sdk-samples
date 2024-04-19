// stdafx.h : Include file for std system included

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif
#ifndef WINVER
#define WINVER 0x0601
#endif


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructores are explizit

#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC-core components
#include <afxext.h>         // MFC-extensions


#include <afxdisp.h>        // MFC-automate classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <pco_err.h>
#include <pco_color_corr_coeff.h>
#include <pco_convstructures.h>
#include <pco_convexport.h>
#include <pco_convdlgexport.h>







#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


