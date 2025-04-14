// VisualSynanView.cpp : implementation of the CVisualSynanView class
//

#include "StdAfx.h"
#include "VisualSynan.h"

#include "VisualSynanDoc.h"
#include "VisualSynanView.h"
#include "Resource.h"
#include "wingdi.h"
#include <locale>
#include <clocale>

CFont	CVisualSynanView::m_FontForWords;
CFont	CVisualSynanView::m_FontForGroupNames;
CFont	CVisualSynanView::m_BoldFontForWords;
CFont	CVisualSynanView::m_BoldUnderlineFontForWords;
CFont	CVisualSynanView::m_UnderlineFontForWords;
CFont	CVisualSynanView::m_SubjectFontForWords;
CFont	CVisualSynanView::m_PredicateFontForWords;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_HOMONYMS_MENU_ITEM 1000
#define ID_WORD_TOOL 1100
/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView

IMPLEMENT_DYNCREATE(CVisualSynanView, CScrollView)

BEGIN_MESSAGE_MAP(CVisualSynanView, CScrollView)
	ON_WM_CONTEXTMENU()
	//{{AFX_MSG_MAP(CVisualSynanView)
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnNeedText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView construction/destruction

CVisualSynanView::CVisualSynanView()
{
	// Set locale for proper Russian text handling
	setlocale(LC_ALL, "Russian");
	std::locale::global(std::locale("Russian"));

	// TODO: add construction code here
	m_pHomonymsArray = NULL;
	m_bDefaultFont = TRUE;
	m_bExistUsefulFont = FALSE;
	m_iActiveWordTT = -1;
	m_iActiveSentenceTT = -1;
	m_iActiveWord = -1;
	m_iActiveSentence = -1;
	m_bShowGroups = TRUE;
	m_bFirsTime = TRUE;
	m_bMore = TRUE;
	m_iStartSent = 0;
	m_iStartLine = 0;
	m_iOffset = 0;
	m_nVScrollPos = 0;
	SetScrollSizes(MM_TEXT, CSize(0,0));
}

CVisualSynanView::~CVisualSynanView()
{
	
}

BOOL CVisualSynanView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView drawing

