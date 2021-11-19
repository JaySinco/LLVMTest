lexer grammar lexers;

options {
    language=Cpp;
}

Def:          'def';
Extern:       'extern';
Plus:         '+';
Minus:        '-';
Star:         '*';
Div:          '/';
Semi:         ';';
LeftParen:    '(';
RightParen:   ')';
Comma:        ',';
Question:     '?';
Colon:        ':';
Less:         '<';
Greater:      '>';
Equal:        '==';
NotEqual:     '!=';
LessEqual:    '<=';
GreaterEqual: '>=';
Identifier:   NONDIGIT (NONDIGIT | DIGIT)*;
Number:       DIGIT+ ('.' DIGIT*)? | '.' DIGIT+;
Whitespace:   [ \t]+               -> skip;
Newline:      ( '\r' '\n'? | '\n') -> skip;

fragment NONDIGIT: [a-zA-Z_];
fragment DIGIT:    [0-9];
