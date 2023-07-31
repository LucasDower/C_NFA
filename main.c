#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <assert.h>

#define EPSILON 0

typedef struct
{
	size_t from_state_index;
	size_t to_state_index;
	char rule; // = 0 denotes epsilon rule
} nfa_transition;

typedef struct
{
	int start_state_index;
	int* final_states;
	size_t final_state_len;
	nfa_transition* transitions;
	size_t transitions_len;
} nfa_machine;

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

typedef struct {
	size_t current_state;
	size_t current_string_index;
} nfa_machine_execution_context;

typedef struct {
	nfa_machine_execution_context* context;
	size_t context_len;
} nfa_machine_execution_stack;

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

	nfa_machine_execution_context* top = stack->context + stack->context_len;
	top->current_state = current_state;
	top->current_string_index = current_string_index;

	++stack->context_len;
	printf("Pushed (%llu, %llu), size = %llu\n", current_state, current_string_index, stack->context_len);
	//debug_print_stack(stack);
}

int nfa_machine_execution_stack_pop(nfa_machine_execution_stack* stack, nfa_machine_execution_context* top)
{
	if (stack->context_len == 0)
	{
		printf("Stack is empty\n");
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

// need to handle infinite epsilons being added
int nfa_machine_execute(const nfa_machine* machine, const char* string)
{
	nfa_machine_execution_stack* stack = nfa_machine_execution_stack_alloc();
	nfa_machine_execution_stack_push(stack, machine->start_state_index, 0);

	nfa_machine_execution_context top;
	while(nfa_machine_execution_stack_pop(stack, &top))
	{
		printf("Context: (%llu, %llu)\n", top.current_state, top.current_string_index);

		// add all outgoing epsilon transitions
		for (size_t transition_index = 0; transition_index < machine->transitions_len; ++transition_index)
		{
			const nfa_transition* transition = &machine->transitions[transition_index];

			if (transition->from_state_index == top.current_state)
			{
				if (transition->rule == EPSILON)
				{
					// we can take this transition
					printf("Taking EPSILON transition (%llu -> %llu)\n", transition->from_state_index, transition->to_state_index);
					nfa_machine_execution_stack_push(stack, transition->to_state_index, top.current_string_index);
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
						printf("Taking '%c' transition (%llu -> %llu)\n", transition->rule, transition->from_state_index, transition->to_state_index);
						nfa_machine_execution_stack_push(stack, transition->to_state_index, top.current_string_index + 1);
					}
				}
			}
		}
	}

	// we've exhausted all routes, the string doesn't pass
	free(stack);
	return 0;
}

int main()
{
	/*
	nfa_machine* machine = nfa_machine_alloc();
	machine->start_state_index = 0;
	machine->final_states = malloc(sizeof(int));
	*(machine->final_states) = 2;
	machine->final_state_len = 1;

	nfa_machine_add_transition(machine, 0, 2, 'a');
	nfa_machine_add_transition(machine, 0, 1, EPSILON);
	nfa_machine_add_transition(machine, 1, 1, 'b');
	nfa_machine_add_transition(machine, 1, 2, EPSILON);

	assert(nfa_machine_execute(machine, "a") == 1);
	assert(nfa_machine_execute(machine, "") == 1);
	assert(nfa_machine_execute(machine, "b") == 1);
	assert(nfa_machine_execute(machine, "bb") == 1);
	assert(nfa_machine_execute(machine, "bbb") == 1);
	assert(nfa_machine_execute(machine, "c") == 0);
	assert(nfa_machine_execute(machine, "bbc") == 0);
	assert(nfa_machine_execute(machine, "bba") == 0);
	*/

	/*
	nfa_machine* machine = nfa_machine_alloc();
	machine->start_state_index = 0;
	machine->final_states = malloc(sizeof(int));
	*(machine->final_states) = 1;
	machine->final_state_len = 1;

	nfa_machine_add_transition(machine, 0, 1, 'a');
	nfa_machine_add_transition(machine, 1, 0, 'b');

	assert(nfa_machine_execute(machine, "") == 0);
	assert(nfa_machine_execute(machine, "a") == 1);
	assert(nfa_machine_execute(machine, "b") == 0);
	assert(nfa_machine_execute(machine, "aba") == 1);
	assert(nfa_machine_execute(machine, "abab") == 0);
	*/

	nfa_machine* machine = nfa_machine_alloc();
	machine->start_state_index = 0;
	machine->final_states = malloc(sizeof(int));
	*(machine->final_states) = 1;
	machine->final_state_len = 1;

	nfa_machine_add_transition(machine, 0, 1, 'a');
	nfa_machine_add_transition(machine, 0, 2, 'b');
	nfa_machine_add_transition(machine, 1, 2, EPSILON);
	nfa_machine_add_transition(machine, 2, 1, EPSILON);
	nfa_machine_add_transition(machine, 1, 1, 'a');
	nfa_machine_add_transition(machine, 2, 2, 'b');

	assert(nfa_machine_execute(machine, "") == 0);
	assert(nfa_machine_execute(machine, "a") == 1);
	assert(nfa_machine_execute(machine, "b") == 1);
	assert(nfa_machine_execute(machine, "az") == 0);

	/*
		need to update execute so that we keep track of all the epsilon transitions taken
		and the current context when they were taken, then in the future each time we think
		about taking an epsilon transition we first make sure we haven't been in the same context
		in the past and tried this transition before, this will make the loop finite
	*/

	return 0;
}