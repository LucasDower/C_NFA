#include <c_nfa/regex.h>
#include <c_nfa/nfa.h>
#include <c_nfa/bridge.h>

nfa_machine* handle_regex(const regex_t* regex)
{
    switch (regex->type)
    {
        case BLANK:
        {
            nfa_machine* machine = nfa_machine_alloc();
            machine->start_state_index = 0;
            machine->final_states = malloc(sizeof(int));
            machine->final_states[0] = 1;
            machine->final_state_len = 1;
            nfa_machine_add_transition(machine, 0, 1, NFA_EPSILON);
            return machine;
        }
        case CHAR:
        {
            nfa_machine* machine = nfa_machine_alloc();
            machine->start_state_index = 0;
            machine->final_states = malloc(sizeof(int));
            machine->final_states[0] = 1;
            machine->final_state_len = 1;
            nfa_machine_add_transition(machine, 0, 1, regex->data.primitive);
            return machine;
        }
        case UNION:
        {
            nfa_machine* machine_a = handle_regex(regex->data.pair.first);
            nfa_machine* machine_b = handle_regex(regex->data.pair.second);
            nfa_machine* machine_union = nfa_machine_union(machine_a, machine_b);
            nfa_machine_free(machine_a);
            nfa_machine_free(machine_b);
            return machine_union;
        }
        case CONCAT:
        {
            nfa_machine* machine_a = handle_regex(regex->data.pair.first);
            nfa_machine* machine_b = handle_regex(regex->data.pair.second);
            nfa_machine* machine_concat = nfa_machine_concat(machine_a, machine_b);
            nfa_machine_free(machine_a);
            nfa_machine_free(machine_b);
            return machine_concat;
        }
        case STAR:
        {
            nfa_machine* machine_a = handle_regex(regex->data.pair.first);
            nfa_machine* machine_star = nfa_machine_kleene_star(machine_a);
            nfa_machine_free(machine_a);
            return machine_star;
        }
    }
}

nfa_machine* regex_to_nfa(const char* input)
{
    regex_t* regex = regex_parse(input);
    nfa_machine* machine = handle_regex(regex);
    regex_free(regex);
    return machine;
}

int regex_execute(const char* regex, const char* input)
{
    nfa_machine* machine = regex_to_nfa(regex);
    int result = nfa_machine_execute(machine, input);
    nfa_machine_free(machine);
    return result;
}