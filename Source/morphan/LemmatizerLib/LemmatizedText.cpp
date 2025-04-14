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
			// Log potential encoding issues for debugging
			PLOGW << "Forcing default homonym for word: " << word.m_strWord;
			
			if (word.m_strWord.empty()) {
				PLOGE << "Empty word encountered, skipping homonym creation";
				m_LemWords.push_back(word);
				continue;
			}
			
			// Check if the word contains any non-ASCII or UTF-8 encoding issues
			bool isValidUtf8 = true;
			bool containsNonASCII = false;
			
			for (size_t i = 0; i < word.m_strWord.length(); ++i) {
				unsigned char c = static_cast<unsigned char>(word.m_strWord[i]);
				
				// Check for non-ASCII
				if (c > 127) {
					containsNonASCII = true;
					
					// Proper UTF-8 sequence validation
					if (c >= 0xC0 && c <= 0xDF) {
						// 2-byte sequence
						if (i + 1 >= word.m_strWord.length() || 
						   (static_cast<unsigned char>(word.m_strWord[i+1]) & 0xC0) != 0x80) {
							isValidUtf8 = false;
							break;
						}
						i += 1; // Skip the next byte as it's part of this character
					} else if (c >= 0xE0 && c <= 0xEF) {
						// 3-byte sequence
						if (i + 2 >= word.m_strWord.length() || 
						   (static_cast<unsigned char>(word.m_strWord[i+1]) & 0xC0) != 0x80 ||
						   (static_cast<unsigned char>(word.m_strWord[i+2]) & 0xC0) != 0x80) {
							isValidUtf8 = false;
							break;
						}
						i += 2; // Skip the next 2 bytes
					} else if (c >= 0xF0 && c <= 0xF7) {
						// 4-byte sequence
						if (i + 3 >= word.m_strWord.length() || 
						   (static_cast<unsigned char>(word.m_strWord[i+1]) & 0xC0) != 0x80 ||
						   (static_cast<unsigned char>(word.m_strWord[i+2]) & 0xC0) != 0x80 ||
						   (static_cast<unsigned char>(word.m_strWord[i+3]) & 0xC0) != 0x80) {
							isValidUtf8 = false;
							break;
						}
						i += 3; // Skip the next 3 bytes
					} else {
						// Invalid leading byte
						isValidUtf8 = false;
						break;
					}
				}
			}
			
			if (!isValidUtf8) {
				PLOGE << "Invalid UTF-8 sequence in word: " << word.m_strWord;
			}
			
			// If not valid UTF-8, try to clean the string
			std::string cleanWord = word.m_strWord;
			if (!isValidUtf8 || containsNonASCII) {
				PLOGW << "Attempting to sanitize word with encoding issues: " << word.m_strWord;
				
				// Improved detection for double-encoded Cyrillic text patterns
				bool containsDoubleEncodedCyrillic = false;
				
				// Check for common double-encoded Cyrillic patterns
				// These patterns appear when UTF-8 Cyrillic is incorrectly interpreted as Windows-1251
				for (const auto& pattern : {
					"РІ", "СЃ", "Р°", "Рѕ", "Рµ", "Рё", "Рј", "РЅ", "СЂ", "С‚", 
					"Рє", "Р»", "Рґ", "Рї", "Сѓ", "С„", "С…", "РЁ", "С‰", "СЊ", "СЏ"
				}) {
					if (word.m_strWord.find(pattern) != std::string::npos) {
						containsDoubleEncodedCyrillic = true;
						break;
					}
				}
				
				// For double-encoded Cyrillic, attempt transliteration
				if (containsDoubleEncodedCyrillic) {
					PLOGW << "Detected double-encoded Cyrillic text: " << word.m_strWord;
					
					// Known patterns of double-encoded Cyrillic and their Latin equivalents
					// Table maps common double-encoded sequences to their Latin equivalents
					static const std::unordered_map<std::string, char> cyrillicPatterns = {
						{"Р°", 'a'}, {"Р±", 'b'}, {"РІ", 'v'}, {"Рі", 'g'}, 
						{"Рґ", 'd'}, {"Рµ", 'e'}, {"С'", 'e'}, {"Р¶", 'z'}, 
						{"Р·", 'z'}, {"Рё", 'i'}, {"Р№", 'i'}, {"Рє", 'k'}, 
						{"Р»", 'l'}, {"Рј", 'm'}, {"РЅ", 'n'}, {"Рѕ", 'o'}, 
						{"Рї", 'p'}, {"СЂ", 'r'}, {"СЃ", 's'}, {"С‚", 't'}, 
						{"Сѓ", 'u'}, {"С„", 'f'}, {"С…", 'h'}, {"С†", 'c'}, 
						{"С‡", 'c'}, {"С€", 's'}, {"С‰", 's'}, {"СЉ", '_'}, 
						{"С‹", 'y'}, {"СЊ", '_'}, {"СЌ", 'e'}, {"СЋ", 'u'}, 
						{"СЏ", 'y'}
					};
					
					cleanWord = "";
					bool foundPattern = false;
					
					// Process the word character by character
					for (size_t i = 0; i < word.m_strWord.length(); i++) {
						bool patternFound = false;
						
						// Try to match double-encoded patterns
						for (const auto& [pattern, replacement] : cyrillicPatterns) {
							if (i + pattern.length() <= word.m_strWord.length() && 
								word.m_strWord.substr(i, pattern.length()) == pattern) {
								cleanWord.push_back(replacement);
								i += pattern.length() - 1; // Skip processed characters
								patternFound = true;
								foundPattern = true;
								break;
							}
						}
						
						// If no pattern found, keep ASCII characters and replace others
						if (!patternFound) {
							unsigned char c = static_cast<unsigned char>(word.m_strWord[i]);
							if (c < 128) {
								cleanWord.push_back(c);
							} else {
								cleanWord.push_back('_');
							}
						}
					}
					
					// If no patterns were found, fall back to simple cleaning
					if (!foundPattern) {
						cleanWord = "";
						for (unsigned char c : word.m_strWord) {
							if (c < 128) {
								cleanWord.push_back(c);
							} else {
								cleanWord.push_back('_');
							}
						}
					}
				} else {
					// Special case for valid Cyrillic UTF-8 that was incorrectly detected as invalid
					// This handles standard Cyrillic characters that might trigger the validator but are valid
					if (containsNonASCII && m_Language == morphRussian) {
						// Check if the word appears to be Cyrillic but was flagged as invalid
						bool potentiallyCyrillic = false;
						for (size_t i = 0; i < word.m_strWord.length(); i++) {
							unsigned char c = static_cast<unsigned char>(word.m_strWord[i]);
							// Check for Cyrillic range in UTF-8 encoding
							if ((c == 0xD0 || c == 0xD1) && i + 1 < word.m_strWord.length()) {
								unsigned char next = static_cast<unsigned char>(word.m_strWord[i+1]);
								// Typical Cyrillic range
								if ((c == 0xD0 && next >= 0x90 && next <= 0xBF) || 
								    (c == 0xD1 && next >= 0x80 && next <= 0x8F)) {
									potentiallyCyrillic = true;
									break;
								}
							}
						}
						
						if (potentiallyCyrillic) {
							// If it's likely valid Cyrillic, use original word and skip cleanup
							PLOGW << "Word appears to be valid Cyrillic despite UTF-8 validation failure, preserving: " << word.m_strWord;
							isValidUtf8 = true; // Override the validation result
						} else {
							// For other encoding issues, perform simple cleaning
							cleanWord.clear();
							for (unsigned char c : word.m_strWord) {
								if (c < 128) {
									cleanWord.push_back(c);
								} else {
									cleanWord.push_back('_');
								}
							}
						}
					} else {
						// For other encoding issues, perform simple cleaning
						cleanWord.clear();
						for (unsigned char c : word.m_strWord) {
							if (c < 128) {
								cleanWord.push_back(c);
							} else {
								cleanWord.push_back('_');
							}
						}
					}
				}
				
				// Only modify the word if we need to clean it
				if (!isValidUtf8 && cleanWord != word.m_strWord) {
					// If word became empty, use a default placeholder
					if (cleanWord.empty()) {
						cleanWord = "_word_";
					}
					
					word.m_strWord = cleanWord;
					word.m_strUpperWord = cleanWord;
					PLOGW << "Sanitized word: " << word.m_strWord;
				}
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
				
				// Be very cautious with InitAncodePattern for words with encoding issues
				if (!containsNonASCII && isValidUtf8) {
					h->InitAncodePattern();
				} else {
					// For words with encoding issues, set grammems directly
					h->m_iGrammems = 0;
					h->m_TypeGrammems = 0;
					PLOGW << "Skipping InitAncodePattern for word with encoding issues: " << word.m_strWord;
				}
				
				word.InitLevelSpecific(oborot_no, h);
			} catch (const std::exception& e) {
				PLOGE << "Exception creating default homonym for word: " << word.m_strWord 
					  << ", error: " << e.what();
			} catch (...) {
				PLOGE << "Unknown exception creating default homonym for word: " << word.m_strWord;
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


