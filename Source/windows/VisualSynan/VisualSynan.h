// VisualSynan.h : main header file for the VISUALSYNAN application
//

#pragma once

#include "Resource.h"       
#include "synan/SynanLib/SyntaxHolder.h"

class CVisualSynanApp : public CWinApp
{
	DECLARE_DYNCREATE(CVisualSynanApp)
public:
	CVisualSynanApp() {} // Add default constructor

	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVisualSynanApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	void SetLanguage(MorphLanguageEnum l);
	CSyntaxHolder& GetHolder();
	CDocTemplate* GetSynTemplate() { return m_pSynTemplate; }

protected:
	CDocTemplate* m_pSynTemplate;

// Implementation
	//{{AFX_MSG(CVisualSynanApp)
	afx_msg void OnAppAbout();
	afx_msg void OnSynFileNew();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

