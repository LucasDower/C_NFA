#include <stdio.h>
#include <stdlib.h>

#include <c_nfa/regex.h>

int main()
{
	// regex describes the set of binary numbers that are multiples of 3
	const char* regex_multiples_of_three = "(0|(1(01*(00)*0)*1)*)*";
	const char* input = "1111";

	if (regex_execute(regex_multiples_of_three, input))
	{
		printf("The input passes the regex!\n");
	}
	else
	{
		printf("The input fails the regex!\n");
	}

	return 0;
}