#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <c_nfa/regex.h>

typedef struct
{
    const char* input;
    size_t input_len;
    size_t cursor;
} regex_parse_context;

regex_t* regex_parse_regex(regex_parse_context* context);
regex_t* regex_parse_term(regex_parse_context* context);
regex_t* regex_parse_factor(regex_parse_context* context);
regex_t* regex_parse_base(regex_parse_context* context);

char regex_parser_peek(const regex_parse_context* context)
{
    return context->input[context->cursor];
}

void regex_parser_eat(regex_parse_context* context, char c)
{
    if (regex_parser_peek(context) == c)
    {
        ++context->cursor; // TODO: Bounds check
        return;
    }
    fprintf(stderr, "Expected '%c', got '%c'\n", c, regex_parser_peek(context));
    assert(0);
}

char regex_parser_next(regex_parse_context* context)
{
    assert(regex_parser_more(context));

    char c = regex_parser_peek(context);
    regex_parser_eat(context, c);
    return c;
}

int regex_parser_more(regex_parse_context* context)
{
    return context->cursor < context->input_len;
}

// Parsers

regex_t* regex_parse_regex(regex_parse_context* context)
{
    regex_t* term = regex_parse_term(context);

    if (regex_parser_more(context) && regex_parser_peek(context) == '|')
    {
        regex_parser_eat(context, '|');
        regex_t* regex = regex_parse_regex(context);
        regex_t* res = malloc(sizeof(regex_t));
        res->type = UNION;
        res->data.pair.first = term;
        res->data.pair.second = regex;
        return res;
    }
    else
    {
        return term;
    }
}

regex_t* regex_parse_term(regex_parse_context* context)
{
    regex_t* factor = malloc(sizeof(regex_t));
    factor->type = BLANK;

    while (regex_parser_more(context) && regex_parser_peek(context) != ')' && regex_parser_peek(context) != '|')
    {
        regex_t* next_factor = regex_parse_factor(context);
        regex_t* new_factor = malloc(sizeof(regex_t));
        new_factor->type = CONCAT;
        new_factor->data.pair.first = factor;
        new_factor->data.pair.second = next_factor;
        factor = new_factor;
    }

    return factor;
}

regex_t* regex_parse_factor(regex_parse_context* context)
{
    regex_t* base = regex_parse_base(context);

    while (regex_parser_more(context) && regex_parser_peek(context) == '*')
    {
        regex_parser_eat(context, '*');
        regex_t* new_base = malloc(sizeof(regex_t));
        new_base->type = STAR;
        new_base->data.pair.first = base;
        new_base->data.pair.second = NULL;
        base = new_base;
    }

    return base;
}

regex_t* regex_parse_base(regex_parse_context* context)
{
    switch (regex_parser_peek(context))
    {
    case '(':
    {
        regex_parser_eat(context, '(');
        regex_t* r = regex_parse_regex(context);
        regex_parser_eat(context, ')');
        return r;
    }
    case '\\':
    {
        regex_parser_eat(context, '\\');
        char esc = regex_parser_next(context);
        regex_t* r = malloc(sizeof(regex_t));
        r->type = CHAR;
        r->data.primitive = esc;
        return r;
    }
    default:
    {
        char next = regex_parser_next(context);
        regex_t* r = malloc(sizeof(regex_t));
        r->type = CHAR;
        r->data.primitive = next;
        return r;
    }
    }
}

void dump_regex_internal(const regex_t* regex)
{
    switch (regex->type)
    {
        case BLANK:
        {
            //printf("BLANK");
            printf("\'\'");
            break;
        }
        case CHAR:
        {
            //printf("CHAR(%c)", regex->data.primitive);
            printf("%c", regex->data.primitive);
            break;
        }
        case UNION:
        {
            /*
            printf("UNION(");
            dump_regex_internal(regex->data.pair.first);
            printf(",");
            dump_regex_internal(regex->data.pair.second);
            printf(")");
            */
            printf("(");
            dump_regex_internal(regex->data.pair.first);
            printf("|");
            dump_regex_internal(regex->data.pair.second);
            printf(")");
            break;
        }
        case CONCAT:
        {
            printf("(");
            dump_regex_internal(regex->data.pair.first);
            printf(",");
            dump_regex_internal(regex->data.pair.second);
            printf(")");
            break;
        }
        case STAR:
        {
            printf("(");
            dump_regex_internal(regex->data.pair.first);
            printf(")*");
            break;
        }
    }
}

void dump_regex(const regex_t* regex)
{
    dump_regex_internal(regex);
    printf("\n");
}

regex_t* regex_parse(const char* input)
{
    regex_parse_context context = {
        .input = input,
        .input_len = strlen(input),
        .cursor = 0
    };

    return regex_parse_regex(&context);
}

void regex_free(regex_t* regex)
{
    switch (regex->type)
    {
        case UNION:
        case CONCAT:
        {
            regex_free(regex->data.pair.first);
            regex_free(regex->data.pair.second);
            break;
        }
        case STAR:
        {
            regex_free(regex->data.pair.first);
            break;
        }
    }
    free(regex);
}