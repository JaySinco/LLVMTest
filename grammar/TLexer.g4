lexer grammar TLexer;

options {
    language=Cpp;
}

CLEAR:      'clear';
ID:         [a-zA-Z]+;
INT:        [0-9]+;
NEWLINE:    '\r'? '\n';
Plus:       '+';
Minus:      '-';
Mul:        '*';
Div:        '/';
LeftParen:  '(';
RightParen: ')';
Assign:     '=';
WS:         [ \t]+ -> skip;
