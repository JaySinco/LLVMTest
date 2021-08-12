#include "completion.h"
#include "utils.h"

std::map<std::string, FollowSetsPerState> CodeCompletion::followSetsByATN;

CodeCompletion::CodeCompletion(antlr4::Parser &parser, const std::string &clsName)
    : parser(parser), clsName(clsName)
{
}

CandidatesCollection CodeCompletion::collectCandidates(size_t caretTokenIndex,
                                                       antlr4::ParserRuleContext *context)
{
    this->shortcutMap.clear();
    this->candidates.rules.clear();
    this->candidates.tokens.clear();
    this->statesProcessed = 0;
    this->precedenceStack;
    this->tokens.clear();

    this->tokenStartIndex = context != nullptr ? context->start->getTokenIndex() : 0;
    antlr4::TokenStream *tokenStream = this->parser.getTokenStream();
    size_t index = this->tokenStartIndex;
    LOG(INFO) << "caret=" << caretTokenIndex;
    while (true) {
        antlr4::Token *token = tokenStream->get(index);
        LOG(INFO) << "token[{}]='{}'"_format(index, token->getText());
        if (token->getChannel() == antlr4::Token::DEFAULT_CHANNEL) {
            this->tokens.push_back(token);
            if (token->getTokenIndex() >= caretTokenIndex) {
                break;
            }
        }
        if (token->getType() == antlr4::Token::EOF) {
            break;
        }
        ++index;
    }

    RuleWithStartTokenList callStack;
    size_t startRule = context != nullptr ? context->getRuleIndex() : 0;
    this->processRule(this->parser.getATN().ruleToStartState[startRule], 0, callStack, 0, 0);
    if (this->showResult) {
        LOG(INFO) << "states-processed=" << this->statesProcessed;
        LOG(INFO) << "collected-rules:";
        for (auto &rule: this->candidates.rules) {
            std::string path = "";
            for (auto &token: rule.second.ruleList) {
                path += this->parser.getRuleNames()[token] + " ";
            }
            LOG(INFO) << "    " << this->parser.getRuleNames()[rule.first] << ", path=" << path;
        }
        std::set<std::string> sortedTokens;
        for (auto &token: this->candidates.tokens) {
            std::string value = this->parser.getVocabulary().getDisplayName(token.first);
            for (auto &following: token.second) {
                value += " " + this->parser.getVocabulary().getDisplayName(following);
            }
            sortedTokens.insert(value);
        }
        LOG(INFO) << "collected-tokens:";
        for (auto &symbol: sortedTokens) {
            LOG(INFO) << "    " << symbol;
        }
    }
    return this->candidates;
}

bool CodeCompletion::translateStackToRuleIndex(RuleWithStartTokenList &ruleWithStartTokenList)
{
    if (this->preferredRules.size() == 0) {
        return false;
    }
    if (this->translateRulesTopDown) {
        for (int i = ruleWithStartTokenList.size() - 1; i >= 0; i--) {
            if (this->translateToRuleIndex(i, ruleWithStartTokenList)) {
                return true;
            }
        }
    } else {
        for (int i = 0; i < ruleWithStartTokenList.size(); i++) {
            if (this->translateToRuleIndex(i, ruleWithStartTokenList)) {
                return true;
            }
        }
    }
    return false;
}

bool CodeCompletion::translateToRuleIndex(size_t i, RuleWithStartTokenList &ruleWithStartTokenList)
{
    size_t ruleIndex = ruleWithStartTokenList.at(i).ruleIndex;
    size_t startTokenIndex = ruleWithStartTokenList.at(i).startTokenIndex;
    if (this->preferredRules.find(ruleIndex) != this->preferredRules.end()) {
        RuleWithStartTokenList sliced(ruleWithStartTokenList.begin(),
                                      ruleWithStartTokenList.begin() + i);
        std::vector<size_t> path;
        std::transform(sliced.begin(), sliced.end(), std::back_inserter(path),
                       [](RuleWithStartToken &token) { return token.ruleIndex; });
        bool addNew = true;
        for (const auto &rule: this->candidates.rules) {
            if (rule.first != ruleIndex || rule.second.ruleList.size() != path.size()) {
                continue;
            }
            bool allSame = true;
            for (int i = 0; i < path.size(); ++i) {
                if (path.at(i) != rule.second.ruleList.at(i)) {
                    allSame = false;
                }
            }
            if (allSame) {
                addNew = false;
                break;
            }
        }
        if (addNew) {
            this->candidates.rules[ruleIndex] = {
                startTokenIndex,
                std::move(path),
            };
            if (this->showDebugOutput) {
                LOG(INFO) << "collected=" << this->parser.getRuleNames().at(ruleIndex);
            }
        }
        return true;
    }
    return false;
}

