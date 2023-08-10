#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <c_nfa/core.h>
#include <c_nfa/nfa.h>
#include <c_nfa/regex.h>

int main(void)
{
	assert(regex_execute("abcd", "") == 0);
	assert(regex_execute("abcd", "abcd") == 1);
	assert(regex_execute("abcd", "abcdefg") == 0);

	assert(regex_execute("(a|b)*", "") == 1);
	assert(regex_execute("(a|b)*", "a") == 1);
	assert(regex_execute("(a|b)*", "b") == 1);
	assert(regex_execute("(a|b)*", "aa") == 1);
	assert(regex_execute("(a|b)*", "abab") == 1);
	assert(regex_execute("(a|b)*", "baab") == 1);
	assert(regex_execute("(a|b)*", "bbba") == 1);
	assert(regex_execute("(a|b)*", "c") == 0);
	assert(regex_execute("(a|b)*", "abc") == 0);
	assert(regex_execute("(a|b)*", "ac") == 0);

	assert(regex_execute("", "") == 1);
	assert(regex_execute("", "a") == 0);
}