lexer grammar TLexer;

options {
    language=Cpp;
}

Def:          'def';
Extern:       'extern';
If:           'if';
Else:         'else';
Plus:         '+';
Minus:        '-';
Star:         '*';
Semi:         ';';
Div:          '/';
LeftParen:    '(';
RightParen:   ')';
Comma:        ',';
Less:         '<';
Greater:      '>';
Assign:       '=';
Equal:        '==';
NotEqual:     '!=';
LessEqual:    '<=';
GreaterEqual: '>=';
Identifier:   NONDIGIT (NONDIGIT | DIGIT)*;
Number:       DIGIT+ ('.' DIGIT+)?;
Whitespace:   [ \t]+               -> skip;
Newline:      ( '\r' '\n'? | '\n') -> skip;

fragment NONDIGIT: [a-zA-Z_];
fragment DIGIT:    [0-9];