RuleEndStatus CodeCompletion::processRule(antlr4::atn::RuleStartState *startState,
                                          size_t tokenListIndex, RuleWithStartTokenList &callStack,
                                          size_t precedence, size_t indentation)
{
    if (this->shortcutMap.find(startState->ruleIndex) != this->shortcutMap.end()) {
        auto &positionMap = this->shortcutMap.at(startState->ruleIndex);
        if (positionMap.find(tokenListIndex) != positionMap.end()) {
            if (this->showDebugOutput) {
                LOG(INFO) << "take shortcut";
            }
            return positionMap.at(tokenListIndex);
        }
    }
    RuleEndStatus result;
    auto &setsPerState = CodeCompletion::followSetsByATN[this->clsName];
    if (setsPerState.find(startState->stateNumber) == setsPerState.end()) {
        auto &followSets = setsPerState[startState->stateNumber];
        auto stop = this->parser.getATN().ruleToStopState[startState->ruleIndex];
        followSets.sets = this->determineFollowSets(startState, stop);
        antlr4::misc::IntervalSet combined;
        for (const auto &set: followSets.sets) {
            combined.addAll(set.intervals);
        }
        followSets.combined = combined;
    }
    auto followSets = setsPerState.at(startState->stateNumber);
    auto startTokenIndex = this->tokens.at(tokenListIndex)->getTokenIndex();
    callStack.push_back({
        startTokenIndex,
        startState->ruleIndex,
    });
    if (tokenListIndex >= this->tokens.size() - 1) {
        if (this->preferredRules.find(startState->ruleIndex) != this->preferredRules.end()) {
            this->translateStackToRuleIndex(callStack);
        } else {
            for (const auto &set: followSets.sets) {
                RuleWithStartTokenList fullPath = callStack;
                RuleWithStartTokenList followSetPath;
                std::transform(set.path.begin(), set.path.end(), std::back_inserter(followSetPath),
                               [&](size_t p) {
                                   return RuleWithStartToken{startTokenIndex, p};
                               });
                fullPath.insert(fullPath.end(), followSetPath.begin(), followSetPath.end());
                if (!this->translateStackToRuleIndex(fullPath)) {
                    for (const auto &symbol: set.intervals.toList()) {
                        if (this->ignoredTokens.find(symbol) == this->ignoredTokens.end()) {
                            if (this->showDebugOutput) {
                                LOG(INFO) << "collected="
                                          << this->parser.getVocabulary().getDisplayName(symbol);
                            }
                            if (this->candidates.tokens.find(symbol) ==
                                this->candidates.tokens.end()) {
                                this->candidates.tokens[symbol] = set.following;
                            } else {
                                if (this->candidates.tokens.at(symbol) != set.following) {
                                    this->candidates.tokens[symbol] = TokenList{};
                                }
                            }
                        }
                    }
                }
            }
        }
        callStack.pop_back();
        return result;
    } else {
        auto currentSymbol = this->tokens[tokenListIndex]->getType();
        if (!followSets.combined.contains(antlr4::Token::EPSILON) &&
            !followSets.combined.contains(currentSymbol)) {
            callStack.pop_back();
            return result;
        }
    }
    // if (startState.isPrecedenceRule) {
    //     this->precedenceStack.push_back(precedence);
    // }
    std::vector<PipelineEntry> statePipeline;
    statePipeline.push_back({startState, tokenListIndex});
    while (statePipeline.size() > 0) {
        PipelineEntry &currentEntry = statePipeline.back();
        statePipeline.pop_back();
        ++this->statesProcessed;
        size_t currentSymbol = this->tokens[currentEntry.tokenListIndex]->getType();
        bool atCaret = currentEntry.tokenListIndex >= this->tokens.size() - 1;
        if (this->showDebugOutput) {
            // this.printDescription(indentation, currentEntry.state,
            //                       this.generateBaseDescription(currentEntry.state),
            //                       currentEntry.tokenListIndex);
            // if (this.showRuleStack) this.printRuleState(callStack);
        }

        if (currentEntry.state->getStateType() == antlr4::atn::ATNState::RULE_STOP) {
            result.insert(currentEntry.tokenListIndex);
            continue;
        }
        auto &totalTrans = currentEntry.state->transitions;
        for (auto &transition: totalTrans) {
            switch (transition->getSerializationType()) {
                case antlr4::atn::Transition::SerializationType::RULE: {
                    auto ruleTransition = dynamic_cast<antlr4::atn::RuleTransition *>(transition);
                    auto endStatus = this->processRule(
                        dynamic_cast<antlr4::atn::RuleStartState *>(transition->target),
                        currentEntry.tokenListIndex, callStack, ruleTransition->precedence,
                        indentation + 1);
                    for (auto &position: endStatus) {
                        statePipeline.push_back({ruleTransition->followState, position});
                    }
                    break;
                }
                case antlr4::atn::Transition::SerializationType::PREDICATE: {
                    auto predicateTransition =
                        dynamic_cast<antlr4::atn::PredicateTransition *>(transition);
                    if (this->checkPredicate(predicateTransition)) {
                        statePipeline.push_back({transition->target, currentEntry.tokenListIndex});
                    }
                    break;
                }
                case antlr4::atn::Transition::SerializationType::PRECEDENCE: {
                    auto predTransition =
                        dynamic_cast<antlr4::atn::PrecedencePredicateTransition *>(transition);
                    if (predTransition->precedence >=
                        this->precedenceStack[this->precedenceStack.size() - 1])
                        statePipeline.push_back({transition->target, currentEntry.tokenListIndex});

                    break;
                }
                case antlr4::atn::Transition::SerializationType::WILDCARD: {
                    if (atCaret) {
                        if (!this->translateStackToRuleIndex(callStack)) {
                            for (auto &token:
                                 antlr4::misc::IntervalSet::of(antlr4::Token::MIN_USER_TOKEN_TYPE,
                                                               this->parser.getATN().maxTokenType)
                                     .toList()) {
                                if (this->ignoredTokens.find(token) == this->ignoredTokens.end()) {
                                    this->candidates.tokens[token] = TokenList{};
                                }
                            }
                        }
                    } else {
                        statePipeline.push_back(
                            {transition->target, currentEntry.tokenListIndex + 1});
                    }
                    break;
                }

                default: {
                    if (transition->isEpsilon()) {
                        statePipeline.push_back({transition->target, currentEntry.tokenListIndex});
                        continue;
                    }
                    auto &set = transition->label();
                    if (set.size() > 0) {
                        if (transition->getSerializationType() ==
                            antlr4::atn::Transition::SerializationType::NOT_SET) {
                            set = set.complement(
                                antlr4::misc::IntervalSet::of(antlr4::Token::MIN_USER_TOKEN_TYPE,
                                                              this->parser.getATN().maxTokenType));
                        }
                        if (atCaret) {
                            if (!this->translateStackToRuleIndex(callStack)) {
                                auto list = set.toList();
                                bool addFollowing = list.size() == 1;
                                for (auto &symbol: list)
                                    if (this->ignoredTokens.find(symbol) ==
                                        this->ignoredTokens.end()) {
                                        if (this->showDebugOutput) {
                                            LOG(INFO)
                                                << "collected="
                                                << this->parser.getVocabulary().getDisplayName(
                                                       symbol);
                                        }
                                        if (addFollowing) {
                                            this->candidates.tokens[symbol] =
                                                this->getFollowingTokens(transition);
                                        } else {
                                            this->candidates.tokens[symbol] = TokenList{};
                                        }
                                    }
                            }
                        } else {
                            if (set.contains(currentSymbol)) {
                                if (this->showDebugOutput) {
                                    LOG(INFO) << "consumed="
                                              << this->parser.getVocabulary().getDisplayName(
                                                     currentSymbol);
                                }
                                statePipeline.push_back(
                                    {transition->target, currentEntry.tokenListIndex + 1});
                            }
                        }
                    }
                }
            }
        }
    }
    callStack.pop_back();
    // if (startState.isPrecedenceRule) {
    //     this.precedenceStack.pop();
    // }
    this->shortcutMap[startState->ruleIndex][tokenListIndex] = result;
    return result;
}