void CVisualSynanView::OnDraw(CDC* pDC)
{
	CVisualSynanDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView printing

BOOL CVisualSynanView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CVisualSynanView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CVisualSynanView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView diagnostics

#ifdef _DEBUG
void CVisualSynanView::AssertValid() const
{
	CView::AssertValid();
}

void CVisualSynanView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CVisualSynanDoc* CVisualSynanView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument != nullptr);
	if (!m_pDocument) {
		OutputDebugString(_T("[VisualSynan] Error: m_pDocument is null\n"));
		return nullptr;
	}
	if (!m_pDocument->IsKindOf(RUNTIME_CLASS(CVisualSynanDoc))) {
		OutputDebugString(_T("[VisualSynan] Error: Document is not CVisualSynanDoc\n"));
		return nullptr;
	}
	return (CVisualSynanDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView message handlers

void CVisualSynanView::OnPaint() 
{
	// Set locale for proper Russian text handling
	try {
		setlocale(LC_ALL, "Russian_Russia.1251");
		std::locale::global(std::locale("Russian_Russia.1251"));
	}
	catch (...) {
		OutputDebugString(_T("[VisualSynan] Failed to set Russian locale\n"));
	}

	CVisualSynanDoc* pDoc = GetDocument();
	if (!pDoc) {
		OutputDebugString(_T("[VisualSynan] Error: Document is null in OnPaint\n"));
		return;
	}
	
	// Get the work time string and convert it properly for display
	try {
		std::string workTimeStr = _U8(pDoc->m_VisualSentences.m_WorkTimeStr);
		CString S;
		
		// Convert from internal Windows-1251 to UTF-8 then to wide string for CString
		std::string utf8Str = convert_to_utf8(workTimeStr, morphRussian);
		S = utf8_to_wstring(utf8Str).c_str();
		
		// Add null pointer protection for status bar message
		CVisualSynanApp* pApp = (CVisualSynanApp*)AfxGetApp();
		if (pApp && pApp->m_pMainWnd) {
			CFrameWnd* pFrame = (CFrameWnd*)pApp->m_pMainWnd;
			if (pFrame && pFrame->GetSafeHwnd()) {
				pFrame->SetMessageText(S);
			}
		}
	}
	catch (const convert_exception& e) {
		OutputDebugString(_T("[VisualSynan] Text conversion error: "));
		OutputDebugString(_U16(e.what()));
		OutputDebugString(_T("\n"));
	}
	catch (...) {
		OutputDebugString(_T("[VisualSynan] Unknown error in text conversion\n"));
	}

	CClientDC clDC(this);
	CView::OnPaint();

	CRect clientRect;	
	CDC memDC;
	CBitmap bmBmp;
	
	CRect rectDevice;

	try {
		//creating memory DC
		GetClientRect(&clientRect);
		rectDevice = clientRect;
		OnPrepareDC(&clDC);
		clDC.DPtoLP(&clientRect);
		bmBmp.CreateCompatibleBitmap(&clDC, rectDevice.right , rectDevice.bottom);
		memDC.CreateCompatibleDC(&clDC);

		CBitmap* pOldBitmap = memDC.SelectObject(&bmBmp);
		
		// Создаем градиентный фон
		COLORREF colorStart = RGB(248, 248, 255);  // Светло-голубой
		COLORREF colorEnd = RGB(255, 255, 255);    // Белый
		
		for(int i = 0; i < rectDevice.Height(); i++)
		{
			int r = GetRValue(colorStart) + (GetRValue(colorEnd) - GetRValue(colorStart)) * i / rectDevice.Height();
			int g = GetGValue(colorStart) + (GetGValue(colorEnd) - GetGValue(colorStart)) * i / rectDevice.Height();
			int b = GetBValue(colorStart) + (GetBValue(colorEnd) - GetBValue(colorStart)) * i / rectDevice.Height();
			
			COLORREF color = RGB(r, g, b);
			memDC.FillSolidRect(0, i, rectDevice.Width(), 1, color);
		}

		// Устанавливаем улучшенное качество отрисовки
		memDC.SetBkMode(TRANSPARENT);

		//selecting choosen font
		CFont* pOldFont = nullptr;

		if(m_bExistUsefulFont) {
			pOldFont = memDC.SelectObject(&m_FontForWords);
		}

		if(m_bFirsTime && !pDoc->NoSentences()) {
			m_bFirsTime = FALSE;
			pDoc->CalculateCoordinates(&memDC, clientRect.right, m_bShowGroups);
		}

		int iOffset = clientRect.top - rectDevice.top;

		//drawing sentences in memory DC
		pDoc->PrintSentences(&memDC, clientRect, iOffset);

		//drawing it on the screen
		clDC.BitBlt(0, clientRect.top, rectDevice.right, rectDevice.bottom, &memDC, 0, 0, SRCCOPY);

		//restoring old bitmap and old font
		if(pOldBitmap) memDC.SelectObject(pOldBitmap);
		if(pOldFont) memDC.SelectObject(pOldFont);
		memDC.DeleteDC();	
		
		ResizeScroll();
	}
	catch (...) {
		OutputDebugString(_T("[VisualSynan] Exception in drawing code\n"));
	}
}

void CVisualSynanView::OnRButtonDown(UINT nFlags, CPoint point) 
{

	CView::OnRButtonDown(nFlags, point);
}

void CVisualSynanView::OnContextMenu(CWnd*, CPoint point)
{
	CClientDC dc(NULL);
	OnPrepareDC(&dc);
	CPoint ClientPoint = point;
	ScreenToClient(&ClientPoint);
	dc.DPtoLP(&ClientPoint);		
	
	

	// CG: This block was added by the Pop-up Menu component
	{
		if (point.x == -1 && point.y == -1){
			//keystroke invocation
			CRect rect;
			GetClientRect(rect);
			//ClientToScreen(rect);
			ClientToScreen(rect);
			point = rect.TopLeft();
			point.Offset(5, 5);
		}

				
		BOOL bInSomeWord;

		bInSomeWord = GetDocument()->GetHomonymsArray(ClientPoint,&m_pHomonymsArray,&m_iActiveSentence,&m_iActiveWord);
		if(bInSomeWord)
		{
			CMenu menu;
			menu.CreatePopupMenu();
			
			CString strLemma;
			for(int i = 0 ; i < m_pHomonymsArray->GetSize() ; i++)
			{
				strLemma = ((CVisualHomonym*)m_pHomonymsArray->GetAt(i))->m_strLemma;
				CString grm = ((CVisualHomonym*)m_pHomonymsArray->GetAt(i))->m_strCommonGrammems;
				if (!grm.IsEmpty())
					strLemma += " " + grm;;
				menu.AppendMenu(MF_STRING | MF_ENABLED,ID_HOMONYMS_MENU_ITEM + i, strLemma);
			}

			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
				this);
		}
	}
}

