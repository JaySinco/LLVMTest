#pragma once
#include <map>
#include <set>
#include <vector>
#include "antlr4-runtime.h"

using TokenList = std::vector<size_t>;
using RuleList = std::vector<size_t>;

struct RuleWithStartToken
{
    size_t startTokenIndex;
    size_t ruleIndex;
};

using RuleWithStartTokenList = std::vector<RuleWithStartToken>;

struct CandidateRule
{
    size_t startTokenIndex;
    RuleList ruleList;
};

struct CandidatesCollection
{
    std::map<size_t, TokenList> tokens;
    std::map<size_t, CandidateRule> rules;
};

struct FollowSetWithPath
{
    antlr4::misc::IntervalSet intervals;
    RuleList path;
    TokenList following;
};

struct FollowSetsHolder
{
    std::vector<FollowSetWithPath> sets;
    antlr4::misc::IntervalSet combined;
};

using FollowSetsPerState = std::map<size_t, FollowSetsHolder>;
using RuleEndStatus = std::set<size_t>;

struct PipelineEntry
{
    antlr4::atn::ATNState *state;
    size_t tokenListIndex;
};

class CodeCompletion
{
public:
    CodeCompletion(antlr4::Parser &parser);
    CandidatesCollection collectCandidates(size_t caretTokenIndex,
                                           antlr4::ParserRuleContext *context = nullptr);

    bool showResult = false;
    bool showDebugOutput = false;
    bool debugOutputWithTransitions = false;
    bool showRuleStack = false;
    bool translateRulesTopDown = false;
    std::set<size_t> ignoredTokens;
    std::set<size_t> preferredRules;

private:
    // bool checkPredicate(const antlr4::atn::PredicateTransition &transition);
    // bool translateToRuleIndex(const std::vector<size_t> ruleStack);
    // std::vector<size_t> getFollowingTokens(const antlr4::atn::Transition &initialTransition);
    // std::vector<FollowSetWithPath> determineFollowSets(antlr4::atn::ATNState &start,
    //                                                    antlr4::atn::ATNState &stop);
    // void collectFollowSets(antlr4::atn::ATNState &s, antlr4::atn::ATNState &stopState,
    //                        std::vector<FollowSetWithPath> &followSets,
    //                        std::set<antlr4::atn::ATNState> &seen, std::vector<size_t>
    //                        &ruleStack);
    RuleEndStatus processRule(const antlr4::atn::RuleStartState *startState, size_t tokenListIndex,
                              const RuleWithStartTokenList &callStack, size_t precedence,
                              size_t indentation);
    // std::string generateBaseDescription(antlr4::atn::ATNState &state);
    // void printDescription(const std::string &currentIndent, antlr4::atn::ATNState state,
    //                       const std::string &baseDescription, size_t tokenIndex);
    // void printRuleState(const std::vector<size_t> &stack);

    antlr4::Parser &parser;
    std::vector<antlr4::Token *> tokens;
    std::vector<size_t> precedenceStack;
    size_t tokenStartIndex = 0;
    size_t statesProcessed = 0;
    std::map<size_t, std::map<size_t, RuleEndStatus>> shortcutMap;
    CandidatesCollection candidates;
    static std::map<std::string, FollowSetsPerState> followSetsByATN;
};