std::vector<FollowSetWithPath> CodeCompletion::determineFollowSets(antlr4::atn::ATNState *start,
                                                                   antlr4::atn::ATNState *stop)
{
    std::vector<FollowSetWithPath> result;
    std::vector<antlr4::atn::ATNState *> stateStack;
    std::vector<size_t> ruleStack;
    this->collectFollowSets(start, stop, result, stateStack, ruleStack);
    return result;
}

bool CodeCompletion::checkPredicate(antlr4::atn::PredicateTransition *transition)
{
    return transition->getPredicate()->eval(&this->parser, &antlr4::ParserRuleContext::EMPTY);
}

std::vector<size_t> CodeCompletion::getFollowingTokens(antlr4::atn::Transition *initialTransition)
{
    std::vector<size_t> result;
    std::vector<antlr4::atn::ATNState *> pipeline = {initialTransition->target};
    while (pipeline.size() > 0) {
        antlr4::atn::ATNState *state = pipeline.back();
        pipeline.pop_back();
        for (antlr4::atn::Transition *transition: state->transitions) {
            if (transition->getSerializationType() ==
                antlr4::atn::Transition::SerializationType::ATOM) {
                if (!transition->isEpsilon()) {
                    auto list = transition->label().toList();
                    if (list.size() == 1 &&
                        this->ignoredTokens.find(list.at(0)) == this->ignoredTokens.end()) {
                        result.push_back(list.at(0));
                        pipeline.push_back(transition->target);
                    }
                } else {
                    pipeline.push_back(transition->target);
                }
            }
        }
    }
    return result;
}

