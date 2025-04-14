#if !defined(AFX_ROSSDEV_H__553CC260_C720_11D2_A6E4_A290D9000000__INCLUDED_)
#define AFX_ROSSDEV_H__553CC260_C720_11D2_A6E4_A290D9000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdAfx.h"
#include "ReportDoc.h"

inline  CDocTemplate* GetTemplate (CString Name)
{
	POSITION pos = AfxGetApp()->GetFirstDocTemplatePosition();
	CDocTemplate* tmpl = nullptr;
	CString S;
	
	// Loop through all templates until we find the one we need
	while (pos != NULL) {
		tmpl = AfxGetApp()->GetNextDocTemplate(pos);
		if (tmpl == nullptr) {
			break;
		}
		
		tmpl->GetDocString(S, CDocTemplate::regFileTypeId);
		if (S == Name) {
			break;  // Found the template
		}
	}
	
	// Check if template was actually found
	if (tmpl == nullptr || S != Name) {
		CString errMsg;
		errMsg.Format(_T("Template '%s' not found"), Name);
		AfxMessageBox(errMsg, MB_ICONERROR);
		return nullptr;
	}

	return tmpl;
};

inline  CDocTemplate* GetReportTemplate ()
{
	return GetTemplate("VisualSynan.Report");
};

inline  CDocTemplate* GetSynTemplate ()
{
	return GetTemplate("VisualSynan.Document");
};


bool GlobalOpenReport(CString S, CString Name);

#endif