parser grammar TParser;

options {
	tokenVocab = TLexer;
	language = Cpp;
}

init: '(' value (',' value)* ')' EOF;
value: init | Integer;
