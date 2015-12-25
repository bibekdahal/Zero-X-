//////////////////////////////////////////////
// This file contains some useful common
// parsing codes
//////////////////////////////////////////////

#include "stdinc.h"
#include "lex.h"
#include "stageI.h"
#include "stageII.h"


// Skip a line of statement
void SkipLine()
{
    while (Token.Type!=EOL) NextToken;
    return;
}
// Skip a block {...} of statements
void SkipBlock()
{
    int SB = 0;  //To record no. of started and unclosed blocks
                 //{...{...{...}...{... has 3 SB's: 1 out of 4 {'s has been closed with }
    while (Token.Str!="{")
    {
        if (Token.Type==EOL) ln++;
        if (ln>=lines || Token.Str=="}") Error("Invalid file ending: expected '{'", Token.LineStart);
        NextToken;
    }
    NextToken;
    SB++;
    while (SB!=0)
    {
        if (Token.Str=="{") SB++;
        if (Token.Str=="}") SB--;

        if (Token.Type==EOL) ln++;
        if (ln>=lines) Error("Invalid file ending: expected '}'", Token.LineStart);
        NextToken;
    }
}
// Skip EOL's to get to next useful token
void NextStatement()
{
    while (Token.Type==EOL)
    {
        NextToken;
        ln++;
        if (ln>=lines) Error("Invalid ending", Token.LineStart);
    }
}

// Check if a function declaration is incoming
bool IncomingFunction()
{
    // Starting with a type can mean a function can be incoming
    if (!CheckType(Token.Str)) return false;
    // Record the index of out present token : so that we look forward and come back again
    int pTk = tk;
    NextToken;
    // skip the *'s
    while (Token.Str=="*") NextToken;
    NextToken;
    // A '(' after a type name can only mean function's parameters list is incoming, so this is a function declaration
    // Reset the Token index and return true
    if (Token.Str=="(") {tk=pTk; return true;}
    // else reset the index and return false
    tk=pTk; return false;
}

// Create a new pointer type for a given pointer, nPtr means depth of the pointer : no of *'s in the declaration
// Don't create just return, if already created
int SetPointerType(int Type, int nPtr)
{
    // The name of the pointer type is : name_of_type + '*'
    string name=GetTypeName(Type)+"*";
    // Check if the type exists, if not, create the type
    if (!CheckType(name))
    {
        TypeInfo tp;
        tp.Name=name;
        tp.Size=4;  // Every pointer type is 4 bytes in size
        AddType(tp);    // Add it to the database

        // Note that every pointer can have the dereference operator : *
        // int ** a
        // **a should return an integer, pointed by the pointer which is further pointed by 'a'
        OprInfo opr;
        opr.Name="*";
        opr.LType=GetType("VOID");  //PreUnary has LType "VOID"
        opr.RType=GetType(name);
        opr.RetType=Type;
        opr.Unary=true;
        opr.Precedence=-1;  // Every unary operator is marked precedence -1
        AddOpr(opr);    //Add to the database
    }
    // Oh, more no. of pointers to be created: create them
    if (nPtr>1) return SetPointerType(GetType(name), nPtr-1);
    // return the pointer type
    return GetType(name);
}
// Parse the type: int** a
//                 ^^^^^
int ParseType()
{
    int Type =  GetType(Token.Str);
    if (Type==-1)
        Error("Invalid Type", Token.LineStart, Token.Pos);
    NextToken;

    int PTRS=0;
    while (Token.Str=="*")
    {
        PTRS++;
        NextToken;
    }

    if (PTRS>0) Type=SetPointerType(Type, PTRS);
    return Type;
}
// Parse a 'variable/type member/function parameter' declaration
void ParseDeclare(VarInfo & var, string InType)
{
    // Check for valid type : if inside a type, only pointer to the InType is valid (can u guess why?)
    if (!CheckType(Token.Str) || (InType==Token.Str && Tokens[tk+1].Str!="*"))
        Error("Invalid Type", Token.LineStart,Token.Pos);
    // Parse out the type
    int Type = ParseType();

    // Check for validity in name
    if (Token.Type!=IDENTIFIER) Error("Invalid Name: " + Token.Str, Token.LineStart,Token.Pos);
    if (!CheckValidName(Token.Str)) Error("Invalid Name; Already in Used: " + Token.Str, Token.LineStart,Token.Pos);
    string Name = Token.Str;

    NextToken;
    // Set the varinfo
    var.Name = Name;
    var.Type = Type;

    ///////// In future, we will be adding code to parse array declarations here as well,
    ///////// but as you shall see, we need to complete stageII before doing that

}
