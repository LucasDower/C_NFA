# C_NFA
*C_NFA* implements [nondeterministic finite automata](https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton) in C. This library lets you construct NFAs and test them against inputs.

### Example
```c
#include "nfa.h"

#include <stdlib.h>
#include <assert.h>

int main(void)
{
    nfa_machine* machine = nfa_machine_alloc();
    {
        // setup
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

        // use
        assert(nfa_machine_execute(machine, "") == 0);
        assert(nfa_machine_execute(machine, "a") == 1);
        assert(nfa_machine_execute(machine, "b") == 1);
        assert(nfa_machine_execute(machine, "ab") == 1);
        assert(nfa_machine_execute(machine, "ba") == 1);
        assert(nfa_machine_execute(machine, "ba") == 1);
        assert(nfa_machine_execute(machine, "bababababa") == 1);
        assert(nfa_machine_execute(machine, "babbababbabbabaaababab") == 1);
    }
    nfa_machine_free(machine);

    return 0;
}
```
