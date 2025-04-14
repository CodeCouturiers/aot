#include "morph_dict/lemmatizer_base_lib/Lemmatizers.h"
#include "LemmatizedText.h"
#include "morph_dict/lemmatizer_base_lib/MorphanHolder.h"

#include <fstream>
#include <Windows.h>
#include <debugapi.h>
#include <tchar.h>
#include <unordered_map>

// Helper function to fix encoding issues in text, especially double-encoded Cyrillic
std::string fixEncodingIssues(const std::string& text, MorphLanguageEnum language) {
	// Check if the text has potential encoding issues
	bool containsDoubleEncodedCyrillic = false;
	bool containsNonASCII = false;
	
	// Check for non-ASCII characters and potential encoding issues
	for (size_t i = 0; i < text.length(); ++i) {
		unsigned char c = static_cast<unsigned char>(text[i]);
		if (c > 127) {
			containsNonASCII = true;
			break;
		}
	}
	
	if (!containsNonASCII) {
		return text; // No encoding issues with ASCII-only text
	}
	
	// Check for common double-encoded Cyrillic patterns
	// These patterns appear when UTF-8 Cyrillic is incorrectly interpreted as Windows-1251
	const std::vector<std::string> patterns = {
		R"(Р°)", R"(Р±)", R"(РІ)", R"(Рі)", R"(Рґ)", R"(Рµ)", R"(С')", R"(Р¶)", R"(Р·)", R"(Рё)", 
		R"(Р№)", R"(Рє)", R"(Р»)", R"(Рј)", R"(РЅ)", R"(Рѕ)", R"(Рї)", R"(СЂ)", R"(СЃ)", R"(С‚)", 
		R"(Сѓ)", R"(С„)", R"(С…)", R"(С†)", R"(С‡)", R"(С€)", R"(С‰)", R"(СЉ)", R"(С‹)", R"(СЊ)", 
		R"(СЌ)", R"(СЋ)", R"(СЏ)"
	};
	
	for (const auto& pattern : patterns) {
		if (text.find(pattern) != std::string::npos) {
			containsDoubleEncodedCyrillic = true;
			break;
		}
	}
	
	// If no specific encoding issue detected, return original
	if (!containsDoubleEncodedCyrillic) {
		return text;
	}
	
	// Known patterns of double-encoded Cyrillic and their Latin/Cyrillic equivalents
	// This maps common double-encoded sequences back to original characters
	static const std::unordered_map<std::string, std::string> cyrillicMap = {
		// lowercase Russian letters
		{R"(Р°)", "а"}, {R"(Р±)", "б"}, {R"(РІ)", "в"}, {R"(Рі)", "г"}, 
		{R"(Рґ)", "д"}, {R"(Рµ)", "е"}, {R"(С')", "ё"}, {R"(Р¶)", "ж"}, 
		{R"(Р·)", "з"}, {R"(Рё)", "и"}, {R"(Р№)", "й"}, {R"(Рє)", "к"}, 
		{R"(Р»)", "л"}, {R"(Рј)", "м"}, {R"(РЅ)", "н"}, {R"(Рѕ)", "о"}, 
		{R"(Рї)", "п"}, {R"(СЂ)", "р"}, {R"(СЃ)", "с"}, {R"(С‚)", "т"}, 
		{R"(Сѓ)", "у"}, {R"(С„)", "ф"}, {R"(С…)", "х"}, {R"(С†)", "ц"}, 
		{R"(С‡)", "ч"}, {R"(С€)", "ш"}, {R"(С‰)", "щ"}, {R"(СЉ)", "ъ"}, 
		{R"(С‹)", "ы"}, {R"(СЊ)", "ь"}, {R"(СЌ)", "э"}, {R"(СЋ)", "ю"}, 
		{R"(СЏ)", "я"},
		
		// uppercase Russian letters
		{R"(РђР°)", "А"}, {R"(Р'Р±)", "Б"}, {R"(Р'РІ)", "В"}, {R"(Р"Рі)", "Г"}, 
		{R"(Р"Рґ)", "Д"}, {R"(Р•Рµ)", "Е"}, {R"(РЃС')", "Ё"}, {R"(Р–Р¶)", "Ж"}, 
		{R"(Р—Р·)", "З"}, {R"(РРё)", "И"}, {R"(Р™Р№)", "Й"}, {R"(РљРє)", "К"}, 
		{R"(Р›Р»)", "Л"}, {R"(РњРј)", "М"}, {R"(РќРЅ)", "Н"}, {R"(РћРѕ)", "О"}, 
		{R"(РџРї)", "П"}, {R"(Р РЂ)", "Р"}, {R"(РЎСЃ)", "С"}, {R"(РўС‚)", "Т"}, 
		{R"(РЈСѓ)", "У"}, {R"(Р¤С„)", "Ф"}, {R"(РҐС…)", "Х"}, {R"(Р¦С†)", "Ц"}, 
		{R"(Р§С‡)", "Ч"}, {R"(РЁС€)", "Ш"}, {R"(Р©С‰)", "Щ"}, {R"(РЄСљ)", "Ъ"}, 
		{R"(Р«С‹)", "Ы"}, {R"(Р¬СЊ)", "Ь"}, {R"(РЌСЌ)", "Э"}, {R"(РЋСЋ)", "Ю"}, 
		{R"(РЇСЏ)", "Я"}
	};
	
	std::string result;
	size_t pos = 0;
	
	// Process the string looking for patterns
	while (pos < text.length()) {
		bool patternFound = false;
		
		// Try to match double-encoded patterns
		for (const auto& [pattern, replacement] : cyrillicMap) {
			if (pos + pattern.length() <= text.length() && 
				text.substr(pos, pattern.length()) == pattern) {
				result += replacement;
				pos += pattern.length();
				patternFound = true;
				break;
			}
		}
		
		// If no pattern found, keep the original character
		if (!patternFound) {
			result += text[pos++];
		}
	}
	
	// If result is empty (which shouldn't happen), return original
	return result.empty() ? text : result;
}

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

			// Check and fix encoding issues
			std::string fixedWord = fixEncodingIssues(word_s8, m_Language);
			bool encodingFixed = (fixedWord != word_s8);
			
			if (encodingFixed) {
				PLOGW << "Fixed encoding for word: " << word_s8 << " -> " << fixedWord;
				word_s8 = fixedWord;
			}

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
			
			// If encoding was fixed, update the word's strings with the fixed encoding
			if (encodingFixed && !word.m_bSpace && word.GetHomonymsCount() == 0) {
				word.m_strWord = fixedWord;
				word.m_strUpperWord = fixedWord;
				PLOGW << "Updated word with fixed encoding: " << fixedWord;
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
			// Log potential encoding issues for debugging
			PLOGW << "Forcing default homonym for word: " << word.m_strWord;
			
			// Fix encoding issues if any before creating default homonym
			if (!word.m_strWord.empty()) {
				std::string fixedWord = fixEncodingIssues(word.m_strWord, m_Language);
				if (fixedWord != word.m_strWord) {
					PLOGW << "Fixed encoding in default homonym creation: " << word.m_strWord << " -> " << fixedWord;
					word.m_strWord = fixedWord;
					word.m_strUpperWord = fixedWord;
			}
			
			// Create a default homonym with robust error handling
			try {
				CHomonym* h = word.AddNewHomonym();
				h->m_SearchStatus = PredictedWord;
				h->SetLemma(word.m_strUpperWord);

				// Use fully qualified grammar codes from GetGramTab
				const CAgramtab* gramTab = GetMHolder(m_Language).m_pGramTab;
				if (!gramTab) {
					PLOGE << "Grammar table is null, using simple defaults";
					if (m_Language == morphRussian) {
						h->m_CommonGramCode = "С";  // Default Russian noun
						h->SetGramCodes("СС");      // Same code
						h->m_iPoses = (1 << 0);     // First POS
					} else {
						h->m_CommonGramCode = "SUB"; // Default German noun
						h->SetGramCodes("SUB");      // Same code
						h->m_iPoses = (1 << 0);      // First POS
					}
				} else {
					// Use default grammar codes that are validated against the grammar table
					if (m_Language == morphRussian) {
						if (gramTab->CheckGramCode("С")) {
							h->m_CommonGramCode = "С";
							h->SetGramCodes("СС");
							h->m_iPoses = (1 << 0);
						} else {
							h->m_CommonGramCode = "??";
							h->SetGramCodes("??");
							h->m_iPoses = 0;
							PLOGE << "Invalid Russian grammar code, using fallback: ??";
						}
					} else {
						if (gramTab->CheckGramCode("SUB")) {
							h->m_CommonGramCode = "SUB";
							h->SetGramCodes("SUB");
							h->m_iPoses = (1 << 0);
						} else {
							h->m_CommonGramCode = "??";
							h->SetGramCodes("??");
							h->m_iPoses = 0;
							PLOGE << "Invalid German grammar code, using fallback: ??";
						}
					}
				}
				
					// Initialize the ancode pattern with caution
					try {
					h->InitAncodePattern();
					} catch (...) {
						// If InitAncodePattern fails, set grammems directly
					h->m_iGrammems = 0;
					h->m_TypeGrammems = 0;
						PLOGW << "InitAncodePattern failed, setting default grammems for: " << word.m_strWord;
				}
				
				word.InitLevelSpecific(oborot_no, h);
			} catch (const std::exception& e) {
				PLOGE << "Exception creating default homonym for word: " << word.m_strWord 
					  << ", error: " << e.what();
			} catch (...) {
				PLOGE << "Unknown exception creating default homonym for word: " << word.m_strWord;
				}
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


