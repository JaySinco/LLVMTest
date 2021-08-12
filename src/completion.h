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
    CodeCompletion(antlr4::Parser &parser, const std::string &clsName);

    CandidatesCollection collectCandidates(size_t caretTokenIndex,
                                           antlr4::ParserRuleContext *context = nullptr);

    bool showResult = true;
    bool showDebugOutput = true;
    bool debugOutputWithTransitions = true;
    bool showRuleStack = true;
    bool translateRulesTopDown = false;
    std::set<size_t> ignoredTokens;
    std::set<size_t> preferredRules;

private:
    bool checkPredicate(antlr4::atn::PredicateTransition *transition);

    bool translateStackToRuleIndex(RuleWithStartTokenList &ruleWithStartTokenList);

    bool translateToRuleIndex(size_t i, RuleWithStartTokenList &ruleWithStartTokenList);

    std::vector<size_t> getFollowingTokens(antlr4::atn::Transition *initialTransition);

    std::vector<FollowSetWithPath> determineFollowSets(antlr4::atn::ATNState *start,
                                                       antlr4::atn::ATNState *stop);

    void collectFollowSets(antlr4::atn::ATNState *s, antlr4::atn::ATNState *stopState,
                           std::vector<FollowSetWithPath> &followSets,
                           std::vector<antlr4::atn::ATNState *> &stateStack,
                           std::vector<size_t> &ruleStack);

    RuleEndStatus processRule(antlr4::atn::RuleStartState *startState, size_t tokenListIndex,
                              RuleWithStartTokenList &callStack, size_t precedence,
                              size_t indentation);

    // std::string generateBaseDescription(antlr4::atn::ATNState &state);
    // void printDescription(const std::string &currentIndent, antlr4::atn::ATNState state,
    //                       const std::string &baseDescription, size_t tokenIndex);
    // void printRuleState( std::vector<size_t> &stack);

    antlr4::Parser &parser;
    std::string clsName;
    std::vector<antlr4::Token *> tokens;
    std::vector<size_t> precedenceStack;
    size_t tokenStartIndex = 0;
    size_t statesProcessed = 0;
    std::map<size_t, std::map<size_t, RuleEndStatus>> shortcutMap;
    CandidatesCollection candidates;
    static std::map<std::string, FollowSetsPerState> followSetsByATN;
};
