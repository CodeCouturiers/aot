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


BOOL CVisualGroup::CalculateCoordinates(CDC* pDC, CPoint& pointLeftLeg, CPoint& pointRightLeg, 
                                         int top, int iWidth, BOOL bOnDifferentLines, 
                                         COLORREF Color, int iLine)
{
	ResetParts();

	// Улучшенные размеры дуг для более эстетичного вида
	const int arcRadius = RADIUS_ANGLE + 2;  // Увеличиваем радиус для более плавных дуг
	const int lineThickness = m_bClause ? 3 : 2;  // Толщина зависит от типа группы
	
	CPartOfGrArc* pPartOfGrArc;
	
	// Вертикальная линия от слова до начала дуги (левая сторона)
	pPartOfGrArc = NewGroupArcPart(Color, iLine, 
	                               pointLeftLeg, 
	                               CPoint(pointLeftLeg.x, top + arcRadius),
	                               Line, CRect(0,0,0,0), FALSE, FALSE);
	m_vectorParts.push_back(pPartOfGrArc);
	
	// Дуга в левом верхнем углу (закругление)
	pPartOfGrArc = NewGroupArcPart(Color, iLine, 
	                               CPoint(pointLeftLeg.x + arcRadius, top), 
	                               CPoint(pointLeftLeg.x, top + arcRadius),
	                               ArcFig, 
	                               CRect(pointLeftLeg.x, top, pointLeftLeg.x + 2*arcRadius, top + 2*arcRadius),
	                               FALSE, FALSE);
	m_vectorParts.push_back(pPartOfGrArc);

	CPoint _Point;

	// Обработка для групп на разных строках (мультилинейные группы)
	if(bOnDifferentLines)
	{
		// Горизонтальная линия от левой дуги до правого края экрана
		pPartOfGrArc = NewGroupArcPart(Color, iLine, 
		                               CPoint(pointLeftLeg.x + arcRadius, top), 
		                               CPoint(iWidth, top),
		                               Line, CRect(0,0,0,0), FALSE, FALSE);
		m_vectorParts.push_back(pPartOfGrArc);

		iLine++;
		int k = 1;
		
		// Вертикальные линии для мультистрочных групп
		while(pointRightLeg.y - top > (k+1)*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE))
		{
			int lineY = top + k*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE);
			
			// Горизонтальная линия между строками
			pPartOfGrArc = NewGroupArcPart(Color, iLine, 
			                               CPoint(0, lineY),
			                               CPoint(iWidth, lineY),
			                               Line, CRect(0,0,0,0), TRUE, FALSE);
			m_vectorParts.push_back(pPartOfGrArc);
			k++;
		}

		int finalLineY = top + k*(m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE);
		
		// Горизонтальная линия от левого края до правой дуги
		pPartOfGrArc = NewGroupArcPart(Color, iLine, 
		                               CPoint(0, finalLineY),
		                               CPoint(pointRightLeg.x - arcRadius, finalLineY),
		                               Line, CRect(0,0,0,0), TRUE, FALSE);
		m_vectorParts.push_back(pPartOfGrArc);

		// Правая дуга (закругление)
		pPartOfGrArc = NewGroupArcPart(Color, iLine,
		                               CPoint(pointRightLeg.x, finalLineY + arcRadius),
		                               CPoint(pointRightLeg.x - arcRadius, finalLineY),
		                               ArcFig,
		                               CRect(pointRightLeg.x - 2*arcRadius, finalLineY, 
		                                     pointRightLeg.x, finalLineY + 2*arcRadius),
		                               FALSE, FALSE);
		m_vectorParts.push_back(pPartOfGrArc);

		_Point.x = pointRightLeg.x;
		_Point.y = finalLineY + arcRadius;
	}
	else // Группа на одной строке
	{
		// Горизонтальная линия между дугами
		pPartOfGrArc = NewGroupArcPart(Color, iLine, 
		                               CPoint(pointLeftLeg.x + arcRadius, top),
		                               CPoint(pointRightLeg.x - arcRadius, top),
		                               Line, CRect(0,0,0,0), TRUE, TRUE);
		m_vectorParts.push_back(pPartOfGrArc);

		// Правая дуга (закругление)
		pPartOfGrArc = NewGroupArcPart(Color, iLine, 
		                               CPoint(pointRightLeg.x, top + arcRadius),
		                               CPoint(pointRightLeg.x - arcRadius, top),
		                               ArcFig,
		                               CRect(pointRightLeg.x - 2*arcRadius, top, 
		                                     pointRightLeg.x, top + 2*arcRadius),
		                               FALSE, TRUE);
		m_vectorParts.push_back(pPartOfGrArc);

		_Point.x = pointRightLeg.x;
		_Point.y = top + arcRadius;
	}

	// Вертикальная линия от дуги до слова (правая сторона)
	pPartOfGrArc = NewGroupArcPart(Color, iLine, 
	                               _Point, 
	                               pointRightLeg,
	                               Line, CRect(0,0,0,0), FALSE, FALSE);
	m_vectorParts.push_back(pPartOfGrArc);

	// Сохраняем координаты для использования в других функциях
	m_pointLeft.x = pointLeftLeg.x;
	m_pointLeft.y = top;
	m_pointRight.x = pointRightLeg.x;

	if(bOnDifferentLines)
		m_pointRight.y = top + m_iSpaceBetweenLines + SPACE_BETWEEN_SENTENCE;
	else
		m_pointRight.y = top;
		
	return TRUE;
}

