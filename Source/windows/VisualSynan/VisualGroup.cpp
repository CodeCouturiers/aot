#include "StdAfx.h"
#include "VisualSentences.h"
#include "VisualSynanView.h"


CVisualGroup::CVisualGroup()
{
	m_pointLeft.x = 0;
	m_pointLeft.y = 0;
	m_pointRight.x = 0;
	m_pointRight.y = 0;
	m_iFirstWord = 0;
	m_iLastWord = 0;
	m_iLevel = 0;
	m_iSpaceBetweenLines = 70;
	m_bClause = FALSE;
}


BOOL CVisualGroup::Init(const CClause& clause, const CGroup& piGroup)
{
	
	m_iFirstWord = piGroup.m_iFirstWord;
	m_iLastWord = piGroup.m_iLastWord;
	m_strDescription = FromRMLEncode(clause.GetOpt()->GetGroupNameByIndex(piGroup.m_GroupType));
	m_strDescription.MakeLower();
	return TRUE;
}



BOOL CVisualGroup::CalculateCoordinates(CDC* pDC, CPoint& pointLeftLeg, CPoint& poitRightLeg, int top, int iWidth, BOOL bOnDifferentLines, COLORREF Color, int iLine)
{
	
	ResetParts();

	CPartOfGrArc* pPartOfGrArc;
	pPartOfGrArc = NewGroupArcPart(Color, iLine, 
									pointLeftLeg, 
									CPoint(pointLeftLeg.x, top + RADIUS_ANGLE /*- 2*/),
									Line, CRect(0,0,0,0),FALSE, FALSE);

	m_vectorParts.push_back(pPartOfGrArc);	

	pPartOfGrArc = NewGroupArcPart(Color, iLine, 
									CPoint(pointLeftLeg.x + RADIUS_ANGLE, top), 
									CPoint(pointLeftLeg.x, top + RADIUS_ANGLE + 2),
									ArcFig, CRect(pointLeftLeg.x, top , pointLeftLeg.x + 2*RADIUS_ANGLE - 2, top + 2*RADIUS_ANGLE),FALSE, FALSE);

	m_vectorParts.push_back(pPartOfGrArc);	



	CPoint _Point;

	//if groups are on different lines
	if( bOnDifferentLines)
	{


		pPartOfGrArc = NewGroupArcPart(Color, iLine, 
										CPoint(pointLeftLeg.x + RADIUS_ANGLE, top), 
										CPoint(iWidth, top),
										Line, CRect(0,0,0,0),FALSE, FALSE);

		m_vectorParts.push_back(pPartOfGrArc);

		iLine++;

		int k = 1;
		
		while( poitRightLeg.y - top > (k+1)*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE) )
		{

			pPartOfGrArc = NewGroupArcPart(Color, iLine, 
											CPoint(0,top + k*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE) ),
											CPoint(iWidth, top + k*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE) ),
											Line, CRect(0,0,0,0) ,TRUE, FALSE);
			
			m_vectorParts.push_back(pPartOfGrArc);		
			k++;
		}

		pPartOfGrArc = NewGroupArcPart(Color, iLine, 
										CPoint(0,top + k*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE) ),
										CPoint(poitRightLeg.x - RADIUS_ANGLE + 1, top + k*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE) ),
										Line, CRect(0,0,0,0) ,TRUE, FALSE);
		
		m_vectorParts.push_back(pPartOfGrArc);		


		pPartOfGrArc = NewGroupArcPart(Color, iLine, 										
										CPoint(poitRightLeg.x , top + k*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE) + RADIUS_ANGLE),
										CPoint(poitRightLeg.x - RADIUS_ANGLE + 1,top + k*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE) - 1),
										ArcFig,CRect(poitRightLeg.x - 2*RADIUS_ANGLE, top + k*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE), poitRightLeg.x + 1, top + 2*RADIUS_ANGLE + k*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE) ) ,FALSE, FALSE);

		m_vectorParts.push_back(pPartOfGrArc);		



		_Point.x = poitRightLeg.x;
		_Point.y = top + k*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE) +  RADIUS_ANGLE;

	}
	else
	{
		pPartOfGrArc = NewGroupArcPart(Color, iLine, 
										CPoint(pointLeftLeg.x + RADIUS_ANGLE, top),
										CPoint(poitRightLeg.x - RADIUS_ANGLE + 1, top),
										Line, CRect(0,0,0,0),TRUE, TRUE);

		m_vectorParts.push_back(pPartOfGrArc);		

		pPartOfGrArc = NewGroupArcPart(Color, iLine, 
										CPoint(poitRightLeg.x +1, top + RADIUS_ANGLE),
										CPoint(poitRightLeg.x - RADIUS_ANGLE + 1,top ),
										ArcFig,CRect(poitRightLeg.x - 2*RADIUS_ANGLE + 3, top, poitRightLeg.x + 1, top + 2*RADIUS_ANGLE ) ,FALSE, TRUE);

		m_vectorParts.push_back(pPartOfGrArc);		



		_Point.x = poitRightLeg.x;
		_Point.y = top +  RADIUS_ANGLE;		
	}

	pPartOfGrArc = NewGroupArcPart(Color, iLine, 
									_Point, 
									poitRightLeg,
									Line, CRect(0,0,0,0),FALSE, FALSE);

	m_vectorParts.push_back(pPartOfGrArc);		

	m_pointLeft.x = pointLeftLeg.x;
	m_pointLeft.y = top;
	m_pointRight.x = poitRightLeg.x;

	if( bOnDifferentLines )
		m_pointRight.y = top + m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE;
	else
		m_pointRight.y = top;
	return TRUE;
}

