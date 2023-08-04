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
	size_t final_state_len; // TODO: rename final_states_len
	nfa_transition* transitions; // TODO: sort via from_state_index and bin_search into transitions when searching for transitions to take
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

// Returns the union of two NFAs, i.e. adds a initial state with an e-transition to the initial states of machine_a and machine_b
nfa_machine* nfa_machine_union(const nfa_machine* machine_a, const nfa_machine* machine_b);

// Returns the concatenation of two NFAs, i.e. the final state(s) of machine_a is piped into the initial state of machine_b
nfa_machine* nfa_machine_concat(const nfa_machine* machine_a, const nfa_machine* machine_b);

// Returns the Kleene star of an NFA, i.e. a new NFA where you can take machine 0 times or any number of times
nfa_machine* nfa_machine_kleene_star(const nfa_machine* machine);

// Returns a NFA equivalent to the given regex, only supports concatenation, union, and kleene star
nfa_machine* nfa_machine_construct(const char* regex);

void nfa_machine_dump(const nfa_machine* machine);

#endif