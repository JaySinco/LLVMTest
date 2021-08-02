parser grammar TParser;

options { tokenVocab=TLexer; language=Cpp; }

expression
    : left=DEC_DIGIT PLUS  right=DEC_DIGIT
    | left=DEC_DIGIT MINUS right=DEC_DIGIT
    ;
