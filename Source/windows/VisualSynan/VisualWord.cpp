#include "StdAfx.h"
#include "VisualSentences.h"
#include "VisualSynanView.h"



CVisualWord::CVisualWord()
{
	m_WordRect.SetRectEmpty();
	m_iActiveHomonym = 0;
	m_bBold = FALSE;
	m_bInTermin = FALSE;
	m_bArtificialCreated = FALSE;
	m_ReferenceWordNo = -1;
}

CVisualWord::~CVisualWord()
{
	for(int i = 0 ; i < m_arrHomonyms.GetSize(); i++ )
	{
		CVisualHomonym* pHom;
		pHom = (CVisualHomonym*)m_arrHomonyms.GetAt(i);
		//delete[] pHom->m_arrGroupIDs;
		delete pHom;
	}
}

BOOL CVisualWord::Init(const CSynWord& piWord, const CSentence& piSentence)
{
	try
	{
		m_strWord = FromRMLEncode(piWord.m_strWord.c_str());
		if (piWord.m_bDeleted)
			m_strWord = "[" + m_strWord + "]";
		m_bInTermin = piWord.m_bInTermin;
		m_bArtificialCreated = piWord.m_bArtificialCreated;
		long lHomCount;

		m_iClauseNo = piWord.m_iClauseNo;
		//вытаскиваем все леммы и грамемы
		lHomCount = piWord.GetHomonymsCount();
		for(int i = 0 ; i < lHomCount ; i++) 
		{
			const CSynHomonym& piHomonym = piWord.GetSynHomonym(i);

			CVisualHomonym* pHomonym = new CVisualHomonym;
			pHomonym->m_strLemma = FromRMLEncode(piHomonym.GetLemma());
			pHomonym->m_strCommonGrammems = FromRMLEncode(piSentence.GetOpt()->GetGramTab()->GrammemsToStr(piHomonym.m_TypeGrammems));
			pHomonym->m_strPOS = FromRMLEncode(piHomonym.GetPartOfSpeechStr());



            uint32_t paradigmID = piHomonym.m_lPradigmID;
			pHomonym->m_strSomeDescr = "";
			if( piHomonym.m_bOborot1 )
			{
				pHomonym->m_strSomeDescr += " <Ob1>";			
				int OborotId = piHomonym.m_OborotNo;
				assert (OborotId != -1);
				if (OborotId != -1)
				{
					CString str;
					str.Format(_T("Ob: %s"), FromRMLEncode(piHomonym.GetOborotPtr()->m_OborotEntryStr));
					pHomonym->m_strOborotsNum += str; 
				};
			}

			if (piHomonym.IsOb1()) {
				pHomonym->m_strSomeDescr += " <InOb>";
			}

			if( piHomonym.IsOb2() )
					pHomonym->m_strSomeDescr += " <Ob2>";

			
			if( piHomonym.m_SimplePrepNos.size() > 0 )
			{
				pHomonym->m_strSomeDescr = " (All preps:";			
				for(int k = 0 ; k < piHomonym.m_SimplePrepNos.size(); k++ )
				{
					CString str = ": ";
					str += FromRMLEncode(piSentence.GetOpt()->GetOborDic()->m_Entries[k].m_OborotEntryStr);
					pHomonym->m_strSomeDescr += str; 
				}
				pHomonym->m_strSomeDescr += ")"; 
			}



			pHomonym->m_iPradigmID = paradigmID;

			m_arrHomonyms.Add(pHomonym);
		}
		
		if( m_arrHomonyms.GetSize() > 1)
			m_bBold = TRUE;
		else
			m_bBold = FALSE;

		m_MainVerbs.clear();
		for (auto& w: piWord.m_MainVerbs)
			m_MainVerbs.push_back(w);

	}
	catch(...)
	{
		return FALSE;
	}

	return TRUE;
}

int CVisualWord::GetWordLen(CDC* pDC)
{
	CSize sizeWord;
	sizeWord = pDC->GetOutputTextExtent(m_strWord);
	return sizeWord.cx;
}

