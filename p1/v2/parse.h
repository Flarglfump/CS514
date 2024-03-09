/*
Author: Gavin Witsken
Program: ish - simple shell
File: parse.h
*/


/*
After the input line is aliased and parsed, and before each command is executed, variable substitution is performed keyed by '$' characters. This expansion can be prevented by preceding the '$' with a '\' except within '"'s where it always occurs, and within '''s where it never occurs. Strings quoted by '`' are interpreted later (see Command substitution below) so '$' substitution does not occur there until later, if at all. A '$' is passed unchanged if followed by a blank, tab, or end-of-line.
*/

