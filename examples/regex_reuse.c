#include <stdlib.h>
#include <assert.h>

#include <c_nfa/regex.h>
#include <c_nfa/nfa.h>

int main()
{
	nfa_machine* my_machine = regex_to_nfa("(c|a*b)");
	{
		assert(nfa_machine_execute(my_machine, "") == 0);
		assert(nfa_machine_execute(my_machine, "c") == 1);
		assert(nfa_machine_execute(my_machine, "b") == 1);
		assert(nfa_machine_execute(my_machine, "a") == 0);
		assert(nfa_machine_execute(my_machine, "ab") == 1);
		assert(nfa_machine_execute(my_machine, "aaaaaaaaab") == 1);
	}
	nfa_machine_free(my_machine);

	return 0;
}