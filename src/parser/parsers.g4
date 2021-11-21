parser grammar parsers;

options {
    tokenVocab=lexers;
    language=Cpp;
}

singleExpression:
    singleExpression '[' singleExpression ']'                         # MemberIndexExpression
    | singleExpression '.' identifierName                             # MemberDotExpression
    | singleExpression '(' expressionSequence? ')'                    # ArgumentsExpression
    | '+' singleExpression                                            # UnaryPlusExpression
    | '-' singleExpression                                            # UnaryMinusExpression
    | '~' singleExpression                                            # BitNotExpression
    | '!' singleExpression                                            # NotExpression
    | <assoc=right> singleExpression '**' singleExpression            # PowerExpression
    | singleExpression ('*' | '/' | '%') singleExpression             # MultiplicativeExpression
    | singleExpression ('+' | '-') singleExpression                   # AdditiveExpression
    | singleExpression ('<<' | '>>' | '>>>') singleExpression         # BitShiftExpression
    | singleExpression ('<' | '>' | '<=' | '>=') singleExpression     # RelationalExpression
    | singleExpression ('==' | '!=' | '===' | '!==') singleExpression # EqualityExpression
    | singleExpression '&' singleExpression                           # BitAndExpression
    | singleExpression '^' singleExpression                           # BitXOrExpression
    | singleExpression '|' singleExpression                           # BitOrExpression
    | singleExpression '&&' singleExpression                          # LogicalAndExpression
    | singleExpression '||' singleExpression                          # LogicalOrExpression
    | singleExpression '?' singleExpression ':' singleExpression      # TernaryExpression
    | Identifier                                                      # IdentifierExpression
    | literal                                                         # LiteralExpression
    | '[' expressionSequence? ']'                                     # ArrayLiteralExpression
    | objectLiteral                                                   # ObjectLiteralExpression
    | '(' singleExpression ')'                                        # ParenthesizedExpression
    ;

expressionSequence: singleExpression (',' singleExpression)*;

propertyAssignment: propertyName ':' singleExpression;

propertyName: identifierName | StringLiteral | numericLiteral;

objectLiteral: '{' (propertyAssignment (',' propertyAssignment)*)? '}';

literal:
    NullLiteral
    | BooleanLiteral
    | StringLiteral
    | numericLiteral
    | bigintLiteral
    ;

numericLiteral:
    DecimalLiteral
    | HexIntegerLiteral
    | OctalIntegerLiteral
    | BinaryIntegerLiteral
    ;

bigintLiteral:
    BigDecimalIntegerLiteral
    | BigHexIntegerLiteral
    | BigOctalIntegerLiteral
    | BigBinaryIntegerLiteral
    ;

identifierName: Identifier | reservedWord;

reservedWord: keyword | NullLiteral | BooleanLiteral;

keyword:
    Break
    | Do
    | Instanceof
    | Typeof
    | Case
    | Else
    | New
    | Var
    | Catch
    | Finally
    | Return
    | Void
    | Continue
    | For
    | Switch
    | While
    | Debugger
    | Function_
    | This
    | With
    | Default
    | If
    | Throw
    | Delete
    | In
    | Try
    | Class
    | Enum
    | Extends
    | Super
    | Const
    | Export
    | Import
    | Implements
    | Let
    | Private
    | Public
    | Interface
    | Package
    | Protected
    | Static
    | Yield
    | Async
    | Await
    | From
    | As
    ;