BOOL CVisualSynanView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{

	if(pHandlerInfo == NULL)
	{
		if(m_pHomonymsArray)
			if(( (nID >= ID_HOMONYMS_MENU_ITEM) && (nID < ID_HOMONYMS_MENU_ITEM + (UINT)(m_pHomonymsArray->GetSize()))) && (nCode == CN_COMMAND) )			
			{
				BOOL bInvalidate;
				bInvalidate = GetDocument()->SetActiveHomonym(m_iActiveSentence, m_iActiveWord,nID - ID_HOMONYMS_MENU_ITEM);
				if(bInvalidate)
				{
					CClientDC dc(this);
					Recalculate(dc);
					Invalidate();
				}
			}
	}
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}






int CALLBACK GetFefaultFontEx(
  const LOGFONT FAR * elfLogFont,  // pointer to logical-font data
  const TEXTMETRIC FAR *lpntm,  // pointer to physical-font data
  unsigned long FontType,            // type of font
  LPARAM lParam            ) // pointer to application-defined data);
{
	// Make sure we have valid parameters
	if (!elfLogFont || !lParam) {
		return 1;
	}

	if( (FontType & TRUETYPE_FONTTYPE) && (elfLogFont->lfCharSet & RUSSIAN_CHARSET) )
	{
		CVisualSynanView* pView = (CVisualSynanView*)lParam;
		
		// Safety check for view object
		if (!pView || !::IsWindow(pView->m_hWnd)) {
			return 1;
		}
		
		// Copy the font properties safely
		ZeroMemory(&(pView->m_LogFontForWords), sizeof(LOGFONT));

		pView->m_LogFontForWords.lfCharSet = RUSSIAN_CHARSET;
		pView->m_LogFontForWords.lfClipPrecision = elfLogFont->lfClipPrecision;
		pView->m_LogFontForWords.lfEscapement = elfLogFont->lfEscapement;
		
		// Safe string copy to prevent buffer overruns
		if (wcslen(elfLogFont->lfFaceName) < LF_FACESIZE) {
			wcscpy(pView->m_LogFontForWords.lfFaceName, elfLogFont->lfFaceName);
		} else {
			wcscpy(pView->m_LogFontForWords.lfFaceName, L"Times New Roman");
		}
		
		pView->m_LogFontForWords.lfHeight = 24;
		pView->m_LogFontForWords.lfItalic = elfLogFont->lfItalic;
		pView->m_LogFontForWords.lfOrientation = elfLogFont->lfOrientation;
		pView->m_LogFontForWords.lfOutPrecision = elfLogFont->lfOutPrecision;
		pView->m_LogFontForWords.lfPitchAndFamily = elfLogFont->lfPitchAndFamily;
		pView->m_LogFontForWords.lfQuality = CLEARTYPE_QUALITY; // Use ClearType for better readability
		pView->m_LogFontForWords.lfStrikeOut = elfLogFont->lfStrikeOut;
		pView->m_LogFontForWords.lfUnderline = elfLogFont->lfUnderline;
		pView->m_LogFontForWords.lfWeight = elfLogFont->lfWeight;
		pView->m_LogFontForWords.lfWidth = 0;
		
		pView->m_bExistUsefulFont = TRUE;
		
		return 0;
	}

	return 1;
}


