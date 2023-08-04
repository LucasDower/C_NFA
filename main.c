#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "nfa.h"
#include "regex.h"
#include "bridge.h"

int main()
{
	nfa_machine* machineA = nfa_machine_alloc();
	{
		// setup
		machineA->start_state_index = 0;
		machineA->final_states = malloc(sizeof(int));
		machineA->final_states[0] = 1;
		machineA->final_state_len = 1;
		nfa_machine_add_transition(machineA, 0, 1, 'a');
		nfa_machine_add_transition(machineA, 0, 2, 'b');
		nfa_machine_add_transition(machineA, 1, 2, NFA_EPSILON);
		nfa_machine_add_transition(machineA, 2, 1, NFA_EPSILON);
		nfa_machine_add_transition(machineA, 1, 1, 'a');
		nfa_machine_add_transition(machineA, 2, 2, 'b');

		// use
		assert(nfa_machine_execute(machineA, "") == 0);
		assert(nfa_machine_execute(machineA, "a") == 1);
		assert(nfa_machine_execute(machineA, "b") == 1);
		assert(nfa_machine_execute(machineA, "ab") == 1);
		assert(nfa_machine_execute(machineA, "ba") == 1);
		assert(nfa_machine_execute(machineA, "ba") == 1);
		assert(nfa_machine_execute(machineA, "bababababa") == 1);
		assert(nfa_machine_execute(machineA, "babbababbabbabaaababab") == 1);
	}


	nfa_machine* machineB = nfa_machine_alloc();
	{
		machineB->start_state_index = 0;
		machineB->final_states = malloc(sizeof(int));
		machineB->final_states[0] = 2;
		machineB->final_state_len = 1;
		nfa_machine_add_transition(machineB, 0, 1, 'c');
		nfa_machine_add_transition(machineB, 1, 2, 'd');

		assert(nfa_machine_execute(machineB, "") == 0);
		assert(nfa_machine_execute(machineB, "c") == 0);
		assert(nfa_machine_execute(machineB, "d") == 0);
		assert(nfa_machine_execute(machineB, "cd") == 1);
		assert(nfa_machine_execute(machineB, "test") == 0);
	}

	nfa_machine* machine_union = nfa_machine_union(machineA, machineB);
	{
		assert(nfa_machine_execute(machine_union, "") == 0);
		assert(nfa_machine_execute(machine_union, "a") == 1);
		assert(nfa_machine_execute(machine_union, "b") == 1);
		assert(nfa_machine_execute(machine_union, "ab") == 1);
		assert(nfa_machine_execute(machine_union, "ba") == 1);
		assert(nfa_machine_execute(machine_union, "ba") == 1);
		assert(nfa_machine_execute(machine_union, "bababababa") == 1);
		assert(nfa_machine_execute(machine_union, "babbababbabbabaaababab") == 1);

		assert(nfa_machine_execute(machine_union, "") == 0);
		assert(nfa_machine_execute(machine_union, "c") == 0);
		assert(nfa_machine_execute(machine_union, "d") == 0);
		assert(nfa_machine_execute(machine_union, "cd") == 1);
		assert(nfa_machine_execute(machine_union, "test") == 0);
	}

	nfa_machine* machine_concat = nfa_machine_concat(machineA, machineB);
	{
		assert(nfa_machine_execute(machine_concat, "abcd") == 1);
		assert(nfa_machine_execute(machine_concat, "babbababbabbabaaabababcd") == 1);
		assert(nfa_machine_execute(machine_concat, "cd") == 0);
	}

	nfa_machine* machine_star = nfa_machine_kleene_star(machineB);
	{
		//nfa_machine_dump(machine_star);
		
		assert(nfa_machine_execute(machine_star, "") == 1);
		assert(nfa_machine_execute(machine_star, "cd") == 1);
		assert(nfa_machine_execute(machine_star, "cdcd") == 1);
		assert(nfa_machine_execute(machine_star, "cdcdcd") == 1);
		assert(nfa_machine_execute(machine_star, "test") == 0);
	}

	nfa_machine_free(machineA);
	nfa_machine_free(machineB);
	nfa_machine_free(machine_union);
	nfa_machine_free(machine_concat);
	nfa_machine_free(machine_star);

	regex_t* regex = regex_parse("(0|(1(01*(00)*0)*1)*)*");
	regex_free(regex);

	nfa_machine* my_machine = regex_to_nfa("(c|a*b)");
	{
		assert(nfa_machine_execute(my_machine, "c") == 1);
		assert(nfa_machine_execute(my_machine, "b") == 1);
		assert(nfa_machine_execute(my_machine, "a") == 0);
		assert(nfa_machine_execute(my_machine, "ab") == 1);
	}
	nfa_machine_dump(my_machine);
	nfa_machine_free(my_machine);

	assert(regex_execute("ab*c", "abbbbbbbc") == 1);

	// regex describes the set of binary numbers that are multiples of 3
	const char* regex_multiples_of_three = "(0|(1(01*(00)*0)*1)*)*";

	assert(regex_execute(regex_multiples_of_three, "1111") == 1);
	assert(regex_execute(regex_multiples_of_three, "10000") == 0);

	return 0;
}