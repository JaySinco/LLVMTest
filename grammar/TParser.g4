parser grammar TParser;

options {
    tokenVocab=TLexer;
    language=Cpp;
}

prog: stat*;

stat: ';' | expr ';' | Identifier '=' expr;

expr:
    Constant
    | Identifier
    | ('!' | '+' | '-') expr
    | expr ('*' | '/' | '%') expr
    | expr ('+' | '-') expr
    | expr ('<' | '>' | '<=' | '>=' | '!=' | '==') expr
    | expr '&&' expr
    | expr '||' expr
    ;
