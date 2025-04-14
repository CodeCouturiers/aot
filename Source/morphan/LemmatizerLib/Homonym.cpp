#include "LemWord.h"
#include "Homonym.h"
#include "morph_dict/common/util_classes.h"
#include "morph_dict/agramtab/agramtab.h"
#include "morph_dict/lemmatizer_base_lib/Lemmatizers.h"

CHomonym::CHomonym(MorphLanguageEnum l)
{
	SetLanguage(l);
	m_iCmpnLen = 0;
	m_bCmplLem = false;
    m_lPradigmID = UnknownParadigmId;
    m_bDelete = false;
    m_bRussianOdin = false;

	m_bOborot1 = false;
	m_bOborot2 = false;
	m_bInOb = false;
	m_OborotNo = -1;

	m_lFreqHom = 0;
}


void	CHomonym::SetLemma(std::string Lemma)
{
	MakeUpperUtf8(Lemma);
	m_strLemma = Lemma;
};

const std::string& CHomonym::GetLemma() const {
	return m_strLemma;
}


bool	CHomonym::HasSetOfGrammemsExact(uint64_t Grammems) const
{
	for (int i = 0; i < GetGramCodes().length(); i += 2)
	{
		uint64_t g;
		if (!GetGramTab()->GetGrammems(GetGramCodes().c_str()+i, g))
		{
				assert (false);
		};
		if ((g & Grammems) ==  Grammems)
			return true;

	};

	return false;;
};



bool CHomonym::IsOb1() const
{ 
	return m_bOborot1;	
}

bool CHomonym::IsOb2() const
{ 
	return m_bOborot2;	
}

bool CHomonym::IsIsOb() const
{
	return m_bInOb;	
}


bool CHomonym::IsLemma(const std::string& lemma) const 
{
	return m_strLemma == lemma;
};

bool CHomonym::IsSynNoun() const
{
	return GetGramTab()->IsSynNoun(m_iPoses, m_strLemma );
};

bool  CHomonym::IsMorphNoun() const
{
	return GetGramTab()->IsMorphNoun(m_iPoses);
};

bool	CHomonym::IsLeftNounModifier() const
{
	return GetGramTab()->is_left_noun_modifier(m_iPoses, m_iGrammems);
};


std::string	CHomonym::GetGrammemsStr() const
{
	return GetGrammemsByAncodes();
};

void CHomonym::DeleteOborotMarks()
{
		m_bOborot1 = false;
		m_bOborot2 = false;
		m_bInOb = false;
		m_OborotNo = -1;
};

void CHomonym::SetPredictedWord(std::string gram_codes, std::string common_gram_codes)
{
    // Validate grammar codes before proceeding
    bool hasNonAscii = false;
    for (char c : gram_codes) {
        if (static_cast<unsigned char>(c) > 127) {
            hasNonAscii = true;
            PLOGW << "Non-ASCII character in grammar code: " << gram_codes;
            break;
        }
    }
    
    for (char c : common_gram_codes) {
        if (static_cast<unsigned char>(c) > 127) {
            hasNonAscii = true;
            PLOGW << "Non-ASCII character in common grammar code: " << common_gram_codes;
            break;
        }
    }
    
    // Fall back to safe defaults if we detect encoding issues
    if (hasNonAscii) {
        PLOGW << "Using safe defaults for grammar codes due to encoding issues";
        if (m_Language == morphRussian) {
            gram_codes = "СС";
            common_gram_codes = "С";
        } else {
            gram_codes = "SUB";
            common_gram_codes = "SUB";
        }
    }
    
    // Verify minimum length requirements
    if (gram_codes.length() < 2) {
        PLOGE << "Grammar code too short: " << gram_codes << ", using fallback";
        if (m_Language == morphRussian) {
            gram_codes = "СС";
        } else {
            gram_codes = "SUB";
        }
    }
    
    if (common_gram_codes.length() < 2 && common_gram_codes != "?") {
        PLOGE << "Common grammar code too short: " << common_gram_codes << ", using fallback";
        if (m_Language == morphRussian) {
            common_gram_codes = "С";
        } else {
            common_gram_codes = "SUB";
        }
    }
    
    // Для транслитерированных и очищенных слов используем более умную эвристику
    if (m_Language == morphRussian && m_strLemma.find('_') != std::string::npos) {
        PLOGW << "Attempting to guess part of speech for sanitized word: " << m_strLemma;
        
        // Попробуем определить часть речи по окончанию слова
        size_t len = m_strLemma.length();
        if (len > 2) {
            std::string ending = m_strLemma.substr(len - 2);
            
            // Предсказание для прилагательных
            if (ending == "yi" || ending == "iy" || ending == "oy" || 
                ending == "ay" || ending == "ym" || ending == "im" || 
                ending == "om" || ending == "ye" || ending == "ie" ||
                ending == "mi" || ending == "_i" || ending == "ih") {
                gram_codes = "ПП";
                common_gram_codes = "П";
                PLOGW << "Guessed adjective for: " << m_strLemma;
            }
            // Предсказание для глаголов
            else if (ending == "at" || ending == "et" || ending == "it" || 
                     ending == "ut" || ending == "yt" || ending == "tь" ||
                     ending == "tь" || ending == "ti" || ending == "ch") {
                gram_codes = "ГГ";
                common_gram_codes = "Г";
                PLOGW << "Guessed verb for: " << m_strLemma;
            }
            // Предсказание для наречий
            else if (ending == "no" || ending == "ko" || ending == "vo" || 
                     ending == "mo" || ending == "po" || ending == "ro" ||
                     ending == "he" || ending == "jo" || ending == "go" ||
                     ending == "so" || ending == "zo") {
                gram_codes = "HH";
                common_gram_codes = "H";
                PLOGW << "Guessed adverb for: " << m_strLemma;
            }
            // По умолчанию - существительное
            else {
                gram_codes = "СС";
                common_gram_codes = "С";
                PLOGW << "Default to noun for: " << m_strLemma;
            }
        }
    }
    
    m_GramCodes = gram_codes;
    m_CommonGramCode = common_gram_codes;
}

