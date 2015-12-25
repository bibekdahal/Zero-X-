#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

// Functions to be called globally
void OpenProgram(string FileName);
string GetLine(unsigned long Pos);
void PrepareTokensList();

// Some global vars
extern vector<_Token> Tokens;
extern unsigned long ln;
extern unsigned long lines;


#endif // LEX_H_INCLUDED
