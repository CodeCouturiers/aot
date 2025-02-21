// VisualSynan.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"
#include "WaitThread.h"
#include "VisualSynan.h"
#include "SynReport.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "VisualSynanDoc.h"
#include "VisualSynanView.h"
#include "Splash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only CVisualSynanApp object

CVisualSynanApp theApp;

// For MFC Windows applications
int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
    OutputDebugString(_T("\n[VisualSynan] Starting wWinMain\n"));

    // Initialize MFC and print and error on failure
    if (!AfxWinInit(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
    {
        OutputDebugString(_T("[VisualSynan] Fatal Error: MFC initialization failed\n"));
        return 1;
    }
    OutputDebugString(_T("[VisualSynan] MFC initialized successfully\n"));

    // Initialize application instance
    if (!theApp.InitApplication())
    {
        OutputDebugString(_T("[VisualSynan] Fatal Error: Application initialization failed\n"));
        return 1;
    }
    OutputDebugString(_T("[VisualSynan] Application initialized successfully\n"));

    // Initialize instance
    if (!theApp.InitInstance())
    {
        OutputDebugString(_T("[VisualSynan] Fatal Error: Instance initialization failed\n"));
        return 1;
    }
    OutputDebugString(_T("[VisualSynan] Instance initialized successfully, starting message loop\n"));

    return theApp.Run();
}

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanApp

IMPLEMENT_DYNCREATE(CVisualSynanApp, CWinApp)

BEGIN_MESSAGE_MAP(CVisualSynanApp, CWinApp)
	//{{AFX_MSG_MAP(CVisualSynanApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnSynFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	//}}AFX_MSG_MAP
	// Standard file based document commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanApp initialization

void CVisualSynanApp::OnSynFileNew()
{
	CDocTemplate* T = GetSynTemplate();
	CDocument* pDoc = T->CreateNewDocument();
	T->InitialUpdateFrame(T->CreateNewFrame(pDoc, NULL), pDoc, TRUE);
}

class CVisualSynanCommandLineInfo  : public CCommandLineInfo
{
public:
	MorphLanguageEnum m_Language;

	CVisualSynanCommandLineInfo () : CCommandLineInfo () {
		m_Language = morphRussian;
	};
	
	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
	{
		CString S = pszParam;
		MorphLanguageEnum l;
		std::string utfstr = wstring_to_utf8(std::wstring(pszParam));
		if (GetLanguageByString(utfstr.c_str(), l))
			m_Language = l;
		CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
	};
};

static MorphLanguageEnum GlobalLanguage;
static CSyntaxHolder Rus(morphRussian);
static CSyntaxHolder Ger(morphGerman);

void CVisualSynanApp::SetLanguage(MorphLanguageEnum l) {
	GlobalLanguage = l;
}

CSyntaxHolder& CVisualSynanApp::GetHolder() {
	if (GlobalLanguage == morphGerman)
		return Ger;
	else
		return Rus;
}

BOOL CVisualSynanApp::InitInstance()
{
    OutputDebugString(_T("\n[VisualSynan] Starting InitInstance\n"));

    // Get the executable path and set working directory
    TCHAR exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    CString strExePath(exePath);
    CString strExeDir = strExePath.Left(strExePath.ReverseFind('\\'));
    
    // Set paths
    CString strBaseDir = _T("C:\\RML");
    CString strDictsDir = strBaseDir + _T("\\Dicts\\Morph\\Russian");
    
    // Log actual file system entries with full details
    OutputDebugString(_T("\n[VisualSynan] Scanning directory contents:\n"));
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(strDictsDir + _T("\\*.*"), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            CString fileName = findData.cFileName;
            CString fullPath = strDictsDir + _T("\\") + fileName;
            CString fileInfo;
            fileInfo.Format(_T("[VisualSynan] Found: %s (Attributes: 0x%08X)\n"), fullPath, findData.dwFileAttributes);
            OutputDebugString(fileInfo);
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    } else {
        CString errMsg;
        errMsg.Format(_T("[VisualSynan] Failed to scan directory: %s (Error: %d)\n"), strDictsDir, GetLastError());
        OutputDebugString(errMsg);
    }

    // Check for required dictionary files with case-insensitive search
    const TCHAR* requiredFiles[] = {
        _T("morph.bin"),
        _T("MORPH.BIN"),    // Try uppercase
        _T("Morph.bin")     // Try mixed case
    };

    bool filesFound[1] = {false}; // только morphs
    CString foundPaths[1];

    // First pass - find files with any case
    hFind = FindFirstFile(strDictsDir + _T("\\*.*"), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            CString currentFile = findData.cFileName;
            currentFile.MakeLower(); // Convert to lowercase for comparison

            for (int i = 0; i < 3; i++) {  // теперь проверяем только 3 варианта для morphs.bin
                CString reqFile = requiredFiles[i];
                reqFile.MakeLower();
                
                if (currentFile == reqFile) {
                    filesFound[0] = true;
                    foundPaths[0] = strDictsDir + _T("\\") + findData.cFileName; // Store original filename
                    
                    CString foundMsg;
                    foundMsg.Format(_T("[VisualSynan] Found dictionary file: %s\n"), foundPaths[0]);
                    OutputDebugString(foundMsg);
                }
            }
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }

    // Check results and set correct paths
    if (!filesFound[0]) {
        CString errMsg;
        errMsg.Format(_T("Missing required dictionary file:\nmorphs.bin\nDirectory: %s\n"), strDictsDir);
        AfxMessageBox(errMsg, MB_ICONERROR);
        OutputDebugString(_T("[VisualSynan] ") + errMsg);
        return FALSE;
    }

    // Use the found paths for further processing
    OutputDebugString(_T("[VisualSynan] Using dictionary file:\n"));
    CString logMsg;
    logMsg.Format(_T("[VisualSynan] %s\n"), foundPaths[0]);
    OutputDebugString(logMsg);

    // Set working directory if all checks pass
    if (!SetCurrentDirectory(strDictsDir)) {
        CString errMsg;
        errMsg.Format(_T("Failed to set working directory to:\n%s\nError code: %d"), strDictsDir, GetLastError());
        AfxMessageBox(errMsg, MB_ICONERROR);
        OutputDebugString(_T("[VisualSynan] Error: Failed to set working directory\n"));
        return FALSE;
    }
    
    // Log the directories for debugging
    CString dirInfo;
    dirInfo.Format(_T("[VisualSynan] Executable directory: %s\n[VisualSynan] Dictionary directory: %s\n"), 
                   strExeDir, strDictsDir);
    OutputDebugString(dirInfo);

    // CG: The following block was added by the Splash Screen component.
    {
        CCommandLineInfo cmdInfo;
        ParseCommandLine(cmdInfo);
        CSplashWnd::EnableSplashScreen(cmdInfo.m_bShowSplash);
        OutputDebugString(_T("[VisualSynan] Splash screen initialized\n"));
    }

    // Create main MDI Frame window first, before other initialization
    OutputDebugString(_T("[VisualSynan] Creating main frame...\n"));
    CMainFrame* pMainFrame = new CMainFrame;
    if (!pMainFrame) {
        OutputDebugString(_T("[VisualSynan] Failed to create main frame!\n"));
        return FALSE;
    }

    if (!pMainFrame->LoadFrame(IDR_MAINFRAME)) {
        OutputDebugString(_T("[VisualSynan] Failed to load main frame!\n"));
        delete pMainFrame;
        return FALSE;
    }
    
    m_pMainWnd = pMainFrame;
    if (!m_pMainWnd) {
        OutputDebugString(_T("[VisualSynan] Critical Error: m_pMainWnd is NULL after assignment!\n"));
        return FALSE;
    }
    OutputDebugString(_T("[VisualSynan] Main frame created and initialized\n"));

    // Initialize morphology
    OutputDebugString(_T("[VisualSynan] Initializing morphology...\n"));
    CVisualSynanCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);
    SetLanguage(cmdInfo.m_Language);
    
    CString langInfo;
    langInfo.Format(_T("[VisualSynan] Selected language: %s\n"), 
                    cmdInfo.m_Language == morphGerman ? _T("German") : _T("Russian"));
    OutputDebugString(langInfo);

    try {
        // Initialize morphology holders first
        OutputDebugString(_T("[VisualSynan] Loading morphological dictionaries...\n"));
        
        // Initialize Russian morphology
        OutputDebugString(_T("[VisualSynan] Loading Russian morphological dictionary...\n"));
        try {
            GlobalLoadMorphHolder(morphRussian);
            if (!GetMHolder(morphRussian).m_pLemmatizer) {
                OutputDebugString(_T("[VisualSynan] Failed to load Russian morphological dictionary - lemmatizer is null!\n"));
                AfxMessageBox(_T("Failed to load Russian morphological dictionary."), MB_ICONERROR);
                return FALSE;
            }
        }
        catch (const std::exception& e) {
            CStringA errorMsg(e.what());
            CString debugMsg;
            debugMsg.Format(_T("[VisualSynan] Exception while loading Russian morphology: %S\n"), errorMsg);
            OutputDebugString(debugMsg);
            AfxMessageBox(_T("Failed to load Russian morphological dictionary."), MB_ICONERROR);
            return FALSE;
        }

        // German morphology disabled for now
        /*
        // Initialize German morphology
        OutputDebugString(_T("[VisualSynan] Loading German morphological dictionary...\n"));
        try {
            GlobalLoadMorphHolder(morphGerman);
            if (!GetMHolder(morphGerman).m_pLemmatizer) {
                OutputDebugString(_T("[VisualSynan] Failed to load German morphological dictionary - lemmatizer is null!\n"));
                AfxMessageBox(_T("Failed to load German morphological dictionary."), MB_ICONERROR);
                return FALSE;
            }
        }
        catch (const std::exception& e) {
            CStringA errorMsg(e.what());
            CString debugMsg;
            debugMsg.Format(_T("[VisualSynan] Exception while loading German morphology: %S\n"), errorMsg);
            OutputDebugString(debugMsg);
            AfxMessageBox(_T("Failed to load German morphological dictionary."), MB_ICONERROR);
            return FALSE;
        }
        */

        // Now initialize syntax
        OutputDebugString(_T("[VisualSynan] Creating syntax options...\n"));
        if (!Rus.m_Synan.CreateOptions(morphRussian)) {
            OutputDebugString(_T("[VisualSynan] Failed to create Russian syntax options!\n"));
            AfxMessageBox(_T("Failed to create Russian syntax options."), MB_ICONERROR);
            return FALSE;
        }

        // German syntax disabled
        /*
        if (!Ger.m_Synan.CreateOptions(morphGerman)) {
            OutputDebugString(_T("[VisualSynan] Failed to create German syntax options!\n"));
            AfxMessageBox(_T("Failed to create German syntax options."), MB_ICONERROR);
            return FALSE;
        }
        */

        // Initialize syntax processors
        OutputDebugString(_T("[VisualSynan] Initializing Russian syntax...\n"));
        Rus.m_Synan.InitializeProcesser();

        // German syntax processor disabled
        /*
        OutputDebugString(_T("[VisualSynan] Initializing German syntax...\n"));
        Ger.m_Synan.InitializeProcesser();
        */

        // Load syntax rules for the selected language
        OutputDebugString(_T("[VisualSynan] Loading syntax rules...\n"));
        GetHolder().LoadSyntax();
        OutputDebugString(_T("[VisualSynan] Morphology initialized successfully\n"));
    }
    catch (const std::exception& e) {
        CStringA errorMsg(e.what());
        CString debugMsg;
        debugMsg.Format(_T("[VisualSynan] Exception while loading morphology: %S\n"), errorMsg);
        OutputDebugString(debugMsg);
        AfxMessageBox(_T("Error initializing morphology. Please check log for details."), MB_ICONERROR);
        return FALSE;
    }
    catch (...) {
        OutputDebugString(_T("[VisualSynan] Unknown exception while loading morphology!\n"));
        AfxMessageBox(_T("Unexpected error while initializing morphology."), MB_ICONERROR);
        return FALSE;
    }

    CWaitThread::m_hEventKill = CreateEvent(NULL, FALSE, FALSE, NULL);
    OutputDebugString(_T("[VisualSynan] Wait thread event created\n"));

    CoInitialize(NULL);
    OutputDebugString(_T("[VisualSynan] COM initialized\n"));

    AfxEnableControlContainer();
    OutputDebugString(_T("[VisualSynan] Control container enabled\n"));

    // Change the registry key under which our settings are stored.
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));
    OutputDebugString(_T("[VisualSynan] Registry key set\n"));

    LoadStdProfileSettings();
    OutputDebugString(_T("[VisualSynan] Profile settings loaded\n"));

    // Register document templates
    OutputDebugString(_T("[VisualSynan] Creating document template...\n"));
    m_pSynTemplate = new CMultiDocTemplate(
        IDR_VISUALTYPE,
        RUNTIME_CLASS(CVisualSynanDoc),
        RUNTIME_CLASS(CChildFrame),
        RUNTIME_CLASS(CVisualSynanView));
    
    if (!m_pSynTemplate) {
        OutputDebugString(_T("[VisualSynan] Failed to create document template!\n"));
        return FALSE;
    }
    
    AddDocTemplate(m_pSynTemplate);
    OutputDebugString(_T("[VisualSynan] Document template created and added\n"));

    // Show and update main window
    OutputDebugString(_T("[VisualSynan] Showing main window...\n"));
    pMainFrame->ShowWindow(SW_SHOW);
    pMainFrame->UpdateWindow();
    OutputDebugString(_T("[VisualSynan] Main window shown and updated\n"));

    // Create initial document
    OutputDebugString(_T("[VisualSynan] Creating initial document...\n"));
    OnSynFileNew();
    OutputDebugString(_T("[VisualSynan] Initial document created\n"));

    // Double check main window pointer before returning
    if (!m_pMainWnd) {
        OutputDebugString(_T("[VisualSynan] Critical Error: m_pMainWnd is NULL at end of InitInstance!\n"));
        return FALSE;
    }

    OutputDebugString(_T("[VisualSynan] InitInstance completed successfully\n"));
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CVisualSynanApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanApp message handlers

int CVisualSynanApp::ExitInstance() 
{
	Rus.ClearHolder();
	Ger.ClearHolder();
	CloseHandle(CWaitThread::m_hEventKill);
	return CWinApp::ExitInstance();
}

BOOL CVisualSynanApp::PreTranslateMessage(MSG* pMsg)
{
	if (CSplashWnd::PreTranslateAppMessage(pMsg))
		return TRUE;

	return CWinApp::PreTranslateMessage(pMsg);
}

