/*
Author: Gavin Witsken
Program: ish - simple shell
File: env.h
*/

/*
From csh man page:

The following metasequences are provided for introducing variable values into the shell input. Except as noted, it is an error to reference a variable which is not set.

$name

${name}
Substitutes the words of the value of variable name, each separated by a blank. Braces insulate name from following characters which would otherwise be part of it. Shell variables have names consisting of letters and digits starting with a letter. The underscore character is considered a letter. If name is not a shell variable, but is set in the environment, then that value is returned (but some of the other forms given below are not available in this case).

*/