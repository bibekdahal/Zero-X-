#include "stdinc.h"
#include "lex.h"

string Program="";

long unsigned pos=0;
long unsigned lineStart=0;
long unsigned ln=0;
long unsigned lines=0;

vector<_Token> Tokens;
_Token token;

//  Get the rest of the line from 'Pos' character in the 'Program'
string GetLine(long unsigned Pos)
{
    string ret="";
    //  Get the text till the end of line
    while (Program[Pos]!='\n')
    {
        if (isspace(Program[Pos]))  ret += ' ';  // Change all types of spaces (like tabs) into plain spaces
        else ret+=Program[Pos];
        Pos++;
    }
    return ret;
}

//  Open the file and store the whole code in 'Program'
void OpenProgram(string FileName)
{
    ifstream file(FileName.c_str());
    if (!file.is_open())
        Error("Error:\n\tCan't open file \"" + FileName + "\"\n");

    string tmp="";
    while (getline(file, tmp))
    {
        Program = Program + tmp + "\n";
        lines += 1;
    }
    file.close();

    if (Program.length() == 0)
        Error("Error:\n\tEmpty File\n");
    return;
}

//  To skip the spaces and tabs as well as the comments that the parser won't need
void SkipWhiteSpace()
{
    while (isspace(Program[pos]))
    {
        if (Program[pos]=='\n')
            break;
        pos++;

    }
    if (Program[pos]=='/' && Program[pos+1]=='/')
    {
        while (Program[pos]!='\n')
            pos++;
    }
}

// To get the identifier type of token
int GetIdentifier()
{
    do
    {
        token.Str += toupper(Program[pos]);
        pos++;
    }while (isalnum(Program[pos]) || Program[pos]=='_');
    token.Type = IDENTIFIER;
    return IDENTIFIER;
}

//  To get the number type of token
int GetNumber()
{
    bool dot=false;
    do
    {
        if (Program[pos]=='.')
            dot=true;
        token.Str += Program[pos];
        pos++;
    }while (isdigit(Program[pos]) || (Program[pos]=='.' && !dot));
    token.Type = NUMBER;
    return NUMBER;
}

//  To get the string type of token
int GetString()
{
    pos++;
    do
    {
        if (Program[pos]=='\n')
            Error("Expected ending quotation mark '\"'...\n\t\tin line:\n\t",lineStart, pos-lineStart+1);
        token.Str += Program[pos];
        pos++;
    }while (Program[pos]!='"');
    pos++;
    token.Type = STRING;
    return STRING;
}

//  To get the character type of token
int GetCharacter()
{
    pos++;
    if (Program[pos]=='\n' || '\'')
        Error("Expected a character between ' '...\n\t\tin line:\n\t",lineStart, pos-lineStart+1);
    token.Str+=Program[pos];
    pos++;
    if (Program[pos]!='\'')
        Error("Expected ending single quotation mark ' ...\n\t\tin line:\n\t",lineStart, pos-lineStart+1);
    token.Type = CHARACTER;
    return CHARACTER;
}

//  To get the symbol type of token
int GetSymbol()
{
    token.Str += Program[pos];
    if ((Program[pos]=='<'||Program[pos]=='>'||Program[pos]=='!'||Program[pos]=='=')&&Program[pos+1]=='=')
    {
        pos++;
        token.Str += Program[pos];
    }
    pos++;
    token.Type = SYMBOL;
    return SYMBOL;
}


//  To get te next token once the current one is scanned
int GetNextToken()
{
    token.Str = "";
    token.Type = UNKNOWN;

    SkipWhiteSpace();
    token.LineStart = lineStart;
    token.Pos = pos-lineStart+1;

    if (Program[pos]=='\n')
        {token.Type = EOL; return EOL;}
    if ((isalpha(Program[pos]))||(Program[pos]=='_' && isalpha(Program[pos+1])))
        return GetIdentifier();
    if ((isdigit(Program[pos]))||(Program[pos]=='.' && isdigit(Program[pos+1])))
        return GetNumber();
    if (Program[pos]=='"')
        return GetString();
    if (Program[pos]=='\'')
        return GetCharacter();
    if (ispunct(Program[pos]))
        return GetSymbol();

    Error("Unrecognized Token...\n\t\tin line:\n\t",lineStart, pos-lineStart+1);
    return UNKNOWN;
}

//  Prepare a list of all the tokens in the program
void PrepareTokensList()
{
    pos=ln=lineStart=0;
    while (ln<lines)
    {
        GetNextToken();
        if (token.Type==EOL){
            lineStart=pos+1;
            ln++;
            pos++;
        }
        Tokens.push_back(token);
    }
}
