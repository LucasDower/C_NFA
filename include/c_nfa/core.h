#ifndef C_NFA_CORE_H
#define C_NFA_CORE_H

// forward declare;
struct nfa_machine;

struct nfa_machine* regex_to_nfa(const char* input);

int regex_execute(const char* regex, const char* input);

#endif