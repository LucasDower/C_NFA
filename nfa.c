#include "nfa.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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