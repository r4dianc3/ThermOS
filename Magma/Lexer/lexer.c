#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// Fast ASCII character classification macros used by the lexer
// These operate on plain chars and assume ASCII encoding.
#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z')) // letter
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')                                 // digit
#define IS_ALNUM(c) (IS_ALPHA(c) || IS_DIGIT(c))                               // letter or digit

// TokenType defines all possible token kinds the lexer can produce.
typedef enum TokenTypeEnum
{
    // LITERALS: identifiers, numbers, strings
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_FLOAT,
    TOK_STRING,

    // KEYWORDS: reserved words with special meaning
    TOK_VAR,   // VARIABLE
    TOK_CONST, // CONSTANT
    TOK_FUNC,  // FUNCTION
    TOK_RETURN,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_BREAK, // BREAK LOOP
    TOK_IMPORT,
    TOK_STRUCT,
    TOK_ENUM, // ENUMERATION
    TOK_CASE,
    TOK_WAIT,
    TOK_REPEAT,
    TOK_DO, // DO-WHILE loop
    TOK_WAITUNTIL,
    TOK_CONTINUE,
    TOK_EXTERN, // EXTERNAL FUNCTION
    TOK_FOR,
    TOK_GOTO,
    TOK_REGISTER,
    TOK_ARRAY,
    TOK_SWITCH,
    TOK_VOID,
    TOK_START,
    TOK_TYPEDEF,
    TOK_GLOBAL,
    TOK_PRINTT, // PRINT TO CONSOLE
    TOK_LIB, // IMPORT LIBRARY

    // OPERATORS: arithmetic and other symbols
    TOK_PLUS,    // +
    TOK_MINUS,   // -
    TOK_STAR,    // *
    TOK_SLASH,   // /
    TOK_PERCENT, // %
    TOK_POWER,   // **

    // EXTRA: increment, decrement, question mark
    TOK_INC,   // ++
    TOK_DEC,   // --
    TOK_QMARK, // ?

    // POINTER: pointer operators
    TOK_POINT_STAR, // @
    TOK_POINT_AND,  // `

    // ASSIGNMENT: various compound assignment operators
    TOK_ASSIGN,       // =
    TOK_PLUS_ASSIGN,  // +=
    TOK_MINUS_ASSIGN, // -=
    TOK_STAR_ASSIGN,  // *=
    TOK_SLASH_ASSIGN, // /=
    TOK_PCNT_ASSIGN,  // %=

    /* COMPARISON operators */
    TOK_EQ_EQ,       // ==
    TOK_NOT_EQ,      // !=
    TOK_LESSTHAN,    // <
    TOK_GREATERTHAN, // >
    TOK_LEQ,         // <=
    TOK_GEQ,         // >=

    /* LOGICAL operators */
    TOK_AND, // &&
    TOK_OR,  // ||
    TOK_NOT, // !

    /* BITWISE operators */
    TOK_BIT_AND,    // &
    TOK_BIT_OR,     // |
    TOK_BIT_XOR,    // ^
    TOK_BIT_NOT,    // ~
    TOK_BIT_LSHIFT, // <<
    TOK_BIT_RSHIFT, // >>

    /* PUNCTUATION */
    TOK_LPAREN,    // (
    TOK_RPAREN,    // )
    TOK_LBRACE,    // {
    TOK_RBRACE,    // }
    TOK_LBRACKET,  // [
    TOK_RBRACKET,  // ]
    TOK_COMMA,     // ,
    TOK_DOT,       // .
    TOK_COLON,     // :
    TOK_ARROW,     // ->
    TOK_BACKSLASH, // \

    /* SPECIAL tokens */
    TOK_COMMENT, // //
    TOK_NEWLINE, // \n
    TOK_EOF      // End of file
} TokenType;

typedef struct OperatorStruct
{
    const char *text;
    uint8_t length;
    TokenType type;
} Operator;

