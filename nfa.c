#include "nfa.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
	size_t current_state;
	size_t current_string_index;
} nfa_machine_execution_context;

typedef struct {
	nfa_machine_execution_context* context;
	size_t context_len;
} nfa_machine_execution_stack;

typedef struct
{
	nfa_machine_execution_context* contexts;
	size_t contexts_len;
} nfa_machine_SET_entry;

nfa_machine* nfa_machine_alloc()
{
	nfa_machine* machine = malloc(sizeof(nfa_machine));
	machine->start_state_index = 0;
	machine->final_states = NULL;
	machine->final_state_len = 0;
	machine->transitions = NULL;
	machine->transitions_len = 0;

	return machine;
}

void nfa_machine_free(nfa_machine* machine)
{
	free(machine->final_states);
	free(machine->transitions);
	free(machine);
}

void nfa_machine_add_transition(nfa_machine* machine, const size_t from_state_index, const size_t to_state_index, const char rule)
{
	if (machine->transitions_len == 0)
	{
		machine->transitions = malloc(sizeof(nfa_transition));
	}
	else
	{
		machine->transitions = realloc(machine->transitions, sizeof(nfa_transition) * (machine->transitions_len + 1));
	}

	nfa_transition* new_transition = machine->transitions + machine->transitions_len;
	new_transition->from_state_index = from_state_index;
	new_transition->to_state_index = to_state_index;
	new_transition->rule = rule;

	++machine->transitions_len;
}

nfa_machine_execution_stack* nfa_machine_execution_stack_alloc()
{
	nfa_machine_execution_stack* stack = malloc(sizeof(nfa_machine_execution_stack));
	stack->context = NULL;
	stack->context_len = 0;

	return stack;
}

void debug_print_stack(nfa_machine_execution_stack* stack)
{
	printf("[ ");
	for (size_t index = 0; index < stack->context_len; ++index)
	{
		printf("(%llu, %llu)", stack->context[index].current_state, stack->context[index].current_string_index);
	}
	printf(" ]\n");
}

void nfa_machine_execution_stack_push(nfa_machine_execution_stack* stack, size_t current_state, size_t current_string_index)
{
	if (stack->context_len == 0)
	{
		stack->context = malloc(sizeof(nfa_machine_execution_context));
	}
	else
	{
		assert(stack->context != NULL);
		stack->context = realloc(stack->context, sizeof(nfa_machine_execution_context) * (stack->context_len + 1));
	}

	//nfa_machine_execution_context* top = stack->context + stack->context_len;
	nfa_machine_execution_context* top = &stack->context[stack->context_len];
	top->current_state = current_state;
	top->current_string_index = current_string_index;

	++stack->context_len;
	//printf("Pushed (%llu, %llu), size = %llu\n", current_state, current_string_index, stack->context_len);
	//debug_print_stack(stack);
}

int nfa_machine_execution_stack_pop(nfa_machine_execution_stack* stack, nfa_machine_execution_context* top)
{
	if (stack->context_len == 0)
	{
		//printf("Stack is empty\n");
		return 0;
	}

	nfa_machine_execution_context tmp = stack->context[stack->context_len - 1];
	top->current_state = tmp.current_state;
	top->current_string_index = tmp.current_string_index;

	assert(stack->context != NULL);
	stack->context = realloc(stack->context, sizeof(nfa_machine_execution_context) * (stack->context_len - 1));
	--stack->context_len;

	//printf("Popped (%llu, %llu), size = %llu\n", top->current_state, top->current_string_index, stack->context_len);
	//debug_print_stack(stack);
	return 1;
}

// SET = seen epsilon transitions
nfa_machine_SET_entry* nfa_machine_execution_SET_alloc(const nfa_machine* machine)
{
	nfa_machine_SET_entry* transition_to_seen = malloc(machine->transitions_len * sizeof(nfa_machine_SET_entry));

	for (size_t transition_index = 0; transition_index < machine->transitions_len; ++transition_index)
	{
		transition_to_seen[transition_index].contexts = NULL;
		transition_to_seen[transition_index].contexts_len = 0;
	}

	return transition_to_seen;
}

nfa_machine_SET_entry* nfa_machine_execution_SET_free(const nfa_machine* machine, const nfa_machine_SET_entry* SET_table)
{
	for (size_t transition_index = 0; transition_index < machine->transitions_len; ++transition_index)
	{
		free(SET_table[transition_index].contexts);
	}
	free(SET_table);
}

