#ifndef NFA_H
#define NFA_H

#define NFA_EPSILON 0

typedef struct
{
	size_t from_state_index;
	size_t to_state_index;
	char rule;
} nfa_transition;

typedef struct
{
	int start_state_index;
	int* final_states;
	size_t final_state_len;
	nfa_transition* transitions;
	size_t transitions_len;
} nfa_machine;

// Create a new NFA machine
nfa_machine* nfa_machine_alloc();

// Dealloc a NFA machine
void nfa_machine_free(nfa_machine* machine);

// Add a transition to a NFA machine
void nfa_machine_add_transition(nfa_machine* machine, const size_t from_state_index, const size_t to_state_index, const char rule);

// Run some input through the NFA, return 1 if passes, 0 otherwise
int nfa_machine_execute(const nfa_machine* machine, const char* string);

#endif