#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <assert.h>

#include "nfa.h"


int main()
{
	nfa_machine* machine = nfa_machine_alloc();
	{
		machine->start_state_index = 0;
		machine->final_states = malloc(sizeof(int));
		machine->final_states[0] = 1;
		machine->final_state_len = 1;

		nfa_machine_add_transition(machine, 0, 1, 'a');
		nfa_machine_add_transition(machine, 0, 2, 'b');
		nfa_machine_add_transition(machine, 1, 2, NFA_EPSILON);
		nfa_machine_add_transition(machine, 2, 1, NFA_EPSILON);
		nfa_machine_add_transition(machine, 1, 1, 'a');
		nfa_machine_add_transition(machine, 2, 2, 'b');

		assert(nfa_machine_execute(machine, "az") == 0);
		assert(nfa_machine_execute(machine, "ab") == 1);
		assert(nfa_machine_execute(machine, "aabbabbbababbaaaab") == 1);
	}
	nfa_machine_free(machine);

	return 0;
}