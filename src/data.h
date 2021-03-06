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
#include <map>
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
    bool Param;     //Is a Parameter?
    string Id;              //Unique name for each variable // an underscore ( _ ) is added for every new scope
};
// Functions have name, type and parameters
struct FuncInfo
{
    string Name;
    int Type;
    unsigned long ParamSize;    //The total size of parameters in bytes can be useful
    vector<VarInfo> Params;

    string Id;
};
extern int RetType;     // Return type of function we are currently parsing

// Types have name, members, a NEW function and a DELETE function
struct TypeInfo
{
    string Name;
    unsigned long Size;         //Total size of memebers in bytes
    vector<VarInfo> Members;

    vector<FuncInfo> NewFuncs;
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

    string Id;
};

// Check if a type is numerical : numerical types are interchangeable
bool IsNumber(int Type);
// Check if two types are equal (or equivalent like different numerical types)
bool CheckTypes(int Type1, int Type2);
// Check if Type2 can be assigned to Type1
bool CheckAssgnTypes(int Type1, int Type2);
// Check if array types match excluding the first dimension
bool CheckFuncArrParam(int Type1, int Type2);
// Check is a type is Pointer Type : pointer types are named by preceding with *'s
bool IsPointer(int Type);
// Get type pointed by a pointer type
int GetPointedType(int Type);
// Get type pointed by a array type
int GetArrType(int Type);
// Get the number of dimensions of array type
int GetArrayIdCnt(int Type);
// Get the size of dm-th dimension of the array type
long GetDimSize(string Type, int dm);

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
// Check if a variable name is actually a parameter
bool CheckParam(string Name);

// Check if a function is already defined
bool CheckFunction(string Name);
// Hey, I think these are simple, so I shall skip commenting these, shall I?
int GetFuncType(string Name);
long unsigned GetParamSize(string Function);
int GetNoOfParam(string Function);

bool CheckParameter(string Function, string Name);
int GetParamType(string Function, int ParamId);
int GetParamType(string Function, string Param);

bool CheckType(string Name);
int GetType(string Type);
string GetTypeName(int Type);
long unsigned GetTypeSize(int Type);
int GetNoOfMember(int Type);

bool CheckMember(int Type, string Name);
int GetMemberType(int Type, string Member);
int GetMemOffset(int Type, string Member);

string GetOprId(string Name, int LType, int RType, int PD);
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
// Get function id of NEW function containing certain parameter
string GetNewTypeId(int Type, int ParamTp);

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
void AddTypeFuncToScope(int Type, string FuncId);
// Get the constructor NEW function of a type; return empty string "" if doesn't exist
string GetCTor(int Type);

int GetGreaterNumType(int Type1, int Type2);
//*************************************************


//***************** TRICODE ***********************
struct tcode
{
    // Three address code / 3 address: a,b,c / 1 operation / Type
    string Type;
    string Opr;
    string a;
    string b;
    string c;
};

struct tblock
{
    // A block of tcodes : so that each block can be optimised independently of other blocks
    string Name;
    string Type;
    vector<tcode> tcodes;
};
// Add new block
void NewBlock(string Name, string Type);
// Add a code to current block
void AddTCode(tcode Tcode);
// Get a new temporary name
string GetTmp();
// Clear all temporary names
void ResetTmp();
// Add a temporary name
void AddTmp(string tmp);

// Add a ToSave temporary name
void ToSave(string tmp);
// Clear all ToSave temporary names
void ClearSaved();
// Save the ToSave temporary names
void SaveAll();
// Get all the saved temporary names
void GetAll();

// Get a new label name
string GetLabel();
//*************************************************


//****************** MISC *************************
void Deallocate();

void PrintOutTypes();
void PrintOutGlobals();
void PrintOutFuncs();
void PrintOutOprs();
void PrintOutTree(Node * nd, string dash="");
void PrintOutTCodes();
//*************************************************
#endif // DATA_H_INCLUDED
