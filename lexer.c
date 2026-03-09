#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// Token type enum
typedef enum
{
    // LITERALS
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_FLOAT,
    TOK_STRING,

    // KEYWORDS
    TOK_VAR,
    TOK_CONST,
    TOK_FUNC,
    TOK_RETURN,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_BREAK,
    TOK_IMPORT,
    TOK_STRUCT,
    TOK_ENUM,
    TOK_CASE,
    TOK_WAIT,
    TOK_REPEAT,
    TOK_DO,
    TOK_WAITUNTIL,
    TOK_CONTINUE,
    TOK_EXTERN,
    TOK_FOR,
    TOK_GOTO,
    TOK_REGISTER,
    TOK_ARRAY,
    TOK_SWITCH,
    TOK_VOID,
    TOK_START,
    TOK_TYPEDEF,

    // OPERATORS
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_POWER,

    // EXTRA
    TOK_INC,
    TOK_DEC,
    TOK_QMARK,

    // POINTER
    TOK_POINT_STAR,
    TOK_POINT_AND,

    // ASSIGNMENT
    TOK_ASSIGN,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,
    TOK_STAR_ASSIGN,
    TOK_SLASH_ASSIGN,
    TOK_PCNT_ASSIGN,

    // COMPARISON
    TOK_EQ_EQ,
    TOK_NOT_EQ,
    TOK_LESSTHAN,
    TOK_GREATERTHAN,
    TOK_LEQ,
    TOK_GEQ,

    // LOGICAL
    TOK_AND,
    TOK_OR,
    TOK_NOT,

    // BITWISE
    TOK_BIT_AND,
    TOK_BIT_OR,
    TOK_BIT_XOR,
    TOK_BIT_NOT,
    TOK_BIT_LSHIFT,
    TOK_BIT_RSHIFT,

    // PUNCTUATION
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_COMMA,
    TOK_DOT,
    TOK_COLON,
    TOK_ARROW,
    TOK_BACKSLASH,

    // SPECIAL
    TOK_COMMENT,
    TOK_NEWLINE,
    TOK_EOF
} TokenType;

// Token struct
typedef struct
{
    TokenType type;
    const char *start;
    uint32_t length;
} Token;

// Lexer state
typedef struct
{
    const char *cur;
    const char *end;
} Lexer;

// Initialize lexer
void lexer_init(Lexer *l, const char *src, size_t len)
{
    l->cur = src;
    l->end = src + len;
}

// Create a token
Token make_token(Lexer *l, TokenType type, const char *start)
{
    Token t;
    t.type = type;
    t.start = start;
    t.length = (uint32_t)(l->cur - start);
    return t;
}

// Helper to check if string is a keyword
TokenType check_keyword(const char *start, uint32_t length)
{
    if (length == 3 && strncmp(start, "for", 3) == 0)
        return TOK_FOR;
    if (length == 2 && strncmp(start, "if", 2) == 0)
        return TOK_IF;
    if (length == 4 && strncmp(start, "else", 4) == 0)
        return TOK_ELSE;
    if (length == 5 && strncmp(start, "while", 5) == 0)
        return TOK_WHILE;
    if (length == 3 && strncmp(start, "var", 3) == 0)
        return TOK_VAR;
    if (length == 5 && strncmp(start, "const", 5) == 0)
        return TOK_CONST;
    if (length == 4 && strncmp(start, "func", 4) == 0)
        return TOK_FUNC;
    if (length == 6 && strncmp(start, "return", 6) == 0)
        return TOK_RETURN;
    if (length == 6 && strncmp(start, "import", 6) == 0)
        return TOK_IMPORT;
    if (length == 6 && strncmp(start, "struct", 6) == 0)
        return TOK_STRUCT;
    if (length == 4 && strncmp(start, "enum", 4) == 0)
        return TOK_ENUM;
    if (length == 4 && strncmp(start, "case", 4) == 0)
        return TOK_CASE;
    if (length == 4 && strncmp(start, "wait", 4) == 0)
        return TOK_WAIT;
    if (length == 6 && strncmp(start, "repeat", 6) == 0)
        return TOK_REPEAT;
    if (length == 2 && strncmp(start, "do", 2) == 0)
        return TOK_DO;
    if (length == 9 && strncmp(start, "waituntil", 9) == 0)
        return TOK_WAITUNTIL;
    if (length == 8 && strncmp(start, "continue", 8) == 0)
        return TOK_CONTINUE;
    if (length == 6 && strncmp(start, "extern", 6) == 0)
        return TOK_EXTERN;
    if (length == 4 && strncmp(start, "goto", 4) == 0)
        return TOK_GOTO;
    if (length == 8 && strncmp(start, "register", 8) == 0)
        return TOK_REGISTER;
    if (length == 5 && strncmp(start, "array", 5) == 0)
        return TOK_ARRAY;
    if (length == 6 && strncmp(start, "switch", 6) == 0)
        return TOK_SWITCH;
    if (length == 4 && strncmp(start, "void", 4) == 0)
        return TOK_VOID;
    if (length == 5 && strncmp(start, "start", 5) == 0)
        return TOK_START;
    if (length == 7 && strncmp(start, "typedef", 7) == 0)
        return TOK_TYPEDEF;

    return TOK_IDENTIFIER;
}

// Scan identifier or keyword
Token lex_identifier(Lexer *l)
{
    const char *start = l->cur;
    while (l->cur < l->end && (isalnum(*l->cur) || *l->cur == '_'))
        l->cur++;
    TokenType type = check_keyword(start, (uint32_t)(l->cur - start));
    return make_token(l, type, start);
}

