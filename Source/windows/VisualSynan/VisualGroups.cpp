#include "StdAfx.h"
#include "VisualSentences.h"
#include <algorithm>


void CVisualGroups::CalculateGroupsCoordinates(CDC* pDC, int iWidth, int& iLine, int iPrevBottom)
{
	CVisualGroup* pGroup;
	BOOL bRes;

	// Получаем метрики текста для лучшего расчета позиционирования
	int iWordsCount = m_pWordsArr->GetSize();
	TEXTMETRIC txtM;
	pDC->GetTextMetrics(&txtM);	
	int iBottom;	
		
	// Обновленная зелено-бирюзовая цветовая схема
	const COLORREF baseColors[5] = {
		RGB(26, 188, 156),  // Бирюзовый (Turquoise)
		RGB(22, 160, 133),  // Зеленое море (Green Sea)
		RGB(46, 204, 113),  // Изумрудный (Emerald)
		RGB(39, 174, 96),   // Нефритовый (Nephrite)
		RGB(40, 180, 135)   // Средний бирюзово-зеленый
	};
	
	// Обрабатываем каждую активную группу
	for(int i = 0; i < m_arrActiveGroups.size(); i++)
	{
		COLORREF Color;
		pGroup = m_arrActiveGroups[i];
		
		// Выбираем цвета на основе типа и уровня группы
		if(pGroup->m_bClause) {
			// Для клауз используем более темный оттенок бирюзового
			Color = RGB(16, 124, 104);  // Темно-бирюзовый вместо красного
		} else {
			// Выбираем цвет из градиентной схемы в зависимости от уровня
			int colorIndex = min(pGroup->m_iLevel % 5, 4);
			Color = baseColors[colorIndex];
		}
		
		int iFirstWord = pGroup->m_iFirstWord;
		int iLastWord = pGroup->m_iLastWord;
		
		// Получаем координаты первого и последнего слова в группе
		CRect& rectFirstWord = ((CVisualWord*)(m_pWordsArr->GetAt(iFirstWord)))->m_WordRect;
		CRect& rectLastWord = ((CVisualWord*)(m_pWordsArr->GetAt(iLastWord)))->m_WordRect;

		iBottom = min(rectFirstWord.bottom, rectLastWord.bottom);
		BOOL bOnDifferentLines = (rectFirstWord.bottom != rectLastWord.bottom);

		// Пропускаем группы нулевого уровня
		if(pGroup->m_iLevel == 0)
			return;
		
		// Расчет высоты дуги в зависимости от уровня группы
		// Более высокий уровень = ниже дуга для лучшей видимости
		int yCoef = ((m_iSpaceBetweenLinesG - (txtM.tmHeight + (txtM.tmHeight/3)*2))/(m_iMaxGroupLevel)) * 
		           (m_iMaxGroupLevel - pGroup->m_iLevel);

		// Находим точки крепления левой и правой ножки дуги
		CPoint pointLeftLeg;
		bRes = GetLeftLegPointForGroupArc(i, &pointLeftLeg, iWidth);
		if(!bRes)
			return;

		CPoint pointRightLeg;
		bRes = GetRightLegPointForGroupArc(i, &pointRightLeg, iWidth);
		if(!bRes)
			return;
			
		// Верхняя точка дуги учитывает уровень вложенности
		int top = rectFirstWord.top - (m_iSpaceBetweenLinesG - (txtM.tmHeight + (txtM.tmHeight/3)*2)) + yCoef;
		
		// Уточняем, находятся ли слова группы на разных строках
		if(bOnDifferentLines) {
			bOnDifferentLines = !((pointLeftLeg.y < rectFirstWord.bottom) && 
			                      (pointRightLeg.y < rectFirstWord.bottom));
		}

		// Инкрементируем линию, если необходимо
		if((iBottom > iPrevBottom) && !bOnDifferentLines)
			iLine++;
		
		// Настраиваем расстояние между строками и рассчитываем координаты группы
		pGroup->SetSpaceBetweenLines(m_iSpaceBetweenLinesG);
		pGroup->CalculateCoordinates(pDC, pointLeftLeg, pointRightLeg, top, iWidth, 
		                             bOnDifferentLines, Color, iLine);

		// Запоминаем нижнюю координату для следующей группы
		iPrevBottom = min(rectFirstWord.bottom, rectLastWord.bottom);		
	}
}