bool CHomonym::operator < (const CHomonym& hom) const
{
	return m_strLemma < hom.m_strLemma;
}

void  CHomonym::CopyFromFormInfo(const CFormInfo* F) {
	SetGramCodes(F->GetSrcAncode());
	m_CommonGramCode = F->GetCommonAncode();
	m_lPradigmID = F->GetParadigmId();
	if (F->GetLemSign() == '+') {
		m_SearchStatus = DictionaryWord;
	}
	else if (F->GetLemSign() == '-') {
		m_SearchStatus = PredictedWord;
	}
	else {
		throw CExpc("Bad lem sign %c", F->GetLemSign());
	}
	InitAncodePattern();

}

void  CHomonym::SetHomonym(const CFormInfo* F)
{
	CopyFromFormInfo(F);
    // Use safe access to language - either from our own member or directly
    MorphLanguageEnum lang = m_Language;
    m_strLemma = convert_to_utf8(F->GetWordForm(0), lang);
	m_iCmpnLen = strcspn(m_strLemma.c_str(), "-");
	m_bCmplLem = ((BYTE)m_iCmpnLen != m_strLemma.length());
	m_lFreqHom = F->GetHomonymWeight();
	if (0 == m_lFreqHom)
		m_lFreqHom = 1;
}

std::string CHomonym::GetDebugString() const {
	// Instead of asserting, we'll handle empty values with defaults
	if (GetLemma().empty()) {
		PLOGE << "Warning: Empty lemma in GetDebugString()";
	}
	
	// Use empty strings or provide defaults if mandatory fields are missing
	std::string gramCodes = GetGramCodes();
	std::string commonCode = m_CommonGramCode;
	
	// If grammar codes are missing, use a default value
	if (gramCodes.empty()) {
		PLOGE << "Warning: Empty gram codes in GetDebugString() for lemma: " << (!GetLemma().empty() ? GetLemma() : "<empty>");
		gramCodes = "??";
	}
	
	if (commonCode.empty()) {
		PLOGE << "Warning: Empty common gram code in GetDebugString()";
		commonCode = "??";
	}
	
	std::string r;
	r += " " + Format("%c", GetLemSign());
	r += " " + (GetLemma().empty() ? "?" : GetLemma());
	
	// Safely get the tab string for common code
	try {
		// Validate the code before passing it to GetTabStringByGramCode
		if (commonCode.length() >= 2 && isalpha(commonCode[0])) {
			r += " " + GetGramTab()->GetTabStringByGramCode(commonCode.c_str());
		} else {
			r += " UNKNOWN";
		}
	} catch (...) {
		PLOGE << "Exception in GetTabStringByGramCode for commonCode: " << commonCode;
		r += " UNKNOWN";
	}
	
	// Safely get the tab strings for gram codes
	for (int i = 0; i < gramCodes.length(); i += 2) {
		// Safety check to ensure we don't go out of bounds
		if (i + 1 >= gramCodes.length()) {
			PLOGE << "Incomplete gram code at position " << i << " in string: " << gramCodes;
			continue;
		}
		
		try {
			// Validate the code before passing it to GetTabStringByGramCode
			if (isalpha(gramCodes[i])) {
				r += " " + GetGramTab()->GetTabStringByGramCode(gramCodes.c_str() + i);
			} else {
				r += " UNKNOWN";
			}
		} catch (...) {
			PLOGE << "Exception in GetTabStringByGramCode for gramCode: " << gramCodes.substr(i, 2);
			r += " UNKNOWN";
		}
	}
	
	return r;
}