static Operator operators[] = {
    {"+=", 2, TOK_PLUS_ASSIGN},
    {"++", 2, TOK_INC},
    {"+", 1, TOK_PLUS},

    {"-=", 2, TOK_MINUS_ASSIGN},
    {"--", 2, TOK_DEC},
    {"->", 2, TOK_ARROW},
    {"-", 1, TOK_MINUS},

    {"*=", 2, TOK_STAR_ASSIGN},
    {"*", 1, TOK_STAR},

    {"/=", 2, TOK_SLASH_ASSIGN},
    {"/", 1, TOK_SLASH},

    {"%=", 2, TOK_PCNT_ASSIGN},
    {"%", 1, TOK_PERCENT},

    {"==", 2, TOK_EQ_EQ},
    {"=", 1, TOK_ASSIGN},

    {"!=", 2, TOK_NOT_EQ},
    {"!", 1, TOK_NOT},

    {"<=", 2, TOK_LEQ},
    {"<<", 2, TOK_BIT_LSHIFT},
    {"<", 1, TOK_LESSTHAN},

    {">=", 2, TOK_GEQ},
    {">>", 2, TOK_BIT_RSHIFT},
    {">", 1, TOK_GREATERTHAN},

    {"&&", 2, TOK_AND},
    {"||", 2, TOK_OR},

    {"&", 1, TOK_BIT_AND},
    {"|", 1, TOK_BIT_OR},
    {"^", 1, TOK_BIT_XOR},
    {"~", 1, TOK_BIT_NOT},

    {"?", 1, TOK_QMARK},

    {NULL, 0, 0}
};

// Representation of a single token produced by the lexer
// `type` indicates the kind of token, `start` points into the source
// buffer at the first character of the token, and `length` stores how many
// characters long the token is.
typedef struct TokenStruct
{
    TokenType type;
    const char *start;
    uint32_t length;
} Token;

typedef struct TokenStreamStruct
{
    Token* tokens;
    int count;
    int capacity;
} TokenStream;

// Lexer holds the state required while scanning. `current` points to the
// next character to read, and `end` marks one past the last character of the
// input buffer.
typedef struct LexerStateStruct
{
    const char *current;
    const char *end;
} Lexer;

// Initialize lexer state with source buffer and length.
// Sets current pointer to beginning and marks end.
void lexer_init(Lexer *lexer, const char *source, size_t length)
{
    lexer->current = source;
    lexer->end = source + length;
}

// Construct a Token given the lexer, type, and starting position.
// The length is computed from the difference between the current pointer
// and the start pointer.
static inline Token make_token(Lexer *lexer, TokenType type, const char *start)
{
    Token currentTOK;
    currentTOK.type = type;
    currentTOK.start = start;
    currentTOK.length = (uint32_t)(lexer->current - start);
    return currentTOK;
}

void tokenstream_add(TokenStream* ts, Token token) {
    if (ts->count >= ts->capacity) {
        ts->capacity *= 2;
        ts->tokens = realloc(ts->tokens, sizeof(Token) * ts->capacity);
    }

    ts->tokens[ts->count++] = token;
}

void tokenstream_init(TokenStream* ts)
{
    ts->count = 0;
    ts->capacity = 64;
    ts->tokens = malloc(sizeof(Token) * ts->capacity);
}

void tokenstream_free(TokenStream* ts)
{
    free(ts->tokens);
    ts->tokens = NULL;
    ts->count = 0;
    ts->capacity = 0;
}

void lexer_scan_all(Lexer* lexer, TokenStream* ts)
{
    for (;;)
    {
        Token tok = lexer_next(lexer);
        tokenstream_add(ts, tok);

        if (tok.type == TOK_EOF)
            break;
    }
}