void CVisualGroup::PrintGroupPart(CDC* pDC, int i, int iOffset)
{
	if( (i < 0) || (i >= m_vectorParts.size()) )
		return;

	CPartOfGrArc* pPart = reinterpret_cast<CPartOfGrArc*>(m_vectorParts[i]);
	ASSERT( pPart != NULL );

	// Настраиваем качество отрисовки
	int oldMode = pDC->SetBkMode(TRANSPARENT);
	pDC->SetROP2(R2_COPYPEN);

	// Создаем градиентную кисть для заливки
	COLORREF baseColor = pPart->m_Color;
	COLORREF lightColor = RGB(
		min(255, GetRValue(baseColor) + 40),
		min(255, GetGValue(baseColor) + 40),
		min(255, GetBValue(baseColor) + 40)
	);

	// Толщина линии зависит от типа (клауза или группа)
	int penWidth = m_bClause ? 3 : 2;
	
	// Создаем перо с закругленными концами для более плавного вида
	LOGBRUSH lb;
	lb.lbStyle = BS_SOLID;
	lb.lbColor = baseColor;
	lb.lbHatch = 0;
	
	CPen pen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_ROUND, 
			 penWidth, &lb);
	CPen* pOldPen = pDC->SelectObject(&pen);

	CPoint pointStart(pPart->m_pointStart.x, pPart->m_pointStart.y - iOffset);
	CPoint pointEnd(pPart->m_pointEnd.x, pPart->m_pointEnd.y - iOffset);
	CRect rectForArc(pPart->m_RectForArc);
	rectForArc.bottom -= iOffset;
	rectForArc.top -= iOffset;

	if(pPart->type == ArcFig)
	{
		// Рисуем дугу с плавным переходом
		pDC->Arc(rectForArc, pointStart, pointEnd);
		
		// Добавляем небольшой блик для объема
		CPen lightPen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND, 1, lightColor);
		pDC->SelectObject(&lightPen);
		rectForArc.DeflateRect(1, 1);
		pDC->Arc(rectForArc, pointStart, pointEnd);
	}
	else if(pPart->type == Line)
	{
		// Рисуем основную линию
		pDC->MoveTo(pointStart);
		pDC->LineTo(pointEnd);
		
		// Добавляем тонкую подсветку сверху для объема
		CPen lightPen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND, 1, lightColor);
		pDC->SelectObject(&lightPen);
		pointStart.y -= 1;
		pointEnd.y -= 1;
		pDC->MoveTo(pointStart);
		pDC->LineTo(pointEnd);
	}

	// Восстанавливаем настройки DC
	pDC->SelectObject(pOldPen);
	pDC->SetROP2(oldMode);

	// Улучшенная отрисовка текста описания
	if(pPart->m_bHasDescription)
	{
		CFont* pOldFont = pDC->GetCurrentFont(); 
		pDC->SelectObject(&(CVisualSynanView::m_FontForGroupNames));

		TEXTMETRIC txtM;
		pDC->GetTextMetrics(&txtM);

		// Создаем эффект тени для текста
		COLORREF oldTextColor = pDC->SetTextColor(RGB(128, 128, 128));
		
		CString& text = m_strDescription;
		int textX, textY;
		
		if(!pPart->m_bWholeArc)
		{
			textX = 6;
			textY = pPart->m_pointStart.y - txtM.tmHeight - 4 - iOffset + 1;
		}
		else
		{
			textX = m_pointLeft.x + (m_pointRight.x - m_pointLeft.x)/2 + 2;
			textY = pPart->m_pointStart.y - txtM.tmHeight - 4 - iOffset + 1;
		}
		
		// Рисуем тень
		pDC->TextOut(textX + 1, textY + 1, text, text.GetLength());
		
		// Рисуем основной текст
		pDC->SetTextColor(baseColor);
		pDC->TextOut(textX, textY, text, text.GetLength());

		// Восстанавливаем настройки
		pDC->SetTextColor(oldTextColor);
		pDC->SelectObject(pOldFont);
	}
}




CPartOfGrArc* NewGroupArcPart(long color, int line, CPoint start, CPoint end,ETypeFigure type, CRect rectForArc, BOOL has_descr, BOOL whole_arc)
{
	CPartOfGrArc* pPartOfGrArc = new CPartOfGrArc;

	pPartOfGrArc->m_Color = color;
	pPartOfGrArc->m_iLine = line;
	pPartOfGrArc->m_pointStart = start;
	pPartOfGrArc->type = type;
	pPartOfGrArc->m_pointEnd = end;
	pPartOfGrArc->m_RectForArc = rectForArc;
	pPartOfGrArc->m_bHasDescription = has_descr;
	pPartOfGrArc->m_bWholeArc = whole_arc;

	return pPartOfGrArc;
}