void CodeCompletion::collectFollowSets(antlr4::atn::ATNState *s, antlr4::atn::ATNState *stopState,
                                       std::vector<FollowSetWithPath> &followSets,
                                       std::vector<antlr4::atn::ATNState *> &stateStack,
                                       std::vector<size_t> &ruleStack)
{
    if (std::find(stateStack.begin(), stateStack.end(), s) != stateStack.end()) {
        return;
    }
    stateStack.push_back(s);
    if (s == stopState || s->getStateType() == antlr4::atn::ATNState::RULE_STOP) {
        FollowSetWithPath set;
        set.intervals = antlr4::misc::IntervalSet::of(antlr4::Token::EPSILON);
        set.path = ruleStack;
        followSets.push_back(std::move(set));
        stateStack.pop_back();
        return;
    }
    for (antlr4::atn::Transition *transition: s->transitions) {
        if (transition->getSerializationType() ==
            antlr4::atn::Transition::SerializationType::RULE) {
            auto ruleTransition = dynamic_cast<antlr4::atn::RuleTransition *>(transition);
            if (std::find(ruleStack.begin(), ruleStack.end(), ruleTransition->target->ruleIndex) !=
                ruleStack.end()) {
                continue;
            }
            ruleStack.push_back(ruleTransition->target->ruleIndex);
            this->collectFollowSets(transition->target, stopState, followSets, stateStack,
                                    ruleStack);
            ruleStack.pop_back();
        } else if (transition->getSerializationType() ==
                   antlr4::atn::Transition::SerializationType::PREDICATE) {
            auto predicateTransition = dynamic_cast<antlr4::atn::PredicateTransition *>(transition);
            if (this->checkPredicate(predicateTransition)) {
                this->collectFollowSets(predicateTransition->target, stopState, followSets,
                                        stateStack, ruleStack);
            }
        } else if (transition->isEpsilon()) {
            this->collectFollowSets(transition->target, stopState, followSets, stateStack,
                                    ruleStack);
        } else if (transition->getSerializationType() ==
                   antlr4::atn::Transition::SerializationType::WILDCARD) {
            FollowSetWithPath set;
            set.intervals = antlr4::misc::IntervalSet::of(antlr4::Token::MIN_USER_TOKEN_TYPE,
                                                          this->parser.getATN().maxTokenType);
            set.path = ruleStack;
            followSets.push_back(std::move(set));
        } else {
            auto label = transition->label();
            if (label.size() > 0) {
                if (transition->getSerializationType() ==
                    antlr4::atn::Transition::SerializationType::NOT_SET) {
                    label = label.complement(antlr4::misc::IntervalSet::of(
                        antlr4::Token::MIN_USER_TOKEN_TYPE, this->parser.getATN().maxTokenType));
                }
                FollowSetWithPath set;
                set.intervals = label;
                set.path = ruleStack;
                set.following = this->getFollowingTokens(transition);
                followSets.push_back(std::move(set));
            }
        }
    }
    stateStack.pop_back();
}
