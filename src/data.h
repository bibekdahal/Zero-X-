#ifndef DATA_H_INCLUDED
#define DATA_H_INCLUDED

////////////////////////////////////////////////
//
//  The ultimate DataBase where, you can get
//  all the data your compiler needs
//  Just include "stdinc.h" and you can
//  have access to everything below
//
////////////////////////////////////////////////

// Some handy includes
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

// To initialize the database
void InitData();

//***************** TokenInfo *********************
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
    long unsigned Pos;              //Token's position in current line << This is what you don't have in old Zero-X
};
extern vector<_Token> Tokens;
extern unsigned long tk;
#define Token       Tokens[tk]
#define NextToken   if (tk<Tokens.size()-1) tk++;
//*************************************************


//**************** ParseTree **********************
struct Node
{
    string Value;
    string Attribs[3];
    Node * Left, * Right;
};
void CreateTree(Node*& pt);
void DeleteTree(Node*& pt);
extern Node* ParseTree;
//*************************************************

//**************** DataBase ***********************
// All variables has got a name and a type
struct VarInfo
{
    string Name;
    int Type;               //The type is a number : the index of the type in 'Types' vector
    string Id;              //Unique name for each variable // an underscore ( _ ) is added for every new scope
};
// Functions have name, type and parameters
struct FuncInfo
{
    string Name;
    int Type;
    unsigned long ParamSize;    //The total size of parameters in bytes can be useful
    vector<VarInfo> Params;
};
extern int RetType;     // Return type of function we are currently parsing

// Types have name, members, a NEW function and a DELETE function
struct TypeInfo
{
    string Name;
    unsigned long Size;         //Total size of memebers in bytes
    vector<VarInfo> Members;

    FuncInfo NewFunc; bool New;  //The bools store if the NEW and DELETE functions are defined
    FuncInfo DelFunc; bool Del;
};
// New scopes are created when valid blocks {...} are created
// void main()
// {                << New Scope
//     if (a==2)
//     {            << New Scope, parent of previous one
//       ...
//     }
// }
// Besides there is also a GLOBAL SCOPE, which is parent of all scopes
struct Scope
{
    Scope * Parent;         // Parent of current scope
    vector<Scope*> Children;    // All children of current scope
    vector<VarInfo> Vars;   // Variables in current scope

    string Id;  // A scope id - to add before variables' names to creare variable id's
};
// Current Scope
extern Scope* CurrentScope;

// Maximum precedence level for any operator, we may change this later, if required
#define MAX_PD 6
// Operators have name, can be unary, can have a Left expression type,
// a Right expression type, a Return type, a precedence level,
// is normally associated with a function, which can be of type SERIES
struct OprInfo
{
    string Name;
    bool Unary;

    int LType;
    int RType;
    int RetType;
    int Precedence;

    FuncInfo OprFunc;
    bool Series;
};

// Check if a type is numerical : numerical types are interchangeable
bool IsNumber(int Type);
// Check if two types are equal (or equivalent like different numerical types)
bool CheckTypes(int Type1, int Type2);
// Check is a type is Pointer Type : pointer types are named by preceding with *'s
bool IsPointer(int Type);
// Get type pointed by a pointer type
int GetPointedType(int Type);

// Check if an identifier is valid for variable/function/type name
bool CheckValidName(string Name);
// Check if an operator with given LType, RType and Precedence level are already defined
bool CheckValidOpr(string Name, int LType, int RType, int PD);

// Check if a variable name is already defined
bool CheckVar(string Name);
// Get the type of a variable
int GetVarType(string Name, Scope* scope = CurrentScope);
// Get the ID of a variable
string GetVarId(string Name, Scope* scope=CurrentScope);

// Check if a function is already defined
bool CheckFunction(string Name);
// Hey, I think these are simple, so I shall skip commenting these, shall I?
int GetFuncType(string Name);
unsigned long GetParamSize(string Function);
int GetNoOfParam(string Function);

bool CheckParameter(string Function, string Name);
int GetParamId(string Function, string Param);
string GetParamName(string Function, int ParamId);
int GetParamType(string Function, int ParamId);
int GetParamType(string Function, string Param);

bool CheckType(string Name);
int GetType(string Type);
string GetTypeName(int Type);
unsigned long GetTypeSize(int Type);
int GetNoOfMember(int Type);

bool CheckMember(int Type, string Name);
int GetMemberId(int Type, string Member);
string GetMemberName(int Type, int MemberId);
int GetMemberType(int Type, int MemberId);
int GetMemberType(int Type, string Member);

bool CheckOpr(string Name);
bool CheckUnaryOpr(string Name, bool Pre); //bool Pre means whether it is preunary or postunary
bool CheckOpr(string Name, int PD);
int GetRetType(string Opr, int LType, int RType, int PD);
int GetRetType(string Opr, int Type, bool Pre); //For unary operators

// Add the Type to database
int AddType(TypeInfo Type);
// Change a Type stored in database
void ChangeType(int tp, TypeInfo Type);
// Add a NEW function for a type in database
void AddTypeNewFunc(int Type, FuncInfo Func);
// Add a DELETE function for...
void AddTypeDelFunc(int Type, FuncInfo Func);

// Yes, understandable, aren't these?
void AddOpr(OprInfo Opr);
void AddFunction(FuncInfo Func);
void AddVar(VarInfo Var, Scope* scope=CurrentScope);

// Start a new scope to store variables
void NewScope();
// Get back to parent scope ending current scope
void ParentScope();

// These functions add FUNCTION PARAMETERS as variables in current scope
// since parameters are also handled similar to local variables
void AddFuncToScope(string Function);
void AddOprFuncToScope(string Opr);
void AddTypeFuncToScope(int Type, string Function);

//*************************************************


//****************** MISC *************************
void Deallocate();

void PrintOutTypes();
void PrintOutGlobals();
void PrintOutFuncs();
void PrintOutOprs();
void PrintOutTree(Node * nd, string dash="");
//*************************************************
#endif // DATA_H_INCLUDED
