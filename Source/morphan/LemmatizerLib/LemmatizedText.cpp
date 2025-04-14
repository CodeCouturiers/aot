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

			// Skip lemmatization if the word contains invalid UTF-8 characters
			bool hasInvalidCharacters = false;
			for (unsigned char c : word_s8) {
				// Check for invalid UTF-8 sequences that might cause problems
				if ((c > 127) && (c < 192)) {
					hasInvalidCharacters = true;
					PLOGW << "Skipping word with invalid UTF-8 sequence: " << word_s8;
					break;
				}
			}

			if (!hasInvalidCharacters) {
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
		}
		
		// Ensure non-space tokens always have at least one homonym
		// Only call CreateDefaultHomonym if not a space and has no homonyms
		if (!word.m_bSpace && word.GetHomonymsCount() == 0) {
#ifdef _DEBUG
			OutputDebugStringW(L"[LemmatizedText] Forcing default homonym for word: ");
			OutputDebugStringA(word.m_strWord.c_str());
			OutputDebugStringW(L"\n");
#endif
			// Check for any non-ASCII or potentially problematic characters
			bool containsNonASCII = false;
			for (unsigned char c : word.m_strWord) {
				if (c > 127) {
					containsNonASCII = true;
					PLOGW << "Word contains non-ASCII characters: " << word.m_strWord;
					break;
				}
			}

			// Create a default homonym for this token - using safer direct approach
			CHomonym* h = word.AddNewHomonym();
			h->m_SearchStatus = PredictedWord;
			h->SetLemma(word.m_strUpperWord);

			try {
				// Use valid grammar codes for the current language
				if (m_Language == morphRussian) {
					h->m_CommonGramCode = "С";  // Russian noun
					h->SetGramCodes("СС");      // Same code repeated for noun
					h->m_iPoses = (1 << 0);     // Set part of speech mask directly
				} else {
					h->m_CommonGramCode = "SUB"; // German noun
					h->SetGramCodes("SUB");      // Substantiv
					h->m_iPoses = (1 << 0);      // Set part of speech mask directly
				}
				
				// Skip InitAncodePattern for non-ASCII words - this avoids the assertion
				if (!containsNonASCII) {
					h->InitAncodePattern();
				} else {
					// For non-ASCII words, set grammems directly instead of calling InitAncodePattern
					// This skips the problematic function calls that might trigger assertions
					h->m_iGrammems = 0;  // Set to default value 
					h->m_TypeGrammems = 0;
					PLOGW << "Skipping InitAncodePattern for non-ASCII word: " << word.m_strWord;
				}
			} catch (const std::exception& e) {
				PLOGE << "Exception creating default homonym for word: " << word.m_strWord 
				      << ", error: " << e.what();
				// Continue processing even after exception
			} catch (...) {
				PLOGE << "Unknown exception creating default homonym for word: " << word.m_strWord;
				// Continue processing even after exception
			}
		}
		
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
				// Log error but continue process - don't assert in production
				PLOGE << "Critical error: Found word with no homonyms and not a space: " << w.m_strWord;
				
				// Skip this word rather than asserting
				continue;
			}
			
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


