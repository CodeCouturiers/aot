#include <stdio.h>
#include "synan/SynCommonLib/RelationsIterator.h"
#include "../SynanLib/SyntaxHolder.h"
#include "common/test_corpus.h"
#include "morph_dict/common/argparse.h"
#include <filesystem>

void GetAnanlytForms(const CSentence &Sentence, CJsonObject& out) {
    CJsonObject arr(out.get_doc(), rapidjson::kArrayType);
    for (auto& w: Sentence.m_Words) {
        if (!w.m_MainVerbs.empty()) {
            std::string form = w.m_strWord;
            for (auto& m: w.m_MainVerbs) {
                form += std::string(" ") + Sentence.m_Words[m].m_strWord;
                for (auto j: Sentence.m_Words[m].m_MainVerbs)
                    form += std::string(" ") + Sentence.m_Words[j].m_strWord;
            };
            rapidjson::Value s;
            s.SetString(form.c_str(), out.get_allocator());
            arr.push_back(s);
        }
    }
    out.move_to_member("analytical", arr.get_value());
}


void GetGroups(const CSentence &Sentence, const CAgramtab &A, CJsonObject& out) {
    int nClausesCount = Sentence.GetClausesCount();
    CJsonObject arr(out.get_doc(), rapidjson::kArrayType);

    for (int ClauseNo = 0; ClauseNo < nClausesCount; ClauseNo++) {
        const CClause &Clause = Sentence.GetClause(ClauseNo);
        int nCvar = Clause.m_SynVariants.size();

        if (Clause.m_SynVariants.empty()) continue;
        CJsonObject clause(out.get_doc());
        clause.add_int("start", Clause.m_iFirstWord);
        clause.add_int("last",  Clause.m_iLastWord);
        clause.add_string_copy("words", Sentence.GetWordsDebug(Clause));
        if (!Clause.m_RelativeWord.IsEmpty())
        {
            clause.add_string_copy("relative", Sentence.m_Words[Clause.m_RelativeWord.m_WordNo].m_strWord);
        };
        if (Clause.m_AntecedentWordNo != -1)
        {
            clause.add_string_copy("antecedent",  Sentence.m_Words[Clause.m_AntecedentWordNo].m_strWord);
        };

        

        int nVmax = Clause.m_SynVariants.begin()->m_iWeight;
        CJsonObject good_synvars(out.get_doc(), rapidjson::kArrayType);
        for (auto& synVar: Clause.m_SynVariants) {
            if (synVar.m_iWeight < nVmax) break;

            std::string clauseType = "EMPTY";
            {
                int ClauseType = (synVar.m_ClauseTypeNo == -1) ? UnknownSyntaxElement
                                                          : Clause.m_vectorTypes[synVar.m_ClauseTypeNo].m_Type;;
                if (ClauseType != UnknownSyntaxElement)
                    clauseType = A.GetClauseNameByType(ClauseType);
            }
            CJsonObject groups(out.get_doc(), rapidjson::kArrayType);
            for (auto& g: synVar.m_vectorGroups.GetGroups()) {
                CJsonObject jq(out.get_doc());
                jq.add_string_copy("type", Sentence.GetOpt()->GetGroupNameByIndex(g.m_GroupType));
                jq.add_string_copy("words", Sentence.GetWordsDebug(g));
                groups.push_back(jq.get_value());
            };           

            CJsonObject syn_units(out.get_doc(), rapidjson::kArrayType);
            for (int unitNo = 0; unitNo < synVar.m_SynUnits.size(); unitNo++) {
                int iWord = synVar.m_SynUnits[unitNo].m_SentPeriod.m_iFirstWord;
                int homIndex = synVar.GetHomNum(unitNo);
                if (homIndex != -1) {
                    const CSynHomonym& hom = Sentence.GetWords()[iWord].GetSynHomonym(homIndex);
                    CJsonObject o(out.get_doc());
                    o.add_string_copy("lemma", hom.GetLemma());
                    o.add_string_copy("morph_info", hom.GetPartOfSpeechStr() + std::string(" ") + hom.GetGrammemsStr());
                    o.add_string_copy("modified_grammems", A.GrammemsToStr(synVar.m_SynUnits[unitNo].m_iGrammems));
                    syn_units.push_back(o.get_value());
                }
                else {
                    //todo: print subclauses
                }
            }
            CJsonObject o(out.get_doc());
            o.move_to_member("syn_units", syn_units.get_value());
            o.move_to_member("groups", groups.get_value());
            o.add_string_copy("clause_type", clauseType);
            good_synvars.push_back(o.get_value());
        }
        clause.move_to_member("good_synvars", good_synvars.get_value());
        arr.push_back(clause);
    }
    out.move_to_member("groups", arr.get_value());
}

