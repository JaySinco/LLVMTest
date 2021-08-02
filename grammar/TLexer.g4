lexer grammar TLexer;

options {
	language = Cpp;
}

Comma: ',';
LeftParen: '(';
RightParen: ')';
Integer: [0-9]+;
Whitespace: [ \t\r\n]+ -> skip;
