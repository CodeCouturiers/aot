#include "morph_dict/lemmatizer_base_lib/Lemmatizers.h"
#include "LemmatizedText.h"
#include "morph_dict/lemmatizer_base_lib/MorphanHolder.h"

#include <fstream>
#include <Windows.h>
#include <debugapi.h>
#include <tchar.h>


CLemmatizedText::CLemmatizedText(MorphLanguageEnum l)
{
	m_Language = l;
};


void CLemmatizedText::CreateFromTokemized(const CGraphmatFile* Gr)
{
	auto lemmatizer = GetMHolder(m_Language).m_pLemmatizer;
	if (!lemmatizer) {
#ifdef _DEBUG
		OutputDebugStringW(L"[LemmatizedText] Error: Lemmatizer is null in CLemmatizedText::CreateFromTokemized\n");
#endif
		throw std::runtime_error("Lemmatizer is null");
	}

	
	m_LemWords.clear();

	bool bInFixedExpression = false;
	bool bHasSpaceAfter = true;
	short oborot_no = -1;
	for (const CGraLine& token: Gr->GetUnits())
	{
		//=====   do not lemmatize oborots with EXPR=Fixed!
		if (token.HasDes(OFixedOborot))
		{
			bInFixedExpression = true;
		};
		if (token.HasDes(OEXPR1)) {
			oborot_no = token.GetOborotNo();
		}

		CLemWord word(m_Language);
		word.CreateFromToken(token);
		word.m_bHasSpaceBefore = bHasSpaceAfter;
		bHasSpaceAfter = word.m_bSpace || token.HasSingleSpaceAfter();

		if (bInFixedExpression)
		{
			if (token.HasDes(OEXPR2))
				bInFixedExpression = false;
		}
		else if (m_Language == token.GetTokenLanguage())
		{
			std::string word_s8 = token.GetToken();
#ifdef _DEBUG
			OutputDebugStringW(L"[LemmatizedText] Attempting to lemmatize word: ");
			OutputDebugStringA(word_s8.c_str());
			OutputDebugStringW(L"\n");
#endif

			std::vector<CFormInfo> paradigms;
			lemmatizer->CreateParadigmCollection(false, word_s8, !token.HasDes(OLw), true, paradigms);
			
			if (paradigms.empty()) {
#ifdef _DEBUG
				OutputDebugStringW(L"[LemmatizedText] No paradigms found for word\n");
#endif
			}

			for(auto& p: paradigms)
			{
				CHomonym* h = word.AddNewHomonym();
				h->SetHomonym(&p);
				word.InitLevelSpecific(oborot_no, h);
			}
		}
		word.CreateDefaultHomonym(oborot_no);
		m_LemWords.push_back(word);

		if (token.HasDes(OEXPR2)) {
			oborot_no = -1;
		}
	}
}

MorphLanguageEnum CLemmatizedText::GetDictLanguage() const {
	return m_Language;
}


bool CLemmatizedText::SaveToFile(std::string filename) const
{
	try
	{
		std::ofstream outp(filename.c_str(), std::ios::binary);
		if (!outp.is_open()) return false;
		for (auto& w : m_LemWords) {
			if (!(w.m_bSpace || w.GetHomonymsCount() > 0)) {
#ifdef _DEBUG
				OutputDebugStringW(L"[LemmatizedText] Assertion will fail for word: ");
				OutputDebugStringA(w.m_strWord.c_str());
				OutputDebugStringW(L"\n");
				OutputDebugStringW(L"[LemmatizedText] m_bSpace=");
				OutputDebugStringW(w.m_bSpace ? L"true" : L"false");
				OutputDebugStringW(L" GetHomonymsCount()=");
				wchar_t buf[32];
				_snwprintf_s(buf, _countof(buf), L"%d", (int)w.GetHomonymsCount());
				OutputDebugStringW(buf);
				OutputDebugStringW(L"\n");
#endif
			}
			assert(w.m_bSpace || w.GetHomonymsCount() > 0);
			for (size_t i = 0; i < w.GetHomonymsCount(); ++i) {
				outp << w.GetDebugString(w.GetHomonym(i), i == 0) << "\n";
			}
		}
	}
	catch(...)
	{
		return false;
	}
	return true;
}