// Улучшенный алгоритм нахождения левой точки крепления дуги
BOOL CVisualGroups::GetLeftLegPointForGroupArc(int iGroupNum, CPoint* pPoint, int iWidth)
{
	CVisualGroup* pGroupForDrawing = m_arrActiveGroups[iGroupNum];	
	BOOL bExistUsefulGroup = FALSE;
	CVisualGroup* pGroup = nullptr;

	// Поиск группы с тем же первым словом
	int i = iGroupNum - 1;
	for(; i >= 0; i--) {
		pGroup = m_arrActiveGroups[i];
		if(pGroup->m_iFirstWord == pGroupForDrawing->m_iFirstWord) {
			bExistUsefulGroup = TRUE;
			break;
		}		
	}

	// Если нашли подходящую группу и это не случай клаузы и не-клаузы
	if(bExistUsefulGroup && !(pGroupForDrawing->m_bClause && !m_arrActiveGroups[i]->m_bClause)) {	
		ASSERT(i >= 0);
		pGroup = m_arrActiveGroups[i];
		
		// Проверка корректности координат
		if((pGroup->m_pointLeft.x == 0) && (pGroup->m_pointRight.x))
			return FALSE;

		// Используем Y-координату существующей группы для визуального выравнивания
		pPoint->y = pGroup->m_pointLeft.y;

		// Если группа на одной линии, сдвигаем точку крепления для лучшего внешнего вида
		if(pGroup->m_pointLeft.y == pGroup->m_pointRight.y) {
			// Смещение для создания плавной дуги
			pPoint->x = pGroup->m_pointLeft.x + 10;
		}
		// Если группа на разных строках, позиционируем иначе
		else {
			pPoint->x = pGroup->m_pointLeft.x + 10;
		}
	}
	// Если нет подходящей группы, привязываемся к слову
	else {
		CVisualWord* pWord = (CVisualWord*)(m_pWordsArr->GetAt(pGroupForDrawing->m_iFirstWord));
		if(pWord->m_WordRect.IsRectEmpty())
			return FALSE;
		
		// Крепим к началу слова со смещением для эстетики
		pPoint->x = pWord->m_WordRect.left + 10;	
		pPoint->y = pWord->m_WordRect.top;
	}
	
	return TRUE;
}

// Улучшенный алгоритм нахождения правой точки крепления дуги
BOOL CVisualGroups::GetRightLegPointForGroupArc(int iGroupNum, CPoint* pPoint, int iWidth)
{
	CVisualGroup* pGroupForDrawing = m_arrActiveGroups[iGroupNum];
	CVisualGroup* pGroup = nullptr;
	BOOL bExistUsefulGroup = FALSE;	
	
	// Проверяем, есть ли группа с тем же последним словом
	if(iGroupNum > 0) {
		pGroup = m_arrActiveGroups[iGroupNum - 1];
		if(pGroup->m_iLastWord == pGroupForDrawing->m_iLastWord)
			bExistUsefulGroup = TRUE;
	}

	// Если нашли подходящую группу и условия соответствуют
	if(bExistUsefulGroup && !(pGroupForDrawing->m_bClause && !pGroup->m_bClause)) {
		// Используем Y-координату существующей группы
		pPoint->y = pGroup->m_pointLeft.y;

		// Если группа на одной строке
		if(pGroup->m_pointLeft.y == pGroup->m_pointRight.y) {
			// Размещаем точку крепления ближе к концу для более плавной дуги
			pPoint->x = pGroup->m_pointRight.x - 10;
		}
		// Если группа на разных строках
		else {
			// Специальное размещение для мультистрочных групп
			pPoint->x = pGroup->m_pointRight.x + iWidth - 10;
		}
	}
	// Если нет подходящей группы, привязываемся к слову
	else {
		CVisualWord* pWord = (CVisualWord*)(m_pWordsArr->GetAt(pGroupForDrawing->m_iLastWord));
		if(pWord->m_WordRect.IsRectEmpty())
			return FALSE;
		
		// Крепим к концу слова для баланса с левой стороной
		pPoint->x = pWord->m_WordRect.right - 10;	
		pPoint->y = pWord->m_WordRect.top;	
	}
	
	return TRUE;
}