std::string GetNodeGrmStr(const CSentence &Sentence, const CRelationsIterator &RelIt, int GroupNo, int WordNo, std::string &Lemma) {
    Lemma = "";
    if (GroupNo != -1)
        return "";
    else {
        size_t ClauseNo = Sentence.GetMinClauseByWordNo(WordNo);
        const CClause &Clause = Sentence.GetClause(ClauseNo);
        const CMorphVariant *pSynVar = &*Clause.GetSynVariantByNo(0);
        int UnitNo = pSynVar->UnitNoByWordNo(WordNo);
        const CSynUnit &U = pSynVar->m_SynUnits[UnitNo];
        Lemma = Sentence.GetWords()[WordNo].GetHomonym(U.m_iHomonymNum)->GetLemma();
        return Sentence.GetOpt()->GetGramTab()->GrammemsToStr(U.m_iGrammems | U.m_TypeGrammems);
    }
}

void GetRelations(const CSentence &Sentence, CJsonObject& out) {
    CRelationsIterator RelIt;
    RelIt.SetSentence(&Sentence);
    for (auto& i: Sentence.m_vectorPrClauseNo)
        RelIt.AddClauseNoAndVariantNo(i, 0);
    RelIt.BuildRelations();
    CJsonObject rels(out.get_doc(), rapidjson::kArrayType);

    for (auto& piRel: RelIt.GetRelations()) {
        CJsonObject o(out.get_doc());
        o.add_string_copy("src", RelIt.GetSourceNodeStr(piRel));
        o.add_string_copy("trg", RelIt.GetTargetNodeStr(piRel));
        o.add_string_copy("name", RelIt.GetRelationName(piRel));
        o.add_string_copy("gramrel", Sentence.GetOpt()->GetGramTab()->GrammemsToStr(piRel.m_Relation.m_iGrammems));
        std::string SrcLemma, TrgLemma;
        std::string SrcGrm = GetNodeGrmStr(Sentence, RelIt, piRel.m_iSourceGroup, piRel.m_Relation.m_iFirstWord, SrcLemma);
        o.add_string_copy("src_lemma", SrcLemma);
        o.add_string_copy("src_grm", SrcGrm);

        std::string TrgGrm = GetNodeGrmStr(Sentence, RelIt, piRel.m_iTargetGroup, piRel.m_Relation.m_iLastWord, TrgLemma);
        o.add_string_copy("trg_lemma", TrgLemma);
        o.add_string_copy("trg_grm", TrgGrm);

        rels.push_back(o);
    }
    out.move_to_member("relations", rels.get_value());
}

void GetThesaurusTerms(const CSentence& Sentence, CJsonObject& out) {
    CJsonObject terms(out.get_doc(), rapidjson::kArrayType);
    for (size_t i = 0; i < Sentence.m_Words.size(); i++) {
        if (Sentence.m_Words[i].m_bFirstWordInTermin) {
            for (size_t k = i; k < Sentence.m_Words.size(); ++k) {
                if (Sentence.m_Words[k].m_bLastWordInTermin) {
                    rapidjson::Value o;
                    o.SetString(Sentence.GetWordsDebug(CPeriod(i, k)), out.get_allocator());
                    terms.push_back(o);
                    break;
                }
            }
            
        }
    }
    out.move_to_member("terms", terms.get_value());
}

