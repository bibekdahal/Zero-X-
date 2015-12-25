#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

void SkipLine();
void SkipBlock();
void NextStatement();
bool IncomingFunction();
int SetPointerType(int Type, int nPtr);
int ParseType();
void ParseDeclare(VarInfo & var,string InType="");

#endif // COMMON_H_INCLUDED
