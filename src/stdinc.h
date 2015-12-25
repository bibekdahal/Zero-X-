#ifndef STDINC_H_INCLUDED
#define STDINC_H_INCLUDED

#include "data.h"

// Display error message 'err', for error occurring
// at line with it first character's pos as 'LineStart'
// and at pos 'Pos' in that line
void Error(string err, long unsigned LineStart, long Pos=-1);
void Error(string err);

// Change a integer into string
string ToStr(long unsigned Num);

// Change a string into integer
int ToInt(string str);

#endif // STDINC_H_INCLUDED
