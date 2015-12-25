#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED


void OpenProgram(string FileName);

string GetLine(unsigned long Pos);

extern vector<_Token> Tokens;
void PrepareTokensList();

extern unsigned long ln;
extern unsigned long lines;


#endif // LEX_H_INCLUDED
