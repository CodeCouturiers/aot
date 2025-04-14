// ==========  This file is under  LGPL, the GNU Lesser General Public License
// ==========  Dialing Syntax Analysis (www.aot.ru)
// ==========  Copyright by Dmitry Pankratov, Igor Nozhov, Alexey Sokirko

#include "Sentence.h"
#include "Word.h"
#include "assert.h"
#include "SynPlmLine.h"
#include "synan/SimpleGrammarLib/SimpleGrammar.h"
#include "morph_dict/agramtab/GerGramTab.h"



CSynWord::CSynWord(MorphLanguageEnum l) : CLemWord(l)
{
	Reset();
};

CSynWord::CSynWord(const CLemWord& c): CLemWord(c) {
	ResetSelfMembers();
	CLemWord::DeleteAllHomonyms(); // homonyms must be in CSynWord::m_Homonyms
}

void CSynWord::ResetSelfMembers() {
	m_Homonyms.clear();
	m_iClauseNo = -1;
	m_Homonyms.clear();
	m_bInTermin = false;
	m_ThesType = NoneThes;
	m_bFirstWordInTermin = false;;
	m_bLastWordInTermin = false;
	m_iTerminID = -1;
	m_bArtificialCreated = false;
	m_iReduplication = 0;
	m_bSmallNumber = false;

	m_bSimilarConj = false;

	m_SubordinateConjNo = -1;
	m_bBadParenthesis = false;
	m_TrennbarePraefixWordNo = -1;
	m_MainVerbs.clear();
}

void CSynWord::Reset()
{
	CLemWord::Reset();
	ResetSelfMembers();
}

void CSynWord::SetSentence(CSentence* s) {
	m_pSent = s;
}

const CSynHomonym& CSynWord::GetSynHomonym(int i) const { 
	// Add safety check to prevent access to an empty vector
	if (m_Homonyms.empty()) {
		PLOGE << "ERROR: Attempting to access homonym in empty vector for word: " << m_strWord;
		// Create an emergency static homonym to return - this is safer than crashing
		static CSynHomonym emergency(morphRussian);
		// Return by reference without modifying the vector (const method)
		return emergency;
	}
	// Boundary check
	if (i >= m_Homonyms.size()) {
		PLOGE << "ERROR: Homonym index out of bounds for word: " << m_strWord 
		      << ", index: " << i << ", size: " << m_Homonyms.size();
		// Return the first homonym as a fallback
		return m_Homonyms[0];
	}
	return m_Homonyms[i]; 
}

CSynHomonym& CSynWord::GetSynHomonym(int i) {
	// Add safety check to prevent access to an empty vector
	if (m_Homonyms.empty()) {
		PLOGE << "ERROR: Attempting to access homonym in empty vector for word: " << m_strWord;
		// Create a default homonym
		CSynHomonym h(m_pSent ? m_pSent->GetOpt()->m_Language : morphRussian);
		if (m_pSent) {
			h.SetSentence(m_pSent);
		}
		
		// For Russian words, default to noun ("С"); for others, use "SUB"
		// CRITICAL: Be extremely careful with these codes - they must be valid in the grammar table
		h.m_SearchStatus = PredictedWord; // Set status directly
		h.SetLemma(m_strUpperWord);
		
		// Use explicit default grammar codes without calling SetPredictedWord
		// to avoid potential null pointer issues
		if (m_pSent) {
			// Initialize pattern directly without calling potentially unsafe methods
			if (m_pSent->GetOpt()->m_Language == morphRussian) {
				h.m_CommonGramCode = "С";  // Russian noun
				h.SetGramCodes("СС");      // Use accessor instead of direct assignment
				h.m_iPoses = (1 << 0);     // First POS is noun in Russian
			} else {
				h.m_CommonGramCode = "SUB"; // German noun
				h.SetGramCodes("SUB");      // Use accessor instead of direct assignment
				h.m_iPoses = (1 << 0);     // Substantiv in German
			}
		} else {
			// Safe defaults if no sentence
			h.m_CommonGramCode = "??";
			h.SetGramCodes("??");          // Use accessor instead of direct assignment
		}
		
		// Add to vector
		m_Homonyms.push_back(h);
		PLOGW << "Created emergency homonym for word: " << m_strWord;
		return m_Homonyms.back();
	}
	
	// Boundary check
	if (i >= m_Homonyms.size()) {
		PLOGE << "ERROR: Homonym index out of bounds for word: " << m_strWord 
		      << ", index: " << i << ", size: " << m_Homonyms.size();
		// Return the first homonym as a fallback
		return m_Homonyms[0];
	}
	return m_Homonyms[i]; 
}