int CVisualWord::GetWordHight(CDC* pDC)
{
	CSize sizeWord;
	sizeWord = pDC->GetOutputTextExtent(m_strWord);
	return sizeWord.cy;
}





BOOL CVisualWord::PrintWord(CDC* pDC, int iOffset)
{
    CFont* pOldFont = NULL;    
    COLORREF old_color = pDC->GetTextColor();

    // Получаем информацию о том, является ли слово подлежащим или сказуемым
    BOOL bSubj = ((CVisualHomonym*)m_arrHomonyms.GetAt(m_iActiveHomonym))->m_bSubj;
    BOOL bPredk = ((CVisualHomonym*)m_arrHomonyms.GetAt(m_iActiveHomonym))->m_bPredk;
    BOOL bMainPart = bSubj || bPredk;
    
    // Основные цвета для различных типов слов
    COLORREF termColor = RGB(26, 188, 156);  // Бирюзовый для терминов
    COLORREF artificialColor = RGB(39, 174, 96);  // Изумрудный для искусственных слов
    
    // Цвета для подлежащих и сказуемых
    COLORREF subjColor = RGB(52, 152, 219);  // Яркий голубой (Peter River)
    COLORREF predkColor = RGB(155, 89, 182);  // Фиолетовый (Amethyst)
    
    // Цвета для эффектов
    COLORREF shadowColor = RGB(210, 210, 210);
    COLORREF highlightColor = RGB(250, 250, 210);  // Светло-желтый для подсветки
    
    // Определяем цвет текста в зависимости от типа
    COLORREF textColor;
    if (bSubj) {
        textColor = subjColor;
    } 
    else if (bPredk) {
        textColor = predkColor;
    }
    else if (m_bInTermin) {
        textColor = termColor;
    }
    else if (m_bArtificialCreated) {
        textColor = artificialColor;
    }
    else {
        textColor = RGB(0, 0, 0);  // Обычный черный для остальных слов
    }

    // Устанавливаем цвет текста
    pDC->SetTextColor(textColor);
    
    // Прозрачный фон для лучшего вида
    int oldMode = pDC->SetBkMode(TRANSPARENT);

    // Выбор шрифта в зависимости от характеристик слова
    if (bMainPart) {
        pOldFont = pDC->SelectObject(m_bBold || m_bArtificialCreated ? 
            &CVisualSynanView::m_BoldUnderlineFontForWords :
            &CVisualSynanView::m_UnderlineFontForWords);
    }
    else if (m_bBold || m_bArtificialCreated) {
        pOldFont = pDC->SelectObject(&CVisualSynanView::m_BoldFontForWords);
    }

    // Рисуем фон для главных членов предложения
    if (bMainPart) {
        // Создаем полупрозрачную заливку для подсветки главных членов
        CBrush highlightBrush;
        COLORREF highlightBgColor;
        
        if (bSubj) {
            // Мягкий голубой фон для подлежащих
            highlightBgColor = RGB(235, 245, 255);
        } else {
            // Мягкий фиолетовый фон для сказуемых
            highlightBgColor = RGB(245, 235, 255);
        }
        
        highlightBrush.CreateSolidBrush(highlightBgColor);
        
        // Рисуем закругленный прямоугольный фон
        CRect bgRect = m_WordRect;
        bgRect.OffsetRect(0, -iOffset);
        bgRect.InflateRect(4, 2);
        
        // Создаем закругленные углы
        int oldBkMode = pDC->GetBkMode();
        pDC->SetBkMode(OPAQUE);
        pDC->SelectStockObject(NULL_PEN);
        CBrush* pOldBrush = pDC->SelectObject(&highlightBrush);
        
        // Рисуем закругленный прямоугольник
        pDC->RoundRect(bgRect, CPoint(8, 8));
        
        // Восстанавливаем объекты
        pDC->SelectObject(pOldBrush);
        pDC->SetBkMode(oldBkMode);
        highlightBrush.DeleteObject();
    }

    // Специальное подчеркивание и декорации для главных членов предложения
    if (bMainPart) {
        // Выбираем цвет и стиль линии в зависимости от типа члена предложения
        COLORREF lineColor = bSubj ? subjColor : predkColor;
        
        // Создаем перо с закругленными концами
        LOGBRUSH lb;
        lb.lbStyle = BS_SOLID;
        lb.lbColor = lineColor;
        lb.lbHatch = 0;
        
        // Толстая линия для выделения
        CPen mainPen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND, 2, &lb);
        CPen* pOldPen = pDC->SelectObject(&mainPen);
        
        // Рисуем основную линию
        int underlineY = m_WordRect.bottom + 2 - iOffset;
        pDC->MoveTo(m_WordRect.left - 2, underlineY);
        pDC->LineTo(m_WordRect.right + 2, underlineY);
        
        // Добавляем декоративную линию
        if (bSubj) {
            // Двойная линия для подлежащего
            pDC->MoveTo(m_WordRect.left, underlineY + 3);
            pDC->LineTo(m_WordRect.right, underlineY + 3);
        } else {
            // Волнистая линия для сказуемого (имитация волны через точки)
            int waveAmp = 2;
            int wavePeriod = 6;
            for (int x = m_WordRect.left; x < m_WordRect.right; x += 2) {
                int phase = ((x - m_WordRect.left) % wavePeriod) * 180 / wavePeriod;
                int yOffset = static_cast<int>(waveAmp * sin(phase * 3.14159 / 180));
                pDC->SetPixel(x, underlineY + 3 + yOffset, lineColor);
                pDC->SetPixel(x + 1, underlineY + 3 + yOffset, lineColor);
            }
        }
        
        pDC->SelectObject(pOldPen);
    }

    // Рисуем сам текст с эффектом тени/свечения для главных членов предложения
    if (bMainPart || m_bBold) {
        // Для главных членов - светящаяся тень
        if (bMainPart) {
            // Более яркая тень для главных членов
            COLORREF glowColor = bSubj ? 
                RGB(200, 230, 250) : // Голубоватая для подлежащего
                RGB(230, 200, 250);  // Фиолетовая для сказуемого
                
            // Эффект свечения через многослойную тень
            for (int i = 1; i <= 2; i++) {
                pDC->SetTextColor(glowColor);
                pDC->TextOut(m_WordRect.left - i, m_WordRect.top - iOffset, m_strWord, m_strWord.GetLength());
                pDC->TextOut(m_WordRect.left + i, m_WordRect.top - iOffset, m_strWord, m_strWord.GetLength());
                pDC->TextOut(m_WordRect.left, m_WordRect.top - iOffset - i, m_strWord, m_strWord.GetLength());
                pDC->TextOut(m_WordRect.left, m_WordRect.top - iOffset + i, m_strWord, m_strWord.GetLength());
            }
        } else {
            // Обычная тень для жирного текста
            pDC->SetTextColor(shadowColor);
            pDC->TextOut(m_WordRect.left + 1, m_WordRect.top - iOffset + 1, m_strWord, m_strWord.GetLength());
        }
        
        // Основной текст
        pDC->SetTextColor(textColor);
        pDC->TextOut(m_WordRect.left, m_WordRect.top - iOffset, m_strWord, m_strWord.GetLength());
    } else {
        // Для обычного текста - просто рисуем
        pDC->TextOut(m_WordRect.left, m_WordRect.top - iOffset, m_strWord, m_strWord.GetLength());
    }

    // Восстанавливаем контекст устройства
    if (pOldFont) {
        pDC->SelectObject(pOldFont);
    }
    pDC->SetTextColor(old_color);
    pDC->SetBkMode(oldMode);

    return TRUE;
}