int CALLBACK TestIfTrueTypeEx(
  const LOGFONT FAR *lpelf,  // pointer to logical-font data
  const TEXTMETRIC FAR *lpntm,  // pointer to physical-font data
  unsigned long FontType,            // type of font
  LPARAM lParam            ) // pointer to application-defined data);
{

	CVisualSynanView* pView = (CVisualSynanView*)lParam;
	if( !wcscmp(pView->m_LogFontForWords.lfFaceName,lpelf->lfFaceName))
	{
		if( FontType & TRUETYPE_FONTTYPE )		
			pView->m_bExistUsefulFont = TRUE;
		else
			pView->m_bExistUsefulFont = FALSE;

		return 0;
	}

	return 1;
}





void CVisualSynanView::UpdateFontsFromLogFont() 
{
	// Validate font name length to prevent buffer overruns
	if (wcslen(m_LogFontForWords.lfFaceName) >= LF_FACESIZE) {
		OutputDebugString(_T("[VisualSynan] Error: Font face name too long in UpdateFontsFromLogFont\n"));
		m_LogFontForWords.lfFaceName[LF_FACESIZE-1] = 0;
	}

	// Delete any existing fonts to prevent memory leaks
	if (m_FontForWords.m_hObject) m_FontForWords.DeleteObject();
	if (m_FontForGroupNames.m_hObject) m_FontForGroupNames.DeleteObject();
	if (m_BoldFontForWords.m_hObject) m_BoldFontForWords.DeleteObject();
	if (m_BoldUnderlineFontForWords.m_hObject) m_BoldUnderlineFontForWords.DeleteObject();
	if (m_UnderlineFontForWords.m_hObject) m_UnderlineFontForWords.DeleteObject();
	if (m_SubjectFontForWords.m_hObject) m_SubjectFontForWords.DeleteObject();
	if (m_PredicateFontForWords.m_hObject) m_PredicateFontForWords.DeleteObject();

	// Основной шрифт с улучшенным сглаживанием
	LOGFONT logFont = m_LogFontForWords;
	logFont.lfQuality = CLEARTYPE_QUALITY;  // Используем ClearType
	
	// Validate height and width values
	if (logFont.lfHeight == 0) logFont.lfHeight = 24;
	if (logFont.lfWidth < 0) logFont.lfWidth = 0;
	
	BOOL fontCreated = m_FontForWords.CreateFontIndirect(&logFont);
	if (!fontCreated) {
		OutputDebugString(_T("[VisualSynan] Error: Failed to create main font\n"));
		return;
	}

	// Шрифт для имен групп - более компактный
	logFont.lfHeight = (m_LogFontForWords.lfHeight/3) * 2;
	if (logFont.lfHeight == 0) logFont.lfHeight = 16;
	logFont.lfWidth = (m_LogFontForWords.lfWidth/3) * 2;
	if (logFont.lfWidth < 0) logFont.lfWidth = 0;
	
	fontCreated = m_FontForGroupNames.CreateFontIndirect(&logFont);
	if (!fontCreated) {
		OutputDebugString(_T("[VisualSynan] Error: Failed to create group names font\n"));
	}

	// Жирный шрифт с улучшенным начертанием
	logFont = m_LogFontForWords;
	logFont.lfWeight = FW_BOLD;
	
	fontCreated = m_BoldFontForWords.CreateFontIndirect(&logFont);
	if (!fontCreated) {
		OutputDebugString(_T("[VisualSynan] Error: Failed to create bold font\n"));
	}

	// Жирный подчеркнутый шрифт
	logFont.lfUnderline = TRUE;
	
	fontCreated = m_BoldUnderlineFontForWords.CreateFontIndirect(&logFont);
	if (!fontCreated) {
		OutputDebugString(_T("[VisualSynan] Error: Failed to create bold underline font\n"));
	}

	// Подчеркнутый шрифт
	logFont = m_LogFontForWords;
	logFont.lfUnderline = TRUE;
	
	fontCreated = m_UnderlineFontForWords.CreateFontIndirect(&logFont);
	if (!fontCreated) {
		OutputDebugString(_T("[VisualSynan] Error: Failed to create underline font\n"));
	}
	
	// Специальный шрифт для подлежащего - полужирный с подчеркиванием и увеличенный
	logFont = m_LogFontForWords;
	logFont.lfWeight = FW_BOLD;
	logFont.lfUnderline = TRUE;
	logFont.lfHeight = m_LogFontForWords.lfHeight + 2; // Немного крупнее
	logFont.lfQuality = CLEARTYPE_QUALITY;
	
	fontCreated = m_SubjectFontForWords.CreateFontIndirect(&logFont);
	if (!fontCreated) {
		OutputDebugString(_T("[VisualSynan] Error: Failed to create subject font\n"));
	}
	
	// Специальный шрифт для сказуемого - с подчеркиванием и курсивом
	logFont = m_LogFontForWords;
	logFont.lfWeight = FW_BOLD;
	logFont.lfUnderline = TRUE;
	logFont.lfItalic = TRUE;  // Курсив для сказуемых
	logFont.lfQuality = CLEARTYPE_QUALITY;
	
	fontCreated = m_PredicateFontForWords.CreateFontIndirect(&logFont);
	if (!fontCreated) {
		OutputDebugString(_T("[VisualSynan] Error: Failed to create predicate font\n"));
	}
}