int nfa_machine_execution_SET_has(const nfa_machine* machine, const nfa_machine_SET_entry* SET_table, size_t transition_index, nfa_machine_execution_context context)
{
	const nfa_machine_SET_entry* entry = &SET_table[transition_index];
	for (size_t entry_index = 0; entry_index < entry->contexts_len; ++entry_index)
	{
		const nfa_machine_execution_context* tmp_context = &entry->contexts[entry_index];
		if (tmp_context->current_state == context.current_state && tmp_context->current_string_index == context.current_string_index)
		{
			return 1;
		}
	}

	return 0;
}

void nfa_machine_execution_SET_add(const nfa_machine* machine, nfa_machine_SET_entry* SET_table, size_t transition_index, nfa_machine_execution_context context)
{
	nfa_machine_SET_entry* entry = &SET_table[transition_index];

	if (entry->contexts_len == 0)
	{
		entry->contexts = malloc(sizeof(nfa_machine_execution_context));
	}
	else
	{
		assert(entry->contexts != NULL);
		entry->contexts = realloc(entry->contexts, sizeof(nfa_machine_execution_context) * (entry->contexts_len + 1));
	}

	nfa_machine_execution_context* top = &entry->contexts[entry->contexts_len];
	top->current_state = context.current_state;
	top->current_string_index = context.current_string_index;

	++entry->contexts_len;
}

void debug_print_SET_table(const nfa_machine* machine, const nfa_machine_SET_entry* SET_table)
{
	printf("[\n");
	for (size_t transition_index = 0; transition_index < machine->transitions_len; ++transition_index)
	{
		printf("\t%llu: [ ", transition_index);
		const nfa_machine_SET_entry* entry = &SET_table[transition_index];
		for (size_t context_index = 0; context_index < entry->contexts_len; ++context_index)
		{
			const nfa_machine_execution_context* context = &entry->contexts[context_index];
			printf("(%llu, %llu)", context->current_state, context->current_string_index);
		}
		printf(" ]\n");
	}
	printf("]\n");
}

// need to handle infinite epsilons being added
int nfa_machine_execute(const nfa_machine* machine, const char* string)
{
	nfa_machine_execution_stack* stack = nfa_machine_execution_stack_alloc();
	nfa_machine_execution_stack_push(stack, machine->start_state_index, 0);

	nfa_machine_SET_entry* SET_table = nfa_machine_execution_SET_alloc(machine);

	nfa_machine_execution_context top;
	while (nfa_machine_execution_stack_pop(stack, &top))
	{
		//printf("Context: (%llu, %llu)\n", top.current_state, top.current_string_index);
		//debug_print_SET_table(machine, SET_table);

		// add all outgoing epsilon transitions
		for (size_t transition_index = 0; transition_index < machine->transitions_len; ++transition_index)
		{
			const nfa_transition* transition = &machine->transitions[transition_index];

			if (transition->from_state_index == top.current_state)
			{
				if (transition->rule == NFA_EPSILON)
				{
					if (!nfa_machine_execution_SET_has(machine, SET_table, transition_index, top))
					{
						// we can take this transition
						//printf("Taking EPSILON transition(%d) (%llu -> %llu)\n", transition_index, transition->from_state_index, transition->to_state_index);
						nfa_machine_execution_stack_push(stack, transition->to_state_index, top.current_string_index);
						nfa_machine_execution_SET_add(machine, SET_table, transition_index, top);
					}
					else
					{
						//printf("Seen EPSILON transition(%d) before (%llu -> %llu)\n", transition_index, transition->from_state_index, transition->to_state_index);
					}
				}
			}
		}

		// are we at the end of the string?
		if (string[top.current_string_index] == '\0')
		{
			// if so, return true if we're in a final state, otherwise false
			for (size_t final_state_index = 0; final_state_index < machine->final_state_len; ++final_state_index)
			{
				if (machine->final_states[final_state_index] == top.current_state)
				{
					// we're at the end of the string and in a final state
					nfa_machine_execution_SET_free(machine, SET_table);
					free(stack);
					return 1;
				}
			}
			// we're at the end of the string but not in a final state, try the next context on the stack
		}
		else
		{
			// otherwise try all transitions and add them to the stack
			for (size_t transition_index = 0; transition_index < machine->transitions_len; ++transition_index)
			{
				const nfa_transition* transition = &machine->transitions[transition_index];

				if (transition->from_state_index == top.current_state)
				{
					if (transition->rule == string[top.current_string_index])
					{
						// we can take this transition
						//printf("Taking '%c' transition(%d) (%llu -> %llu)\n", transition->rule, transition_index, transition->from_state_index, transition->to_state_index);
						nfa_machine_execution_stack_push(stack, transition->to_state_index, top.current_string_index + 1);
					}
				}
			}
		}
	}

	// we've exhausted all routes, the string doesn't pass
	nfa_machine_execution_SET_free(machine, SET_table);
	free(stack);
	return 0;
}

