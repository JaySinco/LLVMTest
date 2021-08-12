parser grammar TParser;

options {
    tokenVocab=TLexer;
    language=Cpp;
}

prog: assignment | simpleExpression;

assignment: (VAR | LET) ID EQUAL simpleExpression;

simpleExpression:
    simpleExpression (MULTIPLY | DIVIDE) simpleExpression
    | simpleExpression (PLUS | MINUS) simpleExpression
    | variableRef
    | functionRef
    ;

variableRef: ID;
functionRef: ID OPEN_PAR CLOSE_PAR;