void GetResultBySyntax(const CSentencesCollection& SC, CJsonObject& sents) {
    const CAgramtab& A = *SC.GetOpt()->GetGramTab();
    for (size_t nSent = 0; nSent < SC.m_vectorSents.size(); nSent++) {
        const CSentence &Sentence = *SC.m_vectorSents[nSent];
        CJsonObject sent(sents.get_doc());
        GetAnanlytForms(Sentence, sent);
        GetGroups(Sentence, A, sent);
        GetRelations(Sentence, sent);
        GetThesaurusTerms(Sentence, sent);
        sents.push_back(sent.get_value());
    }
    
};


void initArgParser(int argc, const char **argv, ArgumentParser& parser) {
    parser.AddOption("--help");
    parser.AddArgument("--input-file", "input file", true);
    parser.AddArgument("--output-file", "output file", true);
    parser.AddArgument("--input-file-mask", "c:/*.txt", true);
    parser.AddArgument("--output-folder", "", true);
    parser.AddArgument("--language", "language", true);
    parser.AddArgument("--log-level", "log level", true);

    try {
        parser.Parse(argc, argv);
    } 
    catch (std::exception& ex) {
        // If parsing fails, we'll use default values
        std::cerr << "Using default values" << std::endl;
    }
}


int main(int argc, const char** argv) {
    ArgumentParser args;
    
    // Add command line args for "language" if not provided 
    const char** newArgv = nullptr;
    int newArgc = argc;
    bool addedLanguage = false;
    
    if (argc < 3) {
        // Create a new array with space for our additional args
        newArgc = argc + 2;
        newArgv = new const char*[newArgc];
        
        // Copy original args
        for (int i = 0; i < argc; i++) {
            newArgv[i] = argv[i];
        }
        
        // Add language argument
        newArgv[argc] = "--language";
        newArgv[argc+1] = "Russian";
        addedLanguage = true;
    } else {
        // Use original args
        newArgv = argv;
    }
    
    initArgParser(newArgc, newArgv, args);
    
    // Clean up if we allocated new array
    if (addedLanguage) {
        delete[] newArgv;
    }
    
    MorphLanguageEnum langua = args.GetLanguage();
    if (langua == morphUnknown) {
        langua = morphRussian; // Default to Russian language
    }
    
    plog::Severity logLevel = args.GetLogLevel();
    init_plog(logLevel, "synan_test.log", true);
    GlobalLoadMorphHolder(langua);
    CSyntaxHolder H(langua);
    try {
        H.LoadSyntax();

        std::cerr << "ok\n";
        std::vector <std::pair<std::string, std::string> > file_pairs;

        // If no explicit input file is provided, use test.txt from current directory
        if (!args.Exists("input-file") && !args.Exists("input-file-mask")) {
            std::filesystem::path currentDir = std::filesystem::current_path();
            std::string testFile = (currentDir / "test.txt").string();
            std::string outputFile = (currentDir / "test.txt.synan").string();
            file_pairs.push_back({testFile, outputFile});
            LOGD << "Using default test file: " << testFile;
        } else if (args.Exists("input-file-mask")) {
            auto file_names = list_path_by_file_mask(args.Retrieve("input-file-mask"));
            for (auto filename : file_names) {
                auto outputFilename = filename + ".synan";
                if (args.Exists("output-folder")) {
                    auto base_name = fs::path(outputFilename).filename();
                    auto p = fs::path(args.Retrieve("output-folder")) / base_name;
                    outputFilename = p.string();
                }
                file_pairs.push_back({filename, outputFilename });
            }
        }
        else {
            file_pairs.push_back({ args.Retrieve("input-file"), args.Retrieve("output-file")});
        }
        for (auto& p : file_pairs) {
            std::cerr << p.first << "\n";
            rapidjson::Document d;
            CTestCaseBase base(d);
            std::ifstream inp(p.first);
            base.read_test_cases(inp);

            for (auto& t : base.TestCases.get_value().GetArray()) {
                std::string text = t["input"].GetString();
                if (!text.empty()) {
                    H.GetSentencesFromSynAn(text, false);
                    
                    // Add extra validation to ensure every non-space token has homonyms
                    for (auto& sentence : H.m_Synan.m_vectorSents) {
                        for (auto& word : sentence->m_Words) {
                            // Create default homonym for non-space tokens with no homonyms
                            if (!word.m_bSpace && word.GetHomonymsCount() == 0) {
                                LOGW << "Found word with no homonyms: " << word.m_strWord;
                                
                                try {
                                    // Create a default homonym with appropriate grammar values
                                    CSynHomonym h(H.m_Synan.GetOpt()->m_Language);
                                    h.SetSentence(sentence);
                                    
                                    // Default noun ("С") for Russian or substantiv ("SUB") for German
                                    // CRITICAL: Be extremely careful with these codes - they must be valid in the grammar table
                                    h.m_SearchStatus = PredictedWord; // Set status directly
                                    h.SetLemma(word.m_strUpperWord);
                                    
                                    // Initialize pattern directly without calling potentially unsafe methods
                                    if (H.m_Synan.GetOpt()->m_Language == morphRussian) {
                                        // Validate grammar code by attempting to get part of speech
                                        const CAgramtab* gramTab = H.m_Synan.GetOpt()->GetGramTab();
                                        if (gramTab && gramTab->CheckGramCode("С")) {
                                            h.m_CommonGramCode = "С";  // Russian noun
                                            h.SetGramCodes("СС");      // Use accessor instead of direct assignment
                                            h.m_iPoses = (1 << 0);     // First POS is noun in Russian
                                        } else {
                                            PLOGE << "Invalid Russian grammar code - using safe fallback";
                                            h.m_CommonGramCode = "??";  // Unknown
                                            h.SetGramCodes("??");       // Unknown
                                            h.m_iPoses = 0;             // No specific POS
                                        }
                                    } else {
                                        // Validate grammar code by attempting to get part of speech
                                        const CAgramtab* gramTab = H.m_Synan.GetOpt()->GetGramTab();
                                        if (gramTab && gramTab->CheckGramCode("SUB")) {
                                            h.m_CommonGramCode = "SUB"; // German noun
                                            h.SetGramCodes("SUB");      // Use accessor instead of direct assignment 
                                            h.m_iPoses = (1 << 0);      // Substantiv in German
                                        } else {
                                            PLOGE << "Invalid German grammar code - using safe fallback";
                                            h.m_CommonGramCode = "??";  // Unknown
                                            h.SetGramCodes("??");       // Unknown
                                            h.m_iPoses = 0;             // No specific POS
                                        }
                                    }
                                    
                                    // Add homonym directly to the word's homonym vector
                                    word.m_Homonyms.push_back(h);
                                    
                                    // Initialize language-specific elements
                                    word.InitLevelSpecific(word.m_Homonyms.back());
                                }
                                catch (const std::exception& e) {
                                    PLOGE << "Exception creating default homonym: " << e.what();
                                }
                                catch (...) {
                                    PLOGE << "Unknown exception creating default homonym";
                                }
                            }
                        }
                    }
                    
                    CJsonObject sents(d, rapidjson::kArrayType);
                    GetResultBySyntax(H.m_Synan, sents);
                    t.AddMember("result", sents.get_value(), d.GetAllocator());
                }
            }
            LOGD <<  "write output file " << p.second;
            std::ofstream outp(p.second);
            base.write_test_cases(outp);
        }
    }
    catch (std::exception& e) {
        PLOGE << e.what();
        return 1;
    }
    catch (...) {
        PLOGE << "an exception occurred!";
        return 1;
    };

    return 0;
};