void CVisualSynanView::OnInitialUpdate() 
{
	try {
		CView::OnInitialUpdate();    
		if (!m_hWnd) {
			OutputDebugString(_T("[VisualSynan] Error: Window handle is invalid in OnInitialUpdate\n"));
			return;
		}

		CClientDC dc(this);
		LOGFONT lfFont;
		ZeroMemory(&lfFont, sizeof(LOGFONT));
		wcscpy(lfFont.lfFaceName,_T("Times New Roman"));
		lfFont.lfCharSet = RUSSIAN_CHARSET;
		
		EnumFontFamiliesEx(dc.m_hDC, &lfFont, &GetFefaultFontEx,(LPARAM)this,0);
		if(!m_bExistUsefulFont) {
			EnumFontFamiliesEx(dc.m_hDC, NULL, &GetFefaultFontEx,(LPARAM)this,0);
		}

		if(m_bExistUsefulFont) {
			UpdateFontsFromLogFont();
		} else {
			OutputDebugString(_T("[VisualSynan] Warning: No suitable font found\n"));
		}

		//creating tooltip ctrl
		EnableToolTips();
		if(!m_ctrlToolTip.Create(this)) {
			OutputDebugString(_T("[VisualSynan] Error: Failed to create tooltip control\n"));
			return;
		}

		CRect StupidRect(0,0,0,0);
		if(!m_ctrlToolTip.AddTool(this, LPSTR_TEXTCALLBACK, StupidRect, ID_WORD_TOOL)) {
			OutputDebugString(_T("[VisualSynan] Error: Failed to add tooltip tool\n"));
			return;
		}

		m_ctrlToolTip.Activate(TRUE);
		m_ctrlToolTip.SetDelayTime(TTDT_AUTOPOP,1000000);
		ResizeScroll();
	}
	catch(...) {
		OutputDebugString(_T("[VisualSynan] Exception in OnInitialUpdate\n"));
	}
}

void CVisualSynanView::ResizeScroll()
{
	CClientDC dc(NULL);
	OnPrepareDC(&dc);
	CSize sizeDoc = GetDocument()->GetSentencesSize();
	dc.LPtoDP(&sizeDoc);
	SetScrollSizes(MM_TEXT, sizeDoc);
}

 void CVisualSynanView::Recalculate(CDC& clDC, CPrintInfo* pInfo)
{
	if( GetDocument()->NoSentences() )
		return;

	//selecting choosen font
	CFont* pOldFont = NULL;
	
	//creating memory DC
	CRect clientRect;
	GetClientRect(&clientRect);
	OnPrepareDC(&clDC,pInfo);
	clDC.DPtoLP(&clientRect);

	if( m_bExistUsefulFont)
	{
		pOldFont = clDC.SelectObject(&m_FontForWords);
	}

	//calculating sentences coordinates 
	GetDocument()->CalculateCoordinates(&clDC,clientRect.right, m_bShowGroups);

	if( pOldFont )
		clDC.SelectObject(pOldFont);

}


