parser grammar TParser;

options {
    tokenVocab = TLexer;
    language = Cpp;
}

prog: stat+;
stat: expr NEWLINE | ID '=' expr NEWLINE | NEWLINE;
expr:
    expr ('*' | '/') expr
    | expr ('+' | '-') expr
    | INT
    | ID
    | '(' expr ')';
