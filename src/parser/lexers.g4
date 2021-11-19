lexer grammar lexers;

options {
    language=Cpp;
}

MultiLineComment:  '/*' .*? '*/'             -> channel(HIDDEN);
SingleLineComment: '//' ~[\r\n\u2028\u2029]* -> channel(HIDDEN);

OpenBracket:                '[';
CloseBracket:               ']';
OpenParen:                  '(';
CloseParen:                 ')';
OpenBrace:                  '{';
CloseBrace:                 '}';
SemiColon:                  ';';
Comma:                      ',';
Assign:                     '=';
QuestionMark:               '?';
Colon:                      ':';
Ellipsis:                   '...';
Dot:                        '.';
PlusPlus:                   '++';
MinusMinus:                 '--';
Plus:                       '+';
Minus:                      '-';
BitNot:                     '~';
Not:                        '!';
Multiply:                   '*';
Divide:                     '/';
Modulus:                    '%';
Power:                      '**';
Hashtag:                    '#';
RightShiftArithmetic:       '>>';
LeftShiftArithmetic:        '<<';
RightShiftLogical:          '>>>';
LessThan:                   '<';
MoreThan:                   '>';
LessThanEquals:             '<=';
GreaterThanEquals:          '>=';
Equals_:                    '==';
NotEquals:                  '!=';
IdentityEquals:             '===';
IdentityNotEquals:          '!==';
BitAnd:                     '&';
BitXOr:                     '^';
BitOr:                      '|';
And:                        '&&';
Or:                         '||';
MultiplyAssign:             '*=';
DivideAssign:               '/=';
ModulusAssign:              '%=';
PlusAssign:                 '+=';
MinusAssign:                '-=';
LeftShiftArithmeticAssign:  '<<=';
RightShiftArithmeticAssign: '>>=';
RightShiftLogicalAssign:    '>>>=';
BitAndAssign:               '&=';
BitXorAssign:               '^=';
BitOrAssign:                '|=';
PowerAssign:                '**=';
ARROW:                      '=>';

/// Null Literals

NullLiteral: 'null';

/// Boolean Literals

BooleanLiteral: 'true' | 'false';

/// Numeric Literals

DecimalLiteral:
    DecimalIntegerLiteral '.' [0-9] [0-9_]* ExponentPart?
    | '.' [0-9] [0-9_]* ExponentPart?
    | DecimalIntegerLiteral ExponentPart?
    ;

/// Numeric Literals

HexIntegerLiteral:    '0' [xX] [0-9a-fA-F] HexDigit*;
OctalIntegerLiteral:  '0' [oO] [0-7] [_0-7]*;
BinaryIntegerLiteral: '0' [bB] [01] [_01]*;

BigHexIntegerLiteral:     '0' [xX] [0-9a-fA-F] HexDigit* 'n';
BigOctalIntegerLiteral:   '0' [oO] [0-7] [_0-7]* 'n';
BigBinaryIntegerLiteral:  '0' [bB] [01] [_01]* 'n';
BigDecimalIntegerLiteral: DecimalIntegerLiteral 'n';

/// Keywords

Break:      'break';
Do:         'do';
Instanceof: 'instanceof';
Typeof:     'typeof';
Case:       'case';
Else:       'else';
New:        'new';
Var:        'var';
Catch:      'catch';
Finally:    'finally';
Return:     'return';
Void:       'void';
Continue:   'continue';
For:        'for';
Switch:     'switch';
While:      'while';
Debugger:   'debugger';
Function_:  'function';
This:       'this';
With:       'with';
Default:    'default';
If:         'if';
Throw:      'throw';
Delete:     'delete';
In:         'in';
Try:        'try';
As:         'as';
From:       'from';

/// Future Reserved Words

Class:   'class';
Enum:    'enum';
Extends: 'extends';
Super:   'super';
Const:   'const';
Export:  'export';
Import:  'import';
Async:   'async';
Await:   'await';

/// The following tokens are also considered to be FutureReservedWords / when parsing strict mode

Implements: 'implements';
Let:        'let';
Private:    'private';
Public:     'public';
Interface:  'interface';
Package:    'package';
Protected:  'protected';
Static:     'static';
Yield:      'yield';

/// Identifier Names and Identifiers

Identifier: IdentifierStart IdentifierPart*;

/// String Literals
StringLiteral: (
        '"' DoubleStringCharacter* '"'
        | '\'' SingleStringCharacter* '\''
    )
    ;

WhiteSpaces: [\t\u000B\u000C\u0020\u00A0]+ -> channel(HIDDEN);

LineTerminator: [\r\n\u2028\u2029] -> channel(HIDDEN);

// Fragment rules

fragment DoubleStringCharacter:
    ~["\\\r\n]
    | '\\' EscapeSequence
    | LineContinuation
    ;

fragment SingleStringCharacter:
    ~['\\\r\n]
    | '\\' EscapeSequence
    | LineContinuation
    ;

fragment EscapeSequence:
    CharacterEscapeSequence
    | '0' // no digit ahead! TODO
    | HexEscapeSequence
    | UnicodeEscapeSequence
    | ExtendedUnicodeEscapeSequence
    ;

fragment CharacterEscapeSequence:
    SingleEscapeCharacter
    | NonEscapeCharacter
    ;

fragment HexEscapeSequence: 'x' HexDigit HexDigit;

fragment UnicodeEscapeSequence:
    'u' HexDigit HexDigit HexDigit HexDigit
    | 'u' '{' HexDigit HexDigit+ '}'
    ;

fragment ExtendedUnicodeEscapeSequence: 'u' '{' HexDigit+ '}';

fragment SingleEscapeCharacter: ['"\\bfnrtv];

fragment NonEscapeCharacter: ~['"\\bfnrtv0-9xu\r\n];

fragment EscapeCharacter: SingleEscapeCharacter | [0-9] | [xu];

fragment LineContinuation: '\\' [\r\n\u2028\u2029];

fragment HexDigit: [_0-9a-fA-F];

fragment DecimalIntegerLiteral: '0' | [1-9] [0-9_]*;

fragment ExponentPart: [eE] [+-]? [0-9_]+;

fragment IdentifierPart:
    IdentifierStart
    | [\p{Mn}]
    | [\p{Nd}]
    | [\p{Pc}]
    | '\u200C'
    | '\u200D'
    ;

fragment IdentifierStart: [\p{L}] | [$_] | '\\' UnicodeEscapeSequence;
