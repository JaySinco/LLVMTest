/** C 2011 grammar built from the C11 Spec */
parser grammar parsers;

options {
    tokenVocab=lexers;
    language=Cpp;
}

primaryExpression:
    Identifier
    | Constant
    | StringLiteral+
    | '(' expression ')'
    | genericSelection
    | '__extension__'? '(' compoundStatement ')' // Blocks (GCC extension)
    | '__builtin_va_arg' '(' unaryExpression ',' typeName ')'
    | '__builtin_offsetof' '(' typeName ',' unaryExpression ')'
    ;

genericSelection:
    '_Generic' '(' assignmentExpression ',' genericAssocList ')'
    ;

genericAssocList: genericAssociation (',' genericAssociation)*;

genericAssociation: (typeName | 'default') ':' assignmentExpression;

postfixExpression:
    (
        primaryExpression
        | '__extension__'? '(' typeName ')' '{' initializerList ','? '}'
    ) (
        '[' expression ']'
        | '(' argumentExpressionList? ')'
        | ('.' | '->') Identifier
        | ('++' | '--')
    )*
    ;

argumentExpressionList:
    assignmentExpression (',' assignmentExpression)*
    ;

unaryExpression:
    ('++' | '--' | 'sizeof')* (
        postfixExpression
        | unaryOperator castExpression
        | ('sizeof' | '_Alignof') '(' typeName ')'
        | '&&' Identifier // GCC extension address of label
    )
    ;

unaryOperator: '&' | '*' | '+' | '-' | '~' | '!';

castExpression:
    '__extension__'? '(' typeName ')' castExpression
    | unaryExpression
    | DigitSequence // for
    ;

multiplicativeExpression:
    castExpression (('*' | '/' | '%') castExpression)*
    ;

additiveExpression:
    multiplicativeExpression (('+' | '-') multiplicativeExpression)*
    ;

shiftExpression:
    additiveExpression (('<<' | '>>') additiveExpression)*
    ;

relationalExpression:
    shiftExpression (('<' | '>' | '<=' | '>=') shiftExpression)*
    ;

equalityExpression:
    relationalExpression (('==' | '!=') relationalExpression)*
    ;

andExpression: equalityExpression ( '&' equalityExpression)*;

exclusiveOrExpression: andExpression ('^' andExpression)*;

inclusiveOrExpression:
    exclusiveOrExpression ('|' exclusiveOrExpression)*
    ;

logicalAndExpression:
    inclusiveOrExpression ('&&' inclusiveOrExpression)*
    ;

logicalOrExpression: logicalAndExpression ( '||' logicalAndExpression)*;

conditionalExpression:
    logicalOrExpression ('?' expression ':' conditionalExpression)?
    ;

assignmentExpression:
    conditionalExpression
    | unaryExpression assignmentOperator assignmentExpression
    | DigitSequence // for
    ;

assignmentOperator:
    '='
    | '*='
    | '/='
    | '%='
    | '+='
    | '-='
    | '<<='
    | '>>='
    | '&='
    | '^='
    | '|='
    ;

expression: assignmentExpression (',' assignmentExpression)*;

constantExpression: conditionalExpression;

declaration:
    declarationSpecifiers initDeclaratorList? ';'
    | staticAssertDeclaration
    ;

declarationSpecifiers: declarationSpecifier+;

declarationSpecifiers2: declarationSpecifier+;

declarationSpecifier:
    storageClassSpecifier
    | typeSpecifier
    | typeQualifier
    | functionSpecifier
    | alignmentSpecifier
    ;

initDeclaratorList: initDeclarator (',' initDeclarator)*;

initDeclarator: declarator ('=' initializer)?;

storageClassSpecifier:
    'typedef'
    | 'extern'
    | 'static'
    | '_Thread_local'
    | 'auto'
    | 'register'
    ;

typeSpecifier: (
        'void'
        | 'char'
        | 'short'
        | 'int'
        | 'long'
        | 'float'
        | 'double'
        | 'signed'
        | 'unsigned'
        | '_Bool'
        | '_Complex'
        | '__m128'
        | '__m128d'
        | '__m128i'
    )
    | '__extension__' '(' ('__m128' | '__m128d' | '__m128i') ')'
    | atomicTypeSpecifier
    | structOrUnionSpecifier
    | enumSpecifier
    | typedefName
    | '__typeof__' '(' constantExpression ')' // GCC extension
    | typeSpecifier pointer
    ;

structOrUnionSpecifier:
    structOrUnion Identifier? '{' structDeclarationList '}'
    | structOrUnion Identifier
    ;

structOrUnion: 'struct' | 'union';

structDeclarationList: structDeclaration+;

structDeclaration:
    specifierQualifierList structDeclaratorList? ';'
    | staticAssertDeclaration
    ;

specifierQualifierList: (typeSpecifier | typeQualifier) specifierQualifierList?
    ;

structDeclaratorList: structDeclarator (',' structDeclarator)*;

structDeclarator: declarator | declarator? ':' constantExpression;

enumSpecifier:
    'enum' Identifier? '{' enumeratorList ','? '}'
    | 'enum' Identifier
    ;

enumeratorList: enumerator (',' enumerator)*;

enumerator: enumerationConstant ('=' constantExpression)?;

enumerationConstant: Identifier;

atomicTypeSpecifier: '_Atomic' '(' typeName ')';

