parser grammar TParser;

options {
    tokenVocab=TLexer;
    language=Cpp;
}

prog: stat*;

stat:
    expr ';'               # ExprStat
    | Extern prototype ';' # FuncDecl
    | Def prototype block  # FuncDef
    ;

expr:
    Number                                              # Num
    | Identifier                                        # ID
    | expr ('*' | '/') expr                             # MulDiv
    | expr ('+' | '-') expr                             # AddSub
    | expr ('<' | '>' | '<=' | '>=' | '!=' | '==') expr # Compare
    | Identifier '(' params? ')'                        # Call
    | If '(' expr ')' block Else block                  # Condition
    | Identifier '=' expr                               # Assign
    ;

block:     '{' stat* '}';
prototype: Identifier '(' args? ')';
args:      Identifier (',' Identifier)*;
params:    expr (',' expr)*;