size_t get_machine_max_state_index(const nfa_machine* machine)
{
	size_t machine_max_state_index = machine->start_state_index;

	for (size_t transition_index = 0; transition_index < machine->transitions_len; ++transition_index)
	{
		const nfa_transition* transition = &machine->transitions[transition_index];
		machine_max_state_index = MAX(machine_max_state_index, transition->from_state_index);
		machine_max_state_index = MAX(machine_max_state_index, transition->to_state_index);
	}

	for (size_t final_state_index = 0; final_state_index < machine->final_state_len; ++final_state_index)
	{
		machine_max_state_index = MAX(machine_max_state_index, machine->final_states[final_state_index]);
	}

	return machine_max_state_index;
}

// TODO: Make new unique final state and add transitions from final states of machine_a and machine_b
// instead of adding the final states together
nfa_machine* nfa_machine_union(const nfa_machine* machine_a, const nfa_machine* machine_b)
{
	// we need to offset all the machine_b state indexes because they will overlap with machine_b
	// find the maximum state index from machine_a to choose as the offset
	size_t machine_a_max_state_index = get_machine_max_state_index(machine_a);

	// All states need to be offset by at least 1 because we have a new initial state
	size_t machine_a_state_index_offset = 1;
	size_t machine_b_state_index_offset = machine_a_max_state_index + 1 + 1;

	nfa_machine* machine_union = nfa_machine_alloc();
	machine_union->final_state_len = machine_a->final_state_len + machine_b->final_state_len;
	machine_union->final_states = malloc((machine_union->final_state_len) * sizeof(int));
	{
		// copy in machine_a final states
		memcpy(machine_union->final_states, machine_a->final_states, machine_a->final_state_len * sizeof(int));
		for (size_t index = 0; index < machine_a->final_state_len; ++index)
		{
			machine_union->final_states[index] += machine_a_state_index_offset;
		}

		// copy in machine_b final states
		memcpy(machine_union->final_states + machine_a->final_state_len, machine_b->final_states, machine_b->final_state_len * sizeof(int));
		for (size_t index = 0; index < machine_b->final_state_len; ++index)
		{
			machine_union->final_states[machine_a->final_state_len + index] += machine_b_state_index_offset;
		}
	}
	machine_union->start_state_index = 0;

	// Copy over all transitions
	{
		nfa_machine_add_transition(machine_union, 0, machine_a->start_state_index + machine_a_state_index_offset, NFA_EPSILON);
		nfa_machine_add_transition(machine_union, 0, machine_b->start_state_index + machine_b_state_index_offset, NFA_EPSILON);

		// machine_a
		for (size_t transition_index = 0; transition_index < machine_a->transitions_len; ++transition_index)
		{
			const nfa_transition* transition = &machine_a->transitions[transition_index];
			nfa_machine_add_transition(machine_union, transition->from_state_index + machine_a_state_index_offset, transition->to_state_index + machine_a_state_index_offset, transition->rule);
		}

		// machine_b
		for (size_t transition_index = 0; transition_index < machine_b->transitions_len; ++transition_index)
		{
			const nfa_transition* transition = &machine_b->transitions[transition_index];
			nfa_machine_add_transition(machine_union, transition->from_state_index + machine_b_state_index_offset, transition->to_state_index + machine_b_state_index_offset, transition->rule);
		}
	}

	return machine_union;
}