typeQualifier: 'const' | 'restrict' | 'volatile' | '_Atomic';

functionSpecifier: (
        'inline'
        | '_Noreturn'
        | '__inline__' // GCC extension
        | '__stdcall'
    )
    | gccAttributeSpecifier
    | '__declspec' '(' Identifier ')'
    ;

alignmentSpecifier: '_Alignas' '(' (typeName | constantExpression) ')';

declarator: pointer? directDeclarator gccDeclaratorExtension*;

directDeclarator:
    Identifier
    | '(' declarator ')'
    | directDeclarator '[' typeQualifierList? assignmentExpression? ']'
    | directDeclarator '[' 'static' typeQualifierList? assignmentExpression ']'
    | directDeclarator '[' typeQualifierList 'static' assignmentExpression ']'
    | directDeclarator '[' typeQualifierList? '*' ']'
    | directDeclarator '(' parameterTypeList ')'
    | directDeclarator '(' identifierList? ')'
    | Identifier ':' DigitSequence // bit field
    | '(' typeSpecifier? pointer directDeclarator ')' // function pointer like: (__cdecl *f)
    ;

gccDeclaratorExtension:
    '__asm' '(' StringLiteral+ ')'
    | gccAttributeSpecifier
    ;

gccAttributeSpecifier:
    '__attribute__' '(' '(' gccAttributeList ')' ')'
    ;

gccAttributeList: gccAttribute? (',' gccAttribute?)*;

gccAttribute:
    ~(',' | '(' | ')') // relaxed def for "identifier or reserved word"
    ('(' argumentExpressionList? ')')?
    ;

nestedParenthesesBlock: (~('(' | ')') | '(' nestedParenthesesBlock ')')*
    ;

pointer: (('*' | '^') typeQualifierList?)+ // ^ - Blocks language extension
    ;

typeQualifierList: typeQualifier+;

parameterTypeList: parameterList (',' '...')?;

parameterList: parameterDeclaration (',' parameterDeclaration)*;

parameterDeclaration:
    declarationSpecifiers declarator
    | declarationSpecifiers2 abstractDeclarator?
    ;

identifierList: Identifier (',' Identifier)*;

typeName: specifierQualifierList abstractDeclarator?;

abstractDeclarator:
    pointer
    | pointer? directAbstractDeclarator gccDeclaratorExtension*
    ;

directAbstractDeclarator:
    '(' abstractDeclarator ')' gccDeclaratorExtension*
    | '[' typeQualifierList? assignmentExpression? ']'
    | '[' 'static' typeQualifierList? assignmentExpression ']'
    | '[' typeQualifierList 'static' assignmentExpression ']'
    | '[' '*' ']'
    | '(' parameterTypeList? ')' gccDeclaratorExtension*
    | directAbstractDeclarator '[' typeQualifierList? assignmentExpression? ']'
    | directAbstractDeclarator '[' 'static' typeQualifierList? assignmentExpression ']'
    | directAbstractDeclarator '[' typeQualifierList 'static' assignmentExpression ']'
    | directAbstractDeclarator '[' '*' ']'
    | directAbstractDeclarator '(' parameterTypeList? ')' gccDeclaratorExtension*
    ;

typedefName: Identifier;

initializer: assignmentExpression | '{' initializerList ','? '}';

initializerList:
    designation? initializer (',' designation? initializer)*
    ;

designation: designatorList '=';

designatorList: designator+;

designator: '[' constantExpression ']' | '.' Identifier;

staticAssertDeclaration:
    '_Static_assert' '(' constantExpression ',' StringLiteral+ ')' ';'
    ;

statement:
    labeledStatement
    | compoundStatement
    | expressionStatement
    | selectionStatement
    | iterationStatement
    | jumpStatement
    | ('__asm' | '__asm__') ('volatile' | '__volatile__') '(' (
        logicalOrExpression (',' logicalOrExpression)*
    )? (':' (logicalOrExpression (',' logicalOrExpression)*)?)* ')' ';'
    ;

labeledStatement:
    Identifier ':' statement
    | 'case' constantExpression ':' statement
    | 'default' ':' statement
    ;

compoundStatement: '{' blockItemList? '}';

blockItemList: blockItem+;

blockItem: statement | declaration;

expressionStatement: expression? ';';

selectionStatement:
    'if' '(' expression ')' statement ('else' statement)?
    | 'switch' '(' expression ')' statement
    ;

iterationStatement:
    While '(' expression ')' statement
    | Do statement While '(' expression ')' ';'
    | For '(' forCondition ')' statement
    ;

// | 'for' '(' expression? ';' expression? ';' forUpdate? ')' statement | For '(' declaration expression? ';'
// expression? ')' statement

forCondition: (forDeclaration | expression?) ';' forExpression? ';' forExpression?
    ;

forDeclaration: declarationSpecifiers initDeclaratorList?;

forExpression: assignmentExpression (',' assignmentExpression)*;

jumpStatement: (
        'goto' Identifier
        | ('continue' | 'break')
        | 'return' expression?
        | 'goto' unaryExpression // GCC extension
    ) ';'
    ;

compilationUnit: translationUnit? EOF;

translationUnit: externalDeclaration+;

externalDeclaration:
    functionDefinition
    | declaration
    | ';' // stray ;
    ;

functionDefinition:
    declarationSpecifiers? declarator declarationList? compoundStatement
    ;

declarationList: declaration+;
