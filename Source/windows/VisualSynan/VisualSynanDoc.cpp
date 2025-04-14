// VisualSynanDoc.cpp : implementation of the CVisualSynanDoc class
//

#include "StdAfx.h"
#include "VisualSynan.h"
#include "MainFrm.h"
#include "VisualSynanDoc.h"
#include "ChildFrm.h"
#include "../../synan/SynanLib/SentencesCollection.h"

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
			// При сохранении формируем специальный формат для SYN файлов
			CString text;
			
			// Добавляем заголовок и служебную информацию
			text = _T("# Syntax analysis results by VisualSynan\r\n");
			
			// Добавляем время работы
			if (!m_WorkTimeStr.IsEmpty()) {
				text += _T("# Processing time: ") + m_WorkTimeStr + _T("\r\n");
			}
			
			// Счетчик предложений и заголовок
			int sentCount = m_VisualSentences.SentCount();
			CString sentCountStr;
			sentCountStr.Format(_T("# Number of sentences: %d\r\n"), sentCount);
			text += sentCountStr;
			
			// Специальный маркер начала данных для распознавания при открытии
			text += _T("# SYNAN_DATA_START\r\n");
			
			// Получаем исходные данные из приложения
			CVisualSynanApp* pApp = (CVisualSynanApp*)AfxGetApp();
			if (pApp) {
				try {
					// Сохраняем оригинальный текст для анализа
					const CSentencesCollection& synan = pApp->GetHolder().m_Synan;
					if (!synan.m_vectorSents.empty()) {
						text += _T("# Original text for analysis:\r\n");
						for (const auto& piSent : synan.m_vectorSents) {
							if (piSent) {
								text += _T("SENT: ");
								for (size_t i = 0; i < piSent->GetWords().size(); i++) {
									text += FromRMLEncode(piSent->m_Words[i].m_strWord.c_str());
									text += _T(" ");
								}
								text += _T("\r\n");
							}
						}
					}
					
					// Добавляем информацию о синтаксических отношениях
					CString relationsReport;
					m_VisualSentences.BuildRels(relationsReport);
					if (!relationsReport.IsEmpty()) {
						text += _T("# Syntactic Relations:\r\n");
						text += relationsReport;
					}
					
					// Специальный маркер конца данных
					text += _T("# SYNAN_DATA_END\r\n");
				}
				catch (...) {
					text += _T("# Error extracting syntax analysis details\r\n");
				}
			}
			
			// Записываем текст в архив
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
	try {
		((CMainFrame*)::AfxGetMainWnd())->m_bNewDoc = FALSE;
		CString strPath = lpszPathName;
		
		// Проверяем, является ли файл SYN-файлом
		CString strExt = strPath.Right(4);
		strExt.MakeLower();
		
		if (strExt == _T(".syn")) {
			// Специальная обработка для .SYN файлов
			CStdioFile file;
			if (!file.Open(lpszPathName, CFile::modeRead | CFile::typeText)) {
				AfxMessageBox(_T("Не удалось открыть файл"), MB_ICONERROR);
				return FALSE;
			}
			
			// Читаем содержимое файла
			CString fileContent, line;
			bool dataSection = false;
			bool foundData = false;
			
			while (file.ReadString(line)) {
				// Проверяем маркеры начала и конца данных
				if (line.Find(_T("# SYNAN_DATA_START")) != -1) {
					dataSection = true;
					continue;
				}
				else if (line.Find(_T("# SYNAN_DATA_END")) != -1) {
					dataSection = false;
					continue;
				}
				
				// Извлекаем данные предложений
				if (dataSection && line.Find(_T("SENT: ")) == 0) {
					// Убираем префикс "SENT: "
					line = line.Mid(6);
					fileContent += line + _T("\n");
					foundData = true;
				}
			}
			file.Close();
			
			// Если нашли текст для анализа, отправляем его на обработку
			if (foundData) {
				return GetSentencesFromSynAn(*this, fileContent, FALSE);
			} 
			else {
				// Если не нашли маркированные данные, пробуем обработать весь файл как текст
				if (!file.Open(lpszPathName, CFile::modeRead | CFile::typeText)) {
					return FALSE;
				}
				
				fileContent.Empty();
				while (file.ReadString(line)) {
					// Пропускаем строки комментариев
					if (!line.IsEmpty() && line[0] != '#') {
						fileContent += line + _T("\n");
					}
				}
				file.Close();
				
				if (!fileContent.IsEmpty()) {
					return GetSentencesFromSynAn(*this, fileContent, FALSE);
				}
				else {
					AfxMessageBox(_T("Файл не содержит данных для анализа"), MB_ICONINFORMATION);
					return FALSE;
				}
			}
		} 
		else {
			// Стандартная обработка для других файлов
			return GetSentencesFromSynAn(*this, strPath, TRUE);
		}
	}
	catch (CFileException* e) {
		e->ReportError();
		e->Delete();
		return FALSE;
	}
	catch (...) {
		AfxMessageBox(_T("Произошла ошибка при открытии документа"), MB_ICONERROR);
		return FALSE;
	}
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
