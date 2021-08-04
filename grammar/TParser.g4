parser grammar TParser;

options {
    tokenVocab=TLexer;
    language=Cpp;
}

prog: stat+;

stat:
    expr NEWLINE          # printExpr
    | ID '=' expr NEWLINE # assign
    | CLEAR NEWLINE       # clear
    | NEWLINE             # blank
    ;

expr:
    expr op=('*' | '/') expr   # MulDiv
    | expr op=('+' | '-') expr # AddSub
    | INT                      # int
    | ID                       # id
    | '(' expr ')'             # parens
    ;
