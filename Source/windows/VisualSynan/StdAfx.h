#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

// MFC core and standard components
#include <afxwin.h>         
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "morph_dict/common/utilit.h"
#include <tchar.h>
#include <map>

extern CString FromRMLEncode(std::string s);