nfa_machine* nfa_machine_concat(const nfa_machine* machine_a, const nfa_machine* machine_b)
{
	// we need to offset all the machine_b state indexes because they will overlap with machine_b
	// find the maximum state index from machine_a to choose as the offset
	size_t machine_a_max_state_index = get_machine_max_state_index(machine_a);

	// All states need to be offset by at least 1 because we have a new initial state
	size_t machine_b_state_index_offset = machine_a_max_state_index + 1;

	nfa_machine* machine_concat = nfa_machine_alloc();
	machine_concat->start_state_index = machine_a->start_state_index;
	machine_concat->final_state_len = machine_b->final_state_len;
	machine_concat->final_states = malloc(machine_concat->final_state_len * sizeof(int));
	for (size_t index = 0; index < machine_b->final_state_len; ++index)
	{
		machine_concat->final_states[index] = machine_b->final_states[index] + machine_b_state_index_offset;
	}

	// Copy transitions
	{
		// machine_a
		machine_concat->transitions_len = machine_a->transitions_len;
		machine_concat->transitions = malloc(machine_concat->transitions_len * sizeof(nfa_transition));
		memcpy(machine_concat->transitions, machine_a->transitions, machine_a->transitions_len * sizeof(nfa_transition));

		// machine_b
		for (size_t transition_index = 0; transition_index < machine_b->transitions_len; ++transition_index)
		{
			const nfa_transition* transition = &machine_b->transitions[transition_index];
			nfa_machine_add_transition(machine_concat, transition->from_state_index + machine_b_state_index_offset, transition->to_state_index + machine_b_state_index_offset, transition->rule);
		}
	}

	// Add final transition from final states of machine_a to start state of machine_b
	for (size_t state_index = 0; state_index < machine_a->final_state_len; ++state_index)
	{
		nfa_machine_add_transition(machine_concat, machine_a->final_states[state_index], machine_b->start_state_index + machine_b_state_index_offset, NFA_EPSILON);
	}

	return machine_concat;
}

nfa_machine* nfa_machine_kleene_star(const nfa_machine* machine)
{
	// 1. Copy transitions from machine
	nfa_machine* machine_star = nfa_machine_alloc();
	machine_star->transitions_len = machine->transitions_len;
	machine_star->transitions = malloc(machine->transitions_len * sizeof(nfa_transition));
	memcpy(machine_star->transitions, machine->transitions, machine->transitions_len * sizeof(nfa_transition));

	// 2. Set the start state to a new state Q
	size_t machine_max_state_index = get_machine_max_state_index(machine);
	size_t state_index_q = machine_max_state_index + 1;
	machine_star->start_state_index = state_index_q;

	// 3. Add an e-transition from Q to the original start state
	nfa_machine_add_transition(machine_star, state_index_q, machine->start_state_index, NFA_EPSILON);

	// 4. Add a new single final state F
	machine_star->final_states = malloc(sizeof(int));
	size_t state_index_f = machine_max_state_index + 2;
	machine_star->final_states[0] = state_index_f;
	machine_star->final_state_len = 1;

	// 5. Add e-transitions from all original final states to F
	for (size_t index = 0; index < machine->final_state_len; ++index)
	{
		nfa_machine_add_transition(machine_star, machine->final_states[index], state_index_f, NFA_EPSILON);
	}

	// 6. Add a e-transition from Q to F
	nfa_machine_add_transition(machine_star, state_index_q, state_index_f, NFA_EPSILON);

	// 7. Add a e-transitions from original finals to original starts
	// TODO: Can replace this with e-transition from F to Q? Should be equivalent but only adds 1 transition
	for (size_t index = 0; index < machine->final_state_len; ++index)
	{
		nfa_machine_add_transition(machine_star, machine->final_states[index], machine->start_state_index, NFA_EPSILON);
	}

	return machine_star;
}

void nfa_machine_dump(const nfa_machine* machine)
{
	printf("[\n");
	printf("\tStart Index: %d\n", machine->start_state_index);
	printf("\tFinal States: [ ");
	for (size_t index = 0; index < machine->final_state_len; ++index)
	{
		printf("%d", machine->final_states[index]);
		if (index < machine->final_state_len - 1)
		{
			printf(", ");
		}
	}
	printf(" ]\n");
	printf("\tTransitions: [\n");
	//printf("\t\thi\n");
	for (size_t index = 0; index < machine->transitions_len; ++index)
	{
		const nfa_transition* transition = &machine->transitions[index];
		printf("\t\t%llu -- %c --> %llu\n", transition->from_state_index, transition->rule ? transition->rule : ' ', transition->to_state_index);
	}
	printf("\t]\n");
	printf("]\n");
}