void CVisualSynanView::OnSize(UINT nType, int cx, int cy) 
{
			
	ResizeScroll();
	CView::OnSize(nType, cx, cy);	
	CClientDC clDC(this);
	Recalculate(clDC);
	Invalidate();


}

void CVisualSynanView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if( lHint != NULL)
	{
		ResizeScroll();
		CVisualSynanView::OnUpdate(pSender, lHint, pHint);
	}
}


void CVisualSynanView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	
	CScrollView::OnPrepareDC(pDC, pInfo);
	if( pInfo )
	{		
		if( m_iStartSent == -1 )
		{
			pInfo->m_bContinuePrinting = FALSE;
			m_iStartSent = 0;
			m_iStartLine = 0;
			m_iOffset = 0;
			CClientDC dc(this);
			Recalculate(dc);
			return;
		}

		pInfo->m_bContinuePrinting = TRUE;
		
		pDC->SetMapMode(MM_ANISOTROPIC);		
		CSize sizeDoc = GetDocument()->GetSentencesSize();		
		if( (sizeDoc.cx == 0) && (sizeDoc.cy == 0) )
		{
			sizeDoc.cx = 200;
			sizeDoc.cy = 200;
		}
		
		pDC->SetWindowExt(sizeDoc); 

		int xLogPixPerInch = pDC->GetDeviceCaps(LOGPIXELSX);
		int yLogPixPerInch = pDC->GetDeviceCaps(LOGPIXELSY);

		long xExt = (long)sizeDoc.cx * xLogPixPerInch;
		xExt /= 100 ;
		long yExt = (long)sizeDoc.cy * yLogPixPerInch;
		yExt /= 100 ;
		pDC->SetViewportExt((int)xExt, (int)yExt);
	}

}

void CVisualSynanView::LogFontCpy(LOGFONT* dstFont, LOGFONT srcFont)
{
		dstFont->lfCharSet = srcFont.lfCharSet;
		dstFont->lfClipPrecision = srcFont.lfClipPrecision;
		dstFont->lfEscapement = srcFont.lfEscapement;
		wcscpy(dstFont->lfFaceName,srcFont.lfFaceName);
		dstFont->lfHeight = srcFont.lfHeight;
		dstFont->lfItalic = srcFont.lfItalic;
		dstFont->lfOrientation = srcFont.lfOrientation;
		dstFont->lfOutPrecision = srcFont.lfOutPrecision;
		dstFont->lfPitchAndFamily = srcFont.lfPitchAndFamily;
		dstFont->lfQuality = srcFont.lfQuality;
		dstFont->lfStrikeOut = srcFont.lfStrikeOut;
		dstFont->lfUnderline = srcFont.lfUnderline;
		dstFont->lfWeight = srcFont.lfWeight;
		dstFont->lfWidth = srcFont.lfWidth;		
}

BOOL CVisualSynanView::PreTranslateMessage(MSG* pMsg) 
{
	try {
		if(!pMsg) return TRUE;

		if(pMsg->message == WM_LBUTTONDOWN ||
		   pMsg->message == WM_LBUTTONUP ||
		   pMsg->message == WM_MOUSEMOVE) 
		{
			CVisualSynanDoc* pDoc = GetDocument();
			if(!pDoc) {
				return CScrollView::PreTranslateMessage(pMsg);
			}

			CClientDC dc(NULL);
			OnPrepareDC(&dc);
			BOOL bInSomeWord;
			CPoint ClientPoint = pMsg->pt;
			ScreenToClient(&ClientPoint);
			dc.DPtoLP(&ClientPoint);
			
			bInSomeWord = pDoc->GetHomonymsArray(ClientPoint, NULL, &m_iActiveSentenceTT, &m_iActiveWordTT);

			if(bInSomeWord && ::IsWindow(m_ctrlToolTip.m_hWnd))
			{                
				dc.LPtoDP(&ClientPoint);
				CRect rect(ClientPoint.x - 1, ClientPoint.y - 1, ClientPoint.x + 1, ClientPoint.y + 1);
				m_ctrlToolTip.SetToolRect(this, ID_WORD_TOOL, rect);
				m_ctrlToolTip.RelayEvent(pMsg);
			}            
			else
			{
				m_iActiveWordTT = -1;
				m_iActiveSentenceTT = -1;
			}
		}
		return CScrollView::PreTranslateMessage(pMsg);
	}
	catch(...) {
		OutputDebugString(_T("[VisualSynan] Exception in PreTranslateMessage\n"));
		return TRUE;
	}
}