int CVisualWord::CalculateCoordinates(CDC* pDC,int iX, int iY)
{
	CSize sizeWord;

	CSize rectWithBoldFont;
	CFont* pOldFont;

	if( m_arrHomonyms.GetSize() > 1 )
	{

		pOldFont = pDC->GetCurrentFont(); 
		pDC->SelectObject(&CVisualSynanView::m_BoldFontForWords);

		rectWithBoldFont = pDC->GetOutputTextExtent(m_strWord);
		pDC->SelectObject(pOldFont);
		sizeWord = pDC->GetOutputTextExtent(m_strWord);
		sizeWord.cx = rectWithBoldFont.cx;
		pDC->SelectObject(pOldFont);
	}
	else
		sizeWord = pDC->GetOutputTextExtent(m_strWord);
	
	m_WordRect.SetRect(iX,iY,sizeWord.cx + iX, sizeWord.cy + iY);

	return iX + sizeWord.cx;
}

int CVisualWord::PrintWord(CDC* pDC,int iX, int iY)
{
	CSize sizeWord;

	CFont* pOldFont;
	if( m_arrHomonyms.GetSize() > 1)
	{
		pOldFont = pDC->GetCurrentFont(); 
		pDC->SelectObject(& (CVisualSynanView::m_BoldFontForWords) );
	}

	pDC->TextOut(iX,iY,m_strWord,m_strWord.GetLength());

	CSize rectWithBoldFont;
	if( m_arrHomonyms.GetSize() > 1 )
	{
		rectWithBoldFont = pDC->GetOutputTextExtent(m_strWord);
		pDC->SelectObject(pOldFont);
		sizeWord = pDC->GetOutputTextExtent(m_strWord);
		sizeWord.cx = rectWithBoldFont.cx;
	}
	else
		sizeWord = pDC->GetOutputTextExtent(m_strWord);

	m_WordRect.SetRect(iX,iY,sizeWord.cx + iX, sizeWord.cy + iY);


	return iX + sizeWord.cx;
}