// Check whether the identifier starting at `start` with given `length`
// matches one of the language keywords. If so, return the corresponding
// keyword token; otherwise fall back to TOK_IDENTIFIER.
TokenType check_keyword(const char *start, uint32_t length)
{

    switch (start[0])
    {

    case 'f':
        if (length == 3 && memcmp(start, "for", 3) == 0)
            return TOK_FOR; // FOR
        if (length == 4 && memcmp(start, "func", 4) == 0)
            return TOK_FUNC; // FUNCTION
        break;

    case 'i':
        if (length == 2 && memcmp(start, "if", 2) == 0)
            return TOK_IF; // IF
        if (length == 6 && memcmp(start, "import", 6) == 0)
            return TOK_IMPORT; // IMPORT
        break;

    case 'e':
        if (length == 4 && memcmp(start, "else", 4) == 0)
            return TOK_ELSE; // ELSE
        if (length == 4 && memcmp(start, "enum", 4) == 0)
            return TOK_ENUM; // ENUMERATION
        if (length == 6 && memcmp(start, "extern", 6) == 0)
            return TOK_EXTERN; // EXTERNAL FUNCTION
        break;

    case 'w':
        if (length == 5 && memcmp(start, "while", 5) == 0)
            return TOK_WHILE; // WHILE
        if (length == 4 && memcmp(start, "wait", 4) == 0)
            return TOK_WAIT; // WAIT
        if (length == 9 && memcmp(start, "waituntil", 9) == 0)
            return TOK_WAITUNTIL; // WAIT UNTIL
        break;

    case 'v':
        if (length == 3 && memcmp(start, "var", 3) == 0)
            return TOK_VAR; // VARIABLE
        if (length == 4 && memcmp(start, "void", 4) == 0)
            return TOK_VOID; // VOID
        break;

    case 'c':
        if (length == 5 && memcmp(start, "const", 5) == 0)
            return TOK_CONST; // CONSTANT
        if (length == 4 && memcmp(start, "case", 4) == 0)
            return TOK_CASE; // CASE
        if (length == 8 && memcmp(start, "continue", 8) == 0)
            return TOK_CONTINUE; // CONTINUE
        break;

    case 'r':
        if (length == 6 && memcmp(start, "return", 6) == 0)
            return TOK_RETURN; // RETURN
        if (length == 6 && memcmp(start, "repeat", 6) == 0)
            return TOK_REPEAT; // REPEAT
        if (length == 8 && memcmp(start, "register", 8) == 0)
            return TOK_REGISTER; // REGISTER
        break;

    case 's':
        if (length == 6 && memcmp(start, "struct", 6) == 0)
            return TOK_STRUCT; // STRUCT
        if (length == 6 && memcmp(start, "switch", 6) == 0)
            return TOK_SWITCH; // STRUCT
        if (length == 5 && memcmp(start, "start", 5) == 0)
            return TOK_START; // START
        break;

    case 'd':
        if (length == 2 && memcmp(start, "do", 2) == 0)
            return TOK_DO; // DO WHILE
        break;

    case 'g':
        if (length == 4 && memcmp(start, "goto", 4) == 0)
            return TOK_GOTO; // GOTO
        if (length == 6 && memcmp(start, "global", 6) == 0)
            return TOK_GLOBAL; // GlOBAL
        break;

    case 'a':
        if (length == 5 && memcmp(start, "array", 5) == 0)
            return TOK_ARRAY; // ARRAY
        break;

    case 't':
        if (length == 7 && memcmp(start, "typedef", 7) == 0)
            return TOK_TYPEDEF; // TYPEDEF
        break;

    case 'p':
        if (length == 6 && memcmp(start, "Printt", 6) == 0)
            return TOK_PRINTT; // PRINT
        break;

    case 'l':
        if (length == 3 && memcmp(start, "lib", 3) == 0)
            return TOK_LIB; // IMPORT LIBRARY
    }

    return TOK_IDENTIFIER; // not a keyword
}

// Scan an identifier or keyword from current position. Characters allowed are
// letters, digits, and underscore; returns either TOK_IDENTIFIER or a keyword
// token determined by check_keyword.
Token lex_identifier(Lexer *lexer)
{

    const char *start = lexer->current; // remember where identifier begins

    // consume alphanumeric characters or underscore
    while (lexer->current < lexer->end &&
           (IS_ALNUM(*lexer->current) || *lexer->current == '_'))
        lexer->current++;

    // determine if the scanned text matches a keyword
    TokenType type = check_keyword(start, (uint32_t)(lexer->current - start));
    return make_token(lexer, type, start);
}

// Scan a numeric literal; recognizes integer and floating point by looking for a
// decimal point. Advances lexer current pointer appropriately.
Token lex_number(Lexer *lexer)
{

    const char *start = lexer->current; // start of numeric literal

    // read integer part digits
    while (lexer->current < lexer->end && IS_DIGIT(*lexer->current))
        lexer->current++;

    // check for fractional part
    if (lexer->current < lexer->end && *lexer->current == '.')
    {
        lexer->current++; // consume decimal point

        // read digits after decimal
        while (lexer->current < lexer->end && IS_DIGIT(*lexer->current))
            lexer->current++;

        return make_token(lexer, TOK_FLOAT, start);
    }

    // no decimal point -> integer
    return make_token(lexer, TOK_NUMBER, start);
}