void CVisualGroup::PrintGroupPart(CDC* pDC, int i, int iOffset)
{
	if((i < 0) || (i >= m_vectorParts.size()))
		return;

	CPartOfGrArc* pPart = reinterpret_cast<CPartOfGrArc*>(m_vectorParts[i]);
	ASSERT(pPart != NULL);

	// Настраиваем DC для высококачественной отрисовки
	int oldMode = pDC->SetBkMode(TRANSPARENT);
	pDC->SetROP2(R2_COPYPEN);
	
	// Получаем базовый цвет 
	COLORREF baseColor = pPart->m_Color;
	
	// Определяем яркость для выбора цвета текста
	int brightness = (GetRValue(baseColor) + GetGValue(baseColor) + GetBValue(baseColor)) / 3;
	
	// Создаем светлый оттенок базового цвета для эффекта объема
	COLORREF lightColor = RGB(
		min(255, GetRValue(baseColor) + 50),
		min(255, GetGValue(baseColor) + 50),
		min(255, GetBValue(baseColor) + 50)
	);
	
	// Создаем темный оттенок для тени
	COLORREF darkColor = RGB(
		max(0, GetRValue(baseColor) - 40),
		max(0, GetGValue(baseColor) - 40),
		max(0, GetBValue(baseColor) - 40)
	);

	// Увеличиваем толщину линии для лучшей видимости
	int penWidth = m_bClause ? 3 : 2;
	
	// Создаем перо для основной линии
	LOGBRUSH lb;
	lb.lbStyle = BS_SOLID;
	lb.lbColor = baseColor;
	lb.lbHatch = 0;
	
	// Используем закругленные концы линий для более элегантного вида
	CPen pen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_ROUND, 
	         penWidth, &lb);
	CPen* pOldPen = pDC->SelectObject(&pen);

	// Готовим координаты с учетом прокрутки
	CPoint pointStart(pPart->m_pointStart.x, pPart->m_pointStart.y - iOffset);
	CPoint pointEnd(pPart->m_pointEnd.x, pPart->m_pointEnd.y - iOffset);
	CRect rectForArc(pPart->m_RectForArc);
	rectForArc.bottom -= iOffset;
	rectForArc.top -= iOffset;

	// Рисуем в зависимости от типа элемента
	if(pPart->type == ArcFig)
	{
		// Сначала рисуем тень для основной дуги для эффекта объема
		LOGBRUSH lbShadow;
		lbShadow.lbStyle = BS_SOLID;
		lbShadow.lbColor = darkColor;
		lbShadow.lbHatch = 0;
		
		CPen shadowPen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND, penWidth, &lbShadow);
		pDC->SelectObject(&shadowPen);
		
		CRect shadowRect = rectForArc;
		shadowRect.OffsetRect(1, 1);
		pDC->Arc(shadowRect, pointStart, pointEnd);
		
		// Рисуем основную дугу
		pDC->SelectObject(&pen);
		pDC->Arc(rectForArc, pointStart, pointEnd);
		
		// Добавляем световой блик сверху для эффекта объема
		LOGBRUSH lbLight;
		lbLight.lbStyle = BS_SOLID;
		lbLight.lbColor = lightColor;
		lbLight.lbHatch = 0;
		
		CPen lightPen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND, 1, &lbLight);
		pDC->SelectObject(&lightPen);
		
		CRect lightRect = rectForArc;
		lightRect.DeflateRect(1, 1);
		lightRect.OffsetRect(-1, -1);
		pDC->Arc(lightRect, pointStart, pointEnd);
	}
	else if(pPart->type == Line)
	{
		// Определяем направление линии
		bool isHorizontal = abs(pointEnd.x - pointStart.x) > abs(pointEnd.y - pointStart.y);
		
		// Сначала рисуем тень
		LOGBRUSH lbShadow;
		lbShadow.lbStyle = BS_SOLID;
		lbShadow.lbColor = darkColor;
		lbShadow.lbHatch = 0;
		
		CPen shadowPen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND, penWidth, &lbShadow);
		pDC->SelectObject(&shadowPen);
		
		if(isHorizontal) {
			pDC->MoveTo(pointStart.x, pointStart.y + 1);
			pDC->LineTo(pointEnd.x, pointEnd.y + 1);
		} else {
			pDC->MoveTo(pointStart.x + 1, pointStart.y);
			pDC->LineTo(pointEnd.x + 1, pointEnd.y);
		}
		
		// Рисуем основную линию
		pDC->SelectObject(&pen);
		pDC->MoveTo(pointStart);
		pDC->LineTo(pointEnd);
		
		// Рисуем световой блик
		LOGBRUSH lbLight;
		lbLight.lbStyle = BS_SOLID;
		lbLight.lbColor = lightColor;
		lbLight.lbHatch = 0;
		
		CPen lightPen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND, 1, &lbLight);
		pDC->SelectObject(&lightPen);
		
		if(isHorizontal) {
			pDC->MoveTo(pointStart.x, pointStart.y - 1);
			pDC->LineTo(pointEnd.x, pointEnd.y - 1);
		} else {
			pDC->MoveTo(pointStart.x - 1, pointStart.y);
			pDC->LineTo(pointEnd.x - 1, pointEnd.y);
		}
	}

	// Восстанавливаем перо
	pDC->SelectObject(pOldPen);
	
	// Улучшенная отрисовка текста описания группы
	if(pPart->m_bHasDescription)
	{
		CFont* pOldFont = pDC->GetCurrentFont();
		pDC->SelectObject(&(CVisualSynanView::m_FontForGroupNames));

		TEXTMETRIC txtM;
		pDC->GetTextMetrics(&txtM);

		// Позиционирование текста
		int textX, textY;
		CString& text = m_strDescription;
		
		if(!pPart->m_bWholeArc)
		{
			textX = 6;
			textY = pPart->m_pointStart.y - txtM.tmHeight - 4 - iOffset;
		}
		else
		{
			// Центрируем текст для полной дуги
			CSize textSize = pDC->GetTextExtent(text);
			textX = m_pointLeft.x + (m_pointRight.x - m_pointLeft.x)/2 - textSize.cx/2;
			textY = pPart->m_pointStart.y - txtM.tmHeight - 4 - iOffset;
		}
		
		// Сохраняем текущий цвет текста
		COLORREF oldTextColor = pDC->GetTextColor();
		
		// Рисуем тень текста
		pDC->SetTextColor(RGB(60, 60, 60));
		pDC->TextOut(textX + 1, textY + 1, text, text.GetLength());
		
		// Выбираем цвет для текста в зависимости от яркости фона
		// Для более темных групп - светлый текст, для светлых - темный
		COLORREF textColor;
		if(brightness < 160) { // Порог для бирюзовых цветов
			textColor = RGB(240, 240, 240); // Почти белый для темных фонов
		} else {
			textColor = RGB(30, 30, 30); // Почти черный для светлых фонов
		}
		
		// Рисуем текст
		pDC->SetTextColor(textColor);
		pDC->TextOut(textX, textY, text, text.GetLength());
		
		// Восстанавливаем цвет текста
		pDC->SetTextColor(oldTextColor);
		pDC->SelectObject(pOldFont);
	}
	
	// Восстанавливаем режим фона
	pDC->SetBkMode(oldMode);
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


