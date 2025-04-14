// VisualSynanDoc.cpp : implementation of the CVisualSynanDoc class
//

#include "StdAfx.h"
#include "VisualSynan.h"
#include "MainFrm.h"
#include "VisualSynanDoc.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanDoc

IMPLEMENT_DYNCREATE(CVisualSynanDoc, CDocument)

BEGIN_MESSAGE_MAP(CVisualSynanDoc, CDocument)
	//{{AFX_MSG_MAP(CVisualSynanDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanDoc construction/destruction

CVisualSynanDoc::CVisualSynanDoc()
{
}

CVisualSynanDoc::~CVisualSynanDoc()
{
}

BOOL CVisualSynanDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CVisualSynanDoc serialization

void CVisualSynanDoc::Serialize(CArchive& ar)
{
	// For serialization, we'll save the document in raw text format
	if (ar.IsStoring())
	{
		try {
			// Get original text from the view if available
			CString text;
			POSITION pos = GetFirstViewPosition();
			CView* pFirstView = pos ? GetNextView(pos) : NULL;
			
			// Try to get text from the view
			if (pFirstView) {
				// Check if it's a CEditView
				CEditView* pEditView = DYNAMIC_DOWNCAST(CEditView, pFirstView);
				if (pEditView) {
					// Get text from the edit view
					pEditView->GetWindowText(text);
				}
			}
			
			// If no text from view, construct from sentences
			if (text.IsEmpty()) {
				// Placeholder text
				text = _T("# Syntax analysis results saved by VisualSynan\r\n");
				
				// Count sentences
				int sentCount = m_VisualSentences.SentCount();
				CString sentCountStr;
				sentCountStr.Format(_T("# Number of sentences: %d\r\n\r\n"), sentCount);
				text += sentCountStr;
			}
			
			// Write the text to the archive
			ar << text;
		}
		catch (...) {
			AfxMessageBox(_T("Error occurred while saving document."), MB_ICONERROR);
		}
	}
	else
	{
		// Loading is handled by OnOpenDocument
	}
}

BOOL CVisualSynanDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	try {
		// Use the standard document save functionality
		BOOL result = CDocument::OnSaveDocument(lpszPathName);
		if (result) {
			SetModifiedFlag(FALSE);
		}
		return result;
	}
	catch (CFileException* e) {
		e->ReportError();
		e->Delete();
		return FALSE;
	}
	catch (...) {
		AfxMessageBox(_T("An error occurred while saving the document."), MB_ICONERROR);
		return FALSE;
	}
}

void CVisualSynanDoc::PreCloseFrame( CFrameWnd* pFrame )
{
	CDocument::PreCloseFrame(pFrame);
}


/////////////////////////////////////////////////////////////////////////////
// CVisualSynanDoc diagnostics

#ifdef _DEBUG
void CVisualSynanDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CVisualSynanDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanDoc commands

static BOOL GetSentencesFromSynAn(CVisualSynanDoc& C, CString strText, BOOL bFile) 
{
	try {
		CTime StartTime = CTime::GetCurrentTime();
		CVisualSynanApp* A = (CVisualSynanApp*)AfxGetApp();
		std::string s = wstring_to_utf8(std::wstring(strText));
		
		bool bRes = A->GetHolder().GetSentencesFromSynAn(s.c_str(), bFile);
		if( !bRes )
			return FALSE;
			
		bRes = C.m_VisualSentences.FillSentencesArray(A->GetHolder().m_Synan);
		CTime EndTime = CTime::GetCurrentTime();
		CTimeSpan Span = EndTime - StartTime;
		C.m_WorkTimeStr = Span.Format("%M min %S sec");
		return bRes;
	}
	catch (...)
	{
		return FALSE;
	};

};


BOOL CVisualSynanDoc::ProcessString(CString strText) 
{
	return GetSentencesFromSynAn(*this, strText, FALSE);
}

BOOL CVisualSynanDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	((CMainFrame*)::AfxGetMainWnd())->m_bNewDoc = FALSE;
	CString strPath = lpszPathName;
	strPath.MakeLower();
	return GetSentencesFromSynAn(*this, strPath, TRUE);
}


BOOL CVisualSynanDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	SetModifiedFlag (FALSE);
	return true;
}

BOOL CVisualSynanDoc::SaveModified() 
{
	// TODO: Add your specialized code here and/or call the base class
	SetModifiedFlag (FALSE);
	return TRUE;
}
