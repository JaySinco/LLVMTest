lexer grammar TLexer;

options {
    language = Cpp;
}

ID:         [a-zA-Z]+;
INT:        [0-9]+;
NEWLINE:    '\r'? '\n';
WS:         [ \t]+ -> skip;
Plus:       '+';
Minus:      '-';
Star:       '*';
Div:        '/';
LeftParen:  '(';
RightParen: ')';
Assign:     '=';