const CHomonym* CSynWord::GetHomonym(int i) const {
	// Add safety check to prevent access to an empty vector
	if (m_Homonyms.empty()) {
		PLOGE << "ERROR: Attempting to access homonym in empty vector for word: " << m_strWord;
		// Create an emergency static homonym to return - this is safer than crashing
		static CSynHomonym emergency(morphRussian);
		// Return by pointer without modifying the vector (const method)
		return &emergency;
	}
	// Boundary check
	if (i >= m_Homonyms.size()) {
		PLOGE << "ERROR: Homonym index out of bounds for word: " << m_strWord 
		      << ", index: " << i << ", size: " << m_Homonyms.size();
		// Return the first homonym as a fallback
		return &m_Homonyms[0];
	}
	return &m_Homonyms[i]; 
};

CHomonym* CSynWord::GetHomonym(int i) {
	// Simply call the non-const version of GetSynHomonym and cast the result
	// This allows us to reuse all the safety logic from there
	return const_cast<CHomonym*>(static_cast<const CSynWord*>(this)->GetHomonym(i));
};

CHomonym* CSynWord::AddNewHomonym() {
	CSynHomonym h(m_pSent->GetOpt()->m_Language);
	h.SetSentence(m_pSent);
	m_Homonyms.push_back(h); 
	return &m_Homonyms.back(); 
};


const	CSyntaxOpt* CSynWord::GetOpt() const	
{
	return m_pSent->GetOpt(); 
};



void CSynWord::UpdateConjInfo()
{
	m_SubordinateConjNo = -1;
	for(auto& h: m_Homonyms)
	{
		if (		h.m_SearchStatus == DictionaryWord
				&&	(			(GetOpt()->m_Language != morphGerman)
						||		!h.HasPos(gPRP) // "bis", "als" and  so on
					)
			)
			m_SubordinateConjNo = GetOpt()->GetOborDic()->FindSubConj(h.GetLemma() );
        h.m_CoordConjNo = m_pSent->GetCoordConjNo(h.GetLemma().c_str());
	}
    m_bSimilarConj = m_pSent->GetCoordConjNo(m_strUpperWord.c_str()) != -1;
};


CSynHomonym CSynWord::CloneHomonymByAnotherHomonym(const CSynHomonym* pHomonym, uint64_t iGrammems, BYTE iTagID) const 
{
	CSynHomonym hom = *pHomonym;
	if( iTagID == UnknownPartOfSpeech )
		hom.m_iPoses = 0;
	else
		hom.m_iPoses = (1 << iTagID);
	hom.m_iGrammems = iGrammems;

	return hom;
}



void CSynWord::CloneHomonymForOborot(int HNum)
{
	if (m_Homonyms.size() == 0) {
		PLOGE << "No homonyms found for word in CloneHomonymForOborot: " << m_strWord;
		// Create a default homonym before proceeding
		CSynHomonym h(m_pSent ? m_pSent->GetOpt()->m_Language : morphRussian);
		if (m_pSent) {
			h.SetSentence(m_pSent);
		}
		h.m_SearchStatus = PredictedWord;
		h.SetLemma(m_strUpperWord);
		
		// Use valid grammar codes for the current language
		if (m_pSent && m_pSent->GetOpt()->m_Language == morphRussian) {
			h.m_CommonGramCode = "С";  // Russian noun
			h.SetGramCodes("СС");      // Same code repeated for noun
			h.m_iPoses = (1 << 0);     // First POS is noun in Russian
		} else {
			h.m_CommonGramCode = "SUB"; // German noun
			h.SetGramCodes("SUB");      // Substantiv
			h.m_iPoses = (1 << 0);      // Set part of speech mask directly
		}
		
		m_Homonyms.push_back(h);
		PLOGW << "Created emergency homonym for CloneHomonymForOborot";
	}

	CSynHomonym H = CloneHomonymByAnotherHomonym( HNum == -1 ? &m_Homonyms.back() : &m_Homonyms[HNum], 0, UnknownPartOfSpeech);
	
	if( HasOborot1() )
	{
		//nim : добавить ч.р. = ВВОДН для оборота с одноименной GF 
		if (!m_bBadParenthesis)
			if ( GetOborotPtr()->HasPartOfSpeech(GetOpt()->m_RusParenthesis) )
            {
                H.m_iPoses |= (1<<GetOpt()->m_RusParenthesis);
            }
		
	}
	if(HNum == -1 && m_Homonyms.size()>1)
		H.m_lPradigmID = UnknownParadigmId;

	// у всех остальных омонимов помуты оборота стоять не будет
	DeleteOborotMarks();
	
	//  вставляем  омоним в начало списка , чтобы   работала функция  CSynWord::GetOborotNo()
	m_Homonyms.insert(m_Homonyms.begin(), H);
}




