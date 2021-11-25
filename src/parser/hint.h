#pragma once
#include "./expr.h"

namespace hint
{
std::optional<antlr4::tree::ParseTree *> getTreeFromPos(antlr4::tree::ParseTree *root,
                                                        size_t charPosInLine, size_t line);

std::optional<std::string> completion(antlr4::Parser *parser, antlr4::tree::ParseTree *caret);

}  // namespace hint