// Scan a double-quoted string literal. Does not currently handle escapes.
Token lex_string(Lexer *lexer)
{

    const char *start = lexer->current; // include opening quote in token

    lexer->current++; // skip past '"'

    // advance until closing quote or end-of-input
    while (lexer->current < lexer->end && *lexer->current != '"')
        lexer->current++;

    if (lexer->current < lexer->end)
        lexer->current++; // consume closing quote

    return make_token(lexer, TOK_STRING, start);
}

// Scan a single-line comment beginning with "//". The returned token includes
// the comment text (excluding the initial slashes).
Token lex_comment(Lexer *lexer)
{

    const char *start = lexer->current; // starting at first '/'

    lexer->current += 2; // skip '//'

    // consume until end of line
    while (lexer->current < lexer->end && *lexer->current != '\n')
        lexer->current++;

    return make_token(lexer, TOK_COMMENT, start);
}

// Consume whitespace characters (space, tab, carriage return) but stop on
// anything else including newline, which is treated as a token.
void skip_whitespace(Lexer *lexer)
{

    while (lexer->current < lexer->end)
    {

        char c = *lexer->current;

        // skip space, tab, and carriage return only; newline is significant
        if (c == ' ' || c == '\t' || c == '\r')
            lexer->current++;
        else
            break;
    }
}

Token lex_operator(Lexer *lexer)
{
    const char *start = lexer->current;

    for (int i = 0; operators[i].text; i++)
    {

        if ((lexer->end - start) >= operators[1].length &&
            strncmp(start, operators[i].text, operators[1].length) == 0)
        {
            lexer->current += operators[1].length;
            return make_token(lexer, operators[i].type, start);
        }
    }

    lexer->current++;
    return make_token(lexer, TOK_IDENTIFIER, start);
}

// Return the next token from the input stream. Skips whitespace and handles
// all single-character tokens as well as multi-character operators, literals,
// identifiers, numbers, strings, and comments.
Token lexer_next(Lexer *lexer)
{
    skip_whitespace(lexer); // SKIP WHITESPACE

    if (lexer->current >= lexer->end)
        return make_token(lexer, TOK_EOF, lexer->current); // END OF FILE

    char currentchar = *lexer->current;

    if (IS_ALPHA(currentchar) || currentchar == '_') // IDENTIFIER
        return lex_identifier(lexer);

    if (IS_DIGIT(currentchar)) // NUMBER
        return lex_number(lexer);

    if (currentchar == '"') // STRING
        return lex_string(lexer);

    if ((lexer->end - lexer->current) > 1 && currentchar == '/' && lexer->current[1] == '/') // COMMENT
        return lex_comment(lexer);

    switch (*lexer->current)
    {
    case '(':
        lexer->current++;
        return make_token(lexer, TOK_LPAREN, lexer->current - 1); // (

    case ')':
        lexer->current++;
        return make_token(lexer, TOK_RPAREN, lexer->current - 1); // )

    case '{':
        lexer->current++;
        return make_token(lexer, TOK_LBRACE, lexer->current - 1); // {

    case '}':
        lexer->current++;
        return make_token(lexer, TOK_RBRACE, lexer->current - 1); // }

    case '[':
        lexer->current++;
        return make_token(lexer, TOK_LBRACKET, lexer->current - 1); // [

    case ']':
        lexer->current++;
        return make_token(lexer, TOK_RBRACKET, lexer->current - 1); // ]

    case ',':
        lexer->current++;
        return make_token(lexer, TOK_COMMA, lexer->current - 1); // ,

    case '.':
        lexer->current++;
        return make_token(lexer, TOK_DOT, lexer->current - 1); // .

    case ':':
        lexer->current++;
        return make_token(lexer, TOK_COLON, lexer->current - 1); // :

    case '\\':
        lexer->current++;
        return make_token(lexer, TOK_BACKSLASH, lexer->current - 1); // '\'

    case '@':
        lexer->current++;
        return make_token(lexer, TOK_POINT_STAR, lexer->current - 1); // @

    case '`':
        lexer->current++;
        return make_token(lexer, TOK_POINT_AND, lexer->current - 1); // `
    }

    if (*lexer->current == '\n')
    {
        lexer->current++;
        return make_token(lexer, TOK_NEWLINE, lexer->current - 1); // NEWLINE
    }

    return lex_operator(lexer); // OPERATOR
} 