int CVisualSynanView::OnNeedText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	try
	{
		if( (m_iActiveSentenceTT != -1) && (m_iActiveWordTT != -1))
		{
			TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
			CString strLemma, strGramChar;

			//getting lemma and grammatiacl characteristics of the active word
			BOOL bRes = GetDocument()->GetActiveHomDescr(m_iActiveSentenceTT,m_iActiveWordTT,strLemma,strGramChar);
			if(bRes)
			{
				CString s = strLemma + CString(" ") + strGramChar;
				if (s.GetLength() > 80)
				{
					s.Delete(76, s.GetLength() - 76); 
					s += "...";
				};
				ASSERT(s.GetLength() < 80);
				wcscpy(pTTT->szText, s);

				return FALSE;
			}
		}
		return TRUE;
	}
	catch(...)
	{
		return TRUE;
	}
}


void CVisualSynanView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    // Получаем информацию о скроллбаре
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    GetScrollInfo(SB_VERT, &si);
    
    // Текущая позиция скролла
    int nCurPos = si.nPos;
    int nPrevPos = nCurPos;
    
    // Определяем новую позицию в зависимости от действия
    switch (nSBCode)
    {
    case SB_TOP:        // Прокрутка к началу
        nCurPos = si.nMin;
        break;
        
    case SB_BOTTOM:     // Прокрутка к концу
        nCurPos = si.nMax;
        break;
        
    case SB_LINEUP:     // Прокрутка на одну строку вверх
        nCurPos -= 40;
        break;
        
    case SB_LINEDOWN:   // Прокрутка на одну строку вниз
        nCurPos += 40;
        break;
        
    case SB_PAGEUP:     // Прокрутка на страницу вверх
        nCurPos -= si.nPage;
        break;
        
    case SB_PAGEDOWN:   // Прокрутка на страницу вниз
        nCurPos += si.nPage;
        break;
        
    case SB_THUMBTRACK: // Перетаскивание ползунка
    case SB_THUMBPOSITION:
        nCurPos = nPos;
        break;
    }
    
    // Ограничиваем позицию скролла
    nCurPos = max(si.nMin, min(nCurPos, (int)si.nMax - (int)si.nPage + 1));
    
    // Если позиция изменилась - выполняем прокрутку
    if (nCurPos != nPrevPos)
    {
        SetScrollPos(SB_VERT, nCurPos);
        ScrollWindow(0, (nPrevPos - nCurPos));
        UpdateWindow();
    }
}

void CVisualSynanView::Reset()
{
	m_bFirsTime = TRUE;
	GetDocument()->Reset();
}

void CVisualSynanView::OnBuildRels(CString& str) 
{
	GetDocument()->BuildRels(str);		
}


void CVisualSynanView::OnViewTest() 
{
	// Реализация тестового метода
	// Можно оставить пустым или добавить нужную функциональность
}

BOOL CVisualSynanView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // Получаем текущую позицию скролла
    int nScrollPos = GetScrollPos(SB_VERT);
    
    // Настраиваем скорость скроллинга (можно регулировать множитель)
    int nScrollInc = max(40, GetSystemMetrics(SM_CYHSCROLL) * 2);
    
    if (zDelta < 0)  // Прокрутка вниз
        nScrollPos += nScrollInc;
    else             // Прокрутка вверх
        nScrollPos -= nScrollInc;
    
    // Ограничиваем позицию скролла
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    GetScrollInfo(SB_VERT, &si);
    
    nScrollPos = max(si.nMin, min(nScrollPos, (int)si.nMax - (int)si.nPage + 1));
    
    // Плавно прокручиваем к новой позиции
    SetScrollPos(SB_VERT, nScrollPos);
    ScrollWindow(0, (m_nVScrollPos - nScrollPos));
    m_nVScrollPos = nScrollPos;
    
    UpdateWindow();
    return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}
