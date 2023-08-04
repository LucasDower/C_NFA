#ifndef BRIDGE_H
#define BRIDGE_H

// forward declare;
struct nfa_machine;

nfa_machine* regex_to_nfa(const char* input);

int regex_execute(const char* regex, const char* input);

#endif