#ifndef STDINC_H_INCLUDED
#define STDINC_H_INCLUDED

#include "data.h"

// Display error message 'err', for error occurring
// at line with it first character's pos as 'LineStart'
// and at pos 'Pos' in that line
void Error(string err, long unsigned LineStart, long Pos=-1);
void Error(string err);
// Str<->Num functions
string ToStr(double Num);
string ToStr(int Num);
string ToStr(long Num);
double ToNum(string str);

#endif // STDINC_H_INCLUDED
