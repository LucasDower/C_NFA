#ifndef REGEX_H
#define REGEX_H

typedef enum
{
    BLANK,
    CHAR,
    UNION,
    CONCAT,
    STAR
} regex_type;

typedef struct regex_t
{
    regex_type type;
    union {
        char primitive;
        struct
        {
            struct regex_t* first;
            struct regex_t* second;
        } pair;
    } data;
} regex_t;

regex_t* regex_parse(const char* input);
void regex_free(regex_t* regex);

#endif