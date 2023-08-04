# C_NFA
*C_NFA* implements [regular expressions](https://en.wikipedia.org/wiki/Regular_expression) and [nondeterministic finite automata](https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton) in C.

### Usage
```c
#include <c_nfa/core.h>

int main(void)
{
    // regex describes the set of binary numbers that are multiples of 3
    int success = regex_execute("(0|(1(01*(00)*0)*1)*)*" /*regex*/, "1111" /*input*/);
    return 0;
}
```
`regex_execute(const char* regex, const char* input)` will convert `regex` into [AST](https://en.wikipedia.org/wiki/Abstract_syntax_tree) and then into an [NFA](https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton) which `input` is then ran through. If you are testing multiple inputs again the same regex then consider cacheing the intermediate NFA machine to avoid regenerating it on each call to `regex_execute`.

**Note**, the regex parser only supports basic regular expression operations such as concatenation, union, and Kleene star. There is no support for features such as wildcards or alternative quantifiers, etc.

```c
#include <c_nfa/core.h>
#include <c_nfa/nfa.h>

#include <assert.h>

int main(void)
{
    nfa_machine* my_machine = regex_to_nfa("(c|a*b)");
	{
		assert(nfa_machine_execute(my_machine, "c") == 1);
		assert(nfa_machine_execute(my_machine, "b") == 1);
		assert(nfa_machine_execute(my_machine, "a") == 0);
		assert(nfa_machine_execute(my_machine, "ab") == 1);
	}
	nfa_machine_free(my_machine);
}
```

This library performs no compile-time optimisations, i.e. the regex is not converted to an equivalent NFA at compile-time but that is achieveable. Additionally, the NFA built from [Thompson's construction](https://en.wikipedia.org/wiki/Thompson%27s_construction) is not optimised to a minimal NFA.

`nfa.h` includes functions for manually building NFAs.

```c
#include <c_nfa/nfa.h>

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
        nfa_machine_add_transition(machine, 0 /*from*/, 1 /*to*/, 'a' /*rule*/);
        nfa_machine_add_transition(machine, 0, 2, 'b');
        nfa_machine_add_transition(machine, 1, 2, C_NFA_EPSILON);
        nfa_machine_add_transition(machine, 2, 1, C_NFA_EPSILON);
        nfa_machine_add_transition(machine, 1, 1, 'a');
        nfa_machine_add_transition(machine, 2, 2, 'b');

        // use
        assert(nfa_machine_execute(machine, "") == 0);
        assert(nfa_machine_execute(machine, "a") == 1);
        assert(nfa_machine_execute(machine, "b") == 1);
        assert(nfa_machine_execute(machine, "ab") == 1);
        assert(nfa_machine_execute(machine, "ba") == 1);
        assert(nfa_machine_execute(machine, "babbababbabbabaaababab") == 1);
    }
    nfa_machine_free(machine);

    return 0;
}
```

Additionally, `regex.h` includes `regex_parse(const char* regex)` that returns the regex AST.