bool CSynWord::InitializePlmLine(CSynPlmLine& pPlmWord, int HomonymNo)  const
{
	if( (HomonymNo < 0) || (HomonymNo >= m_Homonyms.size()) )
		return false;

	const CSynHomonym& pActiveHomonym = m_Homonyms[HomonymNo];
	pPlmWord.SetGrammems(pActiveHomonym.m_iGrammems) ;
	pPlmWord.SetPoses(pActiveHomonym.m_iPoses);
    pPlmWord.SetGramcodes(  pActiveHomonym.GetGramCodes() );

    pPlmWord.InitSynPlmLine(this,&pActiveHomonym);
	

	if (m_bComma) pPlmWord.SetFlag(fl_comma);
	if (HasDes(ODigits)) pPlmWord.SetFlag(fl_digit);
	if (m_bWord && HasDes(OLLE)) pPlmWord.SetFlag(fl_ile);
	if (m_bWord) pPlmWord.SetFlag(fl_le );
	if (pActiveHomonym.m_bOborot1) pPlmWord.SetFlag(fl_oborot1 );
	if (pActiveHomonym.m_bOborot2) pPlmWord.SetFlag(fl_oborot2  );
	if (HasDes(OPun)) pPlmWord.SetFlag(fl_punct);
	if (pActiveHomonym.m_bSmallNumber) pPlmWord.SetFlag(fl_small_number);
	if (m_Homonyms.size() > 1) pPlmWord.SetFlag(fl_ambiguous);
	
	if (pActiveHomonym.m_bCanSubdueInfinitive) pPlmWord.SetFlag(fl_can_subdue_infinitive  );
	if (pActiveHomonym.m_bCanSubdueInstr) pPlmWord.SetFlag(fl_can_subdue_instr );
	if (pActiveHomonym.m_bNounHasAdjectiveDeclination) pPlmWord.SetFlag(fl_noun_has_adj_declination  );
	if (HasDes(ONumChar)) pPlmWord.SetFlag(fl_dg_ch );
	if (pActiveHomonym.m_bRussianOdin) pPlmWord.SetFlag(fl_russian_odin );
	if (pActiveHomonym.IsIsOb()) pPlmWord.SetFlag(fl_in_oborot );

	if (GetOpt()->GetGramTab()->IsStandardParamAbbr(m_strUpperWord.c_str())) pPlmWord.SetFlag(fl_standard_param_abbr  );
	if (m_pSent->IsProfession(pActiveHomonym)) pPlmWord.SetFlag(fl_ranknoun  );
	if (pPlmWord.is_single_punct('.')) pPlmWord.SetFlag(fl_fullstop );
	
	
	if (GetOpt()->GetGramTab()->IsMorphNoun(pActiveHomonym.m_iPoses)) pPlmWord.SetFlag(fl_morph_noun);
	if (pActiveHomonym.IsSynNoun()) pPlmWord.SetFlag(fl_syn_noun );
	
	
	return true;
}


const COborotForSyntax*	CSynWord::GetOborotPtr() const
{
	if (m_Homonyms.empty()) {
		PLOGE << "Empty homonyms vector in GetOborotPtr for word: " << m_strWord;
		return nullptr;
	}
	return m_Homonyms[0].GetOborotPtr();
};





void CSynWord::EraseHomonym(int iHom)
{
	m_Homonyms.erase(m_Homonyms.begin() + iHom);

	const CWorkGrammar& G = GetOpt()->m_FormatsGrammar;
	if (G.IsLoaded()) {
		BuildTerminalSymbolsByWord(G.m_UniqueGrammarItems, G.GetEndOfStreamSymbol());
	}
}


void CSynWord::InitLevelSpecific(CSynHomonym& h)
{

    m_pSent->InitHomonymLanguageSpecific(h, this);

	if	(		( m_bWord && (GetOpt()->m_Language == morphRussian))
			||	( HasDes(OLLE) && (GetOpt()->m_Language != morphRussian))
		)
	{
		m_pSent->InitHomonymMorphInfo(h);
	}
    

}


void CSynWord::SetGoodHomonym(int i )
{  
	   m_Homonyms[i].m_bGoodHomonym = true;
	   m_Homonyms[i].m_bDelete	 = false;
}

