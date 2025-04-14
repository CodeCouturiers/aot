// VisualSynanDoc.h : interface of the CVisualSynanDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VISUALSYNANDOC_H__1C505B3F_C4B2_11D2_8BB6_00105A68ADF3__INCLUDED_)
#define AFX_VISUALSYNANDOC_H__1C505B3F_C4B2_11D2_8BB6_00105A68ADF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "VisualSentences.h"

// Helper functions for string conversion
inline std::string _U8(CString s) {
	return wstring_to_utf8((const TCHAR*)s);
}

inline CString _U16(std::string s) {
	return utf8_to_wstring(s.c_str()).c_str();
}

class CVisualSynanDoc : public CDocument
{
protected: // create from serialization only
	CVisualSynanDoc();
	DECLARE_DYNCREATE(CVisualSynanDoc)

// Attributes
public:
	CVisualSentences m_VisualSentences;
	CString			 m_WorkTimeStr;

// Operations
public:

	BOOL ProcessString(CString strText);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVisualSynanDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void PreCloseFrame( CFrameWnd* pFrame );
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void Serialize(CArchive& ar);
	protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL



// Implementation
public:
	virtual ~CVisualSynanDoc();

	void BuildRels(CString& str)
	{
		m_VisualSentences.BuildRels(str);
	}

	BOOL GetHomonymsArray(CPoint& point,CPtrArray** pHomonymsArray, int* iActiveSentence,int* iActiveWord)
	{
		return m_VisualSentences.GetHomonymsArray(point,pHomonymsArray, iActiveSentence,iActiveWord);
	}

	BOOL SetActiveHomonym(int iActiveSentence,int iActiveWord, int iActiveHomonim)
	{
		return m_VisualSentences.SetActiveHomonym(iActiveSentence,iActiveWord, iActiveHomonim);
	}

	CSize GetSentencesSize()
	{
		return m_VisualSentences.GetSentencesSize();
	}

	BOOL GetActiveHomDescr(int m_iActiveSentenceTT,int m_iActiveWordTT,CString& strLemma,CString& strGramChar)
	{
		return m_VisualSentences.GetActiveHomDescr(m_iActiveSentenceTT,m_iActiveWordTT,strLemma,strGramChar);
	}

	BOOL CalculateCoordinates(CDC* pDC, int iWidth, BOOL bShowGroups)
	{
		return m_VisualSentences.CalculateCoordinates(pDC, iWidth, bShowGroups);
	}

	BOOL PrintSentences(CDC* pDC,CRect& rectForDrawing, int iOffset)
	{
		return m_VisualSentences.PrintSentences(pDC,rectForDrawing, iOffset);
	}

	BOOL NoSentences()
	{
		return (m_VisualSentences.SentCount() == 0);
	}

	void Reset()
	{
		m_VisualSentences.Reset();
	}


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:	
// Generated message  functions
protected:
	//{{AFX_MSG(CVisualSynanDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Helper function to convert text for internal storage
	std::string ConvertTextForStorage(const CString& text) {
		try {
			// Convert from wide string to UTF-8 then to Windows-1251
			std::string utf8Str = wstring_to_utf8(std::wstring(text));
			return convert_from_utf8(utf8Str.c_str(), morphRussian);
		}
		catch (const convert_exception& e) {
			OutputDebugString(_T("[VisualSynan] Text conversion error in storage: "));
			OutputDebugString(_U16(e.what()));
			OutputDebugString(_T("\n"));
			return std::string();
		}
	}

	// Helper function to convert text for display
	CString ConvertTextForDisplay(const std::string& text) {
		try {
			// Convert from Windows-1251 to UTF-8 then to wide string
			std::string utf8Str = convert_to_utf8(text, morphRussian);
			return utf8_to_wstring(utf8Str).c_str();
		}
		catch (const convert_exception& e) {
			OutputDebugString(_T("[VisualSynan] Text conversion error in display: "));
			OutputDebugString(_U16(e.what()));
			OutputDebugString(_T("\n"));
			return _T("Error displaying text");
		}
	}
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VISUALSYNANDOC_H__1C505B3F_C4B2_11D2_8BB6_00105A68ADF3__INCLUDED_)