// Scan number (integer or float)
Token lex_number(Lexer *l)
{
    const char *start = l->cur;
    while (l->cur < l->end && isdigit(*l->cur))
        l->cur++;

    if (l->cur < l->end && *l->cur == '.')
    { // float
        l->cur++;
        while (l->cur < l->end && isdigit(*l->cur))
            l->cur++;
        return make_token(l, TOK_FLOAT, start);
    }

    return make_token(l, TOK_NUMBER, start);
}

// Scan string literal
Token lex_string(Lexer *l)
{
    const char *start = l->cur;
    l->cur++; // skip opening quote
    while (l->cur < l->end && *l->cur != '"')
        l->cur++;
    if (l->cur < l->end)
        l->cur++; // skip closing quote
    return make_token(l, TOK_STRING, start);
}

// Scan comment (single-line)
Token lex_comment(Lexer *l)
{
    const char *start = l->cur;
    l->cur += 2; // skip //
    while (l->cur < l->end && *l->cur != '\n')
        l->cur++;
    return make_token(l, TOK_COMMENT, start);
}

// Skip whitespace
void skip_whitespace(Lexer *l)
{
    while (l->cur < l->end && (*l->cur == ' ' || *l->cur == '\t' || *l->cur == '\r'))
        l->cur++;
}

// Get next token
Token lexer_next(Lexer *l)
{
    skip_whitespace(l);
    if (l->cur >= l->end)
        return make_token(l, TOK_EOF, l->cur);

    const char *start = l->cur;
    char c = *l->cur++;

    switch (c)
    {
    case '+':
        if (*l->cur == '+')
        {
            l->cur++;
            return make_token(l, TOK_INC, start);
        }
        if (*l->cur == '=')
        {
            l->cur++;
            return make_token(l, TOK_PLUS_ASSIGN, start);
        }
        return make_token(l, TOK_PLUS, start);
    case '-':
        if (*l->cur == '-')
        {
            l->cur++;
            return make_token(l, TOK_DEC, start);
        }
        if (*l->cur == '=')
        {
            l->cur++;
            return make_token(l, TOK_MINUS_ASSIGN, start);
        }
        if (*l->cur == '>')
        {
            l->cur++;
            return make_token(l, TOK_ARROW, start);
        }
        return make_token(l, TOK_MINUS, start);
    case '*':
        if (*l->cur == '=')
        {
            l->cur++;
            return make_token(l, TOK_STAR_ASSIGN, start);
        }
        return make_token(l, TOK_STAR, start);
    case '/':
        if (*l->cur == '/')
            return lex_comment(l);
        if (*l->cur == '=')
        {
            l->cur++;
            return make_token(l, TOK_SLASH_ASSIGN, start);
        }
        return make_token(l, TOK_SLASH, start);
    case '%':
        if (*l->cur == '=')
        {
            l->cur++;
            return make_token(l, TOK_PCNT_ASSIGN, start);
        }
        return make_token(l, TOK_PERCENT, start);
    case '^':
        return make_token(l, TOK_POWER, start);
    case '&':
        return make_token(l, TOK_BIT_AND, start);
    case '|':
        return make_token(l, TOK_BIT_OR, start);
    case '~':
        return make_token(l, TOK_BIT_NOT, start);
    case '!':
        if (*l->cur == '=')
        {
            l->cur++;
            return make_token(l, TOK_NOT_EQ, start);
        }
        return make_token(l, TOK_NOT, start);
    case '=':
        if (*l->cur == '=')
        {
            l->cur++;
            return make_token(l, TOK_EQ_EQ, start);
        }
        return make_token(l, TOK_ASSIGN, start);
    case '<':
        if (*l->cur == '=')
        {
            l->cur++;
            return make_token(l, TOK_LEQ, start);
        }
        if (*l->cur == '<')
        {
            l->cur++;
            return make_token(l, TOK_BIT_LSHIFT, start);
        }
        return make_token(l, TOK_LESSTHAN, start);
    case '>':
        if (*l->cur == '=')
        {
            l->cur++;
            return make_token(l, TOK_GEQ, start);
        }
        if (*l->cur == '>')
        {
            l->cur++;
            return make_token(l, TOK_BIT_RSHIFT, start);
        }
        return make_token(l, TOK_GREATERTHAN, start);
    case '?':
        return make_token(l, TOK_QMARK, start);
    case '(':
        return make_token(l, TOK_LPAREN, start);
    case ')':
        return make_token(l, TOK_RPAREN, start);
    case '{':
        return make_token(l, TOK_LBRACE, start);
    case '}':
        return make_token(l, TOK_RBRACE, start);
    case '[':
        return make_token(l, TOK_LBRACKET, start);
    case ']':
        return make_token(l, TOK_RBRACKET, start);
    case ',':
        return make_token(l, TOK_COMMA, start);
    case '.':
        return make_token(l, TOK_DOT, start);
    case ':':
        return make_token(l, TOK_COLON, start);
    case '\\':
        return make_token(l, TOK_BACKSLASH, start);
    case '\n':
        return make_token(l, TOK_NEWLINE, start);
    case '"':
        return lex_string(l);
    default:
        if (isalpha(c) || c == '_')
            return lex_identifier(l);
        if (isdigit(c))
            return lex_number(l);
        break;
    }

    // fallback to identifier
    return make_token(l, TOK_IDENTIFIER, start);
}

// For testing
void print_token(Token t)
{
    printf("%.*s : %d\n", t.length, t.start, t.type);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s file\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f)
    {
        perror("fopen");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *src = malloc(len + 1);
    fread(src, 1, len, f);
    src[len] = '\0';
    fclose(f);

    Lexer l;
    lexer_init(&l, src, len);

    Token t;
    do
    {
        t = lexer_next(&l);
        print_token(t);
    } while (t.type != TOK_EOF);

    free(src);
    return 0;
}