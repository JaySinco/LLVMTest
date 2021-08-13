parser grammar TParser;

options {
    tokenVocab=TLexer;
    language=Cpp;
}

program: statement* EOF;

statement:
    expression Semi                         # expressionStatement
    | Extern functionSignature Semi         # externalFunction
    | Def functionSignature expression Semi # functionDefinition
    ;

expression:
    Number                                                              # literalExpression
    | Identifier                                                        # idExpression
    | Identifier LeftParen expressionList? RightParen                   # callExpression
    | LeftParen expression RightParen                                   # parenthesesExpression
    | expression (Star | Div) expression                                # multiplicativeExpression
    | expression (Plus | Minus) expression                              # additiveExpression
    | expression (Less | Greater | LessEqual | GreaterEqual) expression # relationalExpression
    | expression (Equal | NotEqual) expression                          # equalityExpression
    | <assoc=right> expression Question expression Colon expression     # conditionalExpression
    ;

expressionList: expression (Comma expression)*;

argumentList: Identifier (Comma Identifier)*;

functionSignature: Identifier LeftParen argumentList? RightParen;
