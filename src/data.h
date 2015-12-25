#ifndef DATA_H_INCLUDED
#define DATA_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <algorithm>
using namespace std;



//***************** Lex ***************************
#define UNKNOWN     -1
#define EOL         0
#define NUMBER      1
#define STRING      2
#define CHARACTER   3
#define SYMBOL      4
#define IDENTIFIER  5
struct _Token
{
    string Str;
    int Type;
    long unsigned LineStart;
    long unsigned Pos;      //Token's position in current line
};
extern vector<_Token> Tokens;
extern unsigned long tk;
#define Token       Tokens[tk]
#define NextToken   if (tk<Tokens.size()-1) tk++;
//*************************************************

//***************** Misc **************************
void PrintOutTokens();
//*************************************************
#endif // DATA_H_INCLUDED
