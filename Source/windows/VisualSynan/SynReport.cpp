#include "SynReport.h"


bool GlobalOpenReport(CString S, CString Name)
{
	CDocTemplate* T = GetReportTemplate();
	if (T == nullptr) {
		// Template not found - already reported in GetTemplate
		return false;
	}
	
	CReportDoc* pDocument = (CReportDoc*)T->CreateNewDocument();
	if (pDocument == nullptr) {
		AfxMessageBox(_T("Failed to create report document"), MB_ICONERROR);
		return false;
	}
    
	CFrameWnd* pFrame = T->CreateNewFrame(pDocument, NULL);
	if (pFrame == nullptr) {
		AfxMessageBox(_T("Failed to create frame for report"), MB_ICONERROR);
		delete pDocument; // Clean up
		return false;
	}
	
	T->InitialUpdateFrame(pFrame, pDocument, TRUE);
	pDocument->m_bRTF = false;
	CRichEditCtrl& C = pDocument->GetView()->GetRichEditCtrl();
	pDocument->SetPathName(Name, FALSE);
		
    C.SetWindowText(S);
	//pDocument->InitFonts();
	return true;
};