BOOL CVisualWord::PointInWordRect(CPoint& point)
{
	return m_WordRect.PtInRect(point);
}


void CVisualWord::GetHomonymsArray(CPtrArray** pHomonymsArray)
{
	if(pHomonymsArray != NULL) 
		(*pHomonymsArray) = &m_arrHomonyms;
}

BOOL CVisualWord::SetActiveHomonym(int iActiveHomonim)
{
	if( (iActiveHomonim < 0) || (iActiveHomonim >= m_arrHomonyms.GetSize()) )
		return FALSE;

	if(m_iActiveHomonym == iActiveHomonim)
		return FALSE;
	
	m_iActiveHomonym = iActiveHomonim;
	return TRUE;
}


void CVisualWord::ResetSubjAndPred()
{
	for(int i = 0 ; i < m_arrHomonyms.GetSize() ; i++ )
	{
		((CVisualHomonym*)m_arrHomonyms.GetAt(i))->m_bSubj = FALSE;
		((CVisualHomonym*)m_arrHomonyms.GetAt(i))->m_bPredk = FALSE;
	}

}

BOOL CVisualWord::GetActiveHomDescr(CString& strLemma,CString& strGramChar)
{
	strLemma = ((CVisualHomonym*)m_arrHomonyms.GetAt(m_iActiveHomonym))->m_strLemma; 
	
	strGramChar = m_strActiveGrammems;
	strGramChar = ((CVisualHomonym*)m_arrHomonyms.GetAt(m_iActiveHomonym))->m_strCommonGrammems + " "+strGramChar;
	strGramChar = ((CVisualHomonym*)m_arrHomonyms.GetAt(m_iActiveHomonym))->m_strPOS + " " + strGramChar;
	int ii = ((CVisualHomonym*)m_arrHomonyms.GetAt(m_iActiveHomonym))->m_iPradigmID;
	CString ss;
	ss.Format(_T("%d"), ii);
	strGramChar = strGramChar + " " + ss;
	strGramChar += ((CVisualHomonym*)m_arrHomonyms.GetAt(m_iActiveHomonym))->m_strSomeDescr;

	ss.Empty();
	ss.Format(_T(" (ClauseNo %d)"), m_iClauseNo);
	strGramChar += ss;

	strGramChar += m_strSomeDescr;
	return TRUE;
}




