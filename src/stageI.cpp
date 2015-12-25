#include "stdinc.h"
#include "lex.h"
#include "common.h"
#include "stageI.h"

// Parse a TYPE
// TYPE type_name
// {
//     mem_type mem_name
//     ...
// }
void ParseTypeDecl()
{
    NextToken;  // The TYPE word is not useful, check if the type_name is valid
    if (Token.Type!=IDENTIFIER)     Error("Invalid Type Name", Token.LineStart);
    if (!CheckValidName(Token.Str)) Error("Invalid Type Name", Token.LineStart);

    // Set the type info
    TypeInfo Type;
    Type.Name = Token.Str;
    Type.Size = 0;
    // We need to record all the members to make sure one is not repeated
    vector<string> rec;

    // NextToken to get after type_name; NextStatement to get to skip EOL's if they exist and get to '{'
    NextToken;  NextStatement();
    // '{' must exist
    if (Token.Str!="{") Error("Expected member list '{...}'", Token.LineStart);
    // Again skip EOL's if exist to get to first member declaration
    NextToken;  NextStatement();

    // Add the new type, since pointer to new_type should be valid for member declaration, as well
    // TYPE my_type{
    //    my_type* hi
    // } is valid
    int tp = AddType(Type);

    // NEW and DELETE are functions, if they are incoming, we just skip them; since they are parsed later
    while (IncomingFunction())   {SkipBlock(); NextToken;  NextStatement();}

    // Now get member declarations
    do
    {
        VarInfo var; // ParseDeclare does the declaration parsing stuff
        ParseDeclare(var, Type.Name);

        // Check if valid member
        for (unsigned i=0;i<rec.size();i++)
            if (rec[i]==var.Name) Error("Already got this member", Token.LineStart);
        rec.push_back(var.Name);

        // Check if EOL, next declaration should be start at new line
        if (Token.Type!=EOL)    Error("Expected End Of Line !!!", Token.LineStart);
        // Skip EOL's
        NextToken;  NextStatement();
        // Also skip function if any is incoming
        while (IncomingFunction())   {SkipBlock(); NextToken;  NextStatement();}

        // New member, okay? Add it to the list
        Type.Members.push_back(var);
        // Record the size in bytes of the whole type : can be useful
        Type.Size += GetTypeSize(var.Type);
    } while (Token.Str!="}");
    // Oh, the vars list has been changed, don't forget to change it in the database
    ChangeType(tp,Type);
    // Always end with one extra token ahead of the statement
    // where an EOL should be residing for a valid statement
    NextToken;
}
// Get all the types declarations
void ParseTypesDecl()
{
    // for each line
    for (ln=0;ln<lines;ln++)
    {
        if (Token.Type!=EOL)    // If the token is not EOL
        {
            if (IncomingFunction() || Token.Str=="OPR")  SkipBlock(); // Skip any operator or function definition
            else if (Token.Str!="TYPE") SkipLine(); // For any other statement that doesn't start with TYPE, skip it
            else ParseTypeDecl();   // Well it is TYPE declaration, parse it and save useful info in database
        }
        if (Token.Type!=EOL)    Error("Expected End Of Line !!!", Token.LineStart);
        NextToken;
    }
}

// Get the global variable declarations
void ParseGlobalVarsDecl()
{
    for (ln=0;ln<lines;ln++)
    {
        if (Token.Type!=EOL)
        {
            if (Token.Str=="TYPE" || Token.Str=="OPR"|| IncomingFunction())  SkipBlock();
            else if (!CheckType(Token.Str)) SkipLine();
            else    // For anything that starts with a valid data type name: int a // where int is valid type name
            {
                VarInfo var;    // Use ParseDeclare to parse the variable declaration
                ParseDeclare(var);
                AddVar(var);    // And add the variable to the database
            }
        }
        if (Token.Type!=EOL)    Error("Expected End Of Line !!!", Token.LineStart);
        NextToken;
    }
}

// Gets a function declaration with the parameters
// oprFunc is set true for OPERATOR function
// and TypeFunc for NEW and DELETE functions inside a type
FuncInfo ParseFuncDecl(bool oprFunc, bool TypeFunc)
{
    int Type=-1;    // The type of a function

    if (oprFunc && Token.Str=="SERIES") {NextToken;}    // Type remains -1 for SERIES operator type
    else  Type = ParseType();   // For any other, parse out the function type

    // Check for validity
    if (oprFunc)
        {if (Token.Type==STRING || Token.Type==NUMBER)     Error("Invalid Operator Name", Token.LineStart);}
    else
    {
        if (Token.Type!=IDENTIFIER)     Error("Invalid Function Name", Token.LineStart);
        if (TypeFunc)
            {if (Token.Str!="NEW" && Token.Str!="DELETE")    Error("Invalid Function Name", Token.LineStart);}
        else
            if (!CheckValidName(Token.Str)) Error("Invalid Function Name", Token.LineStart);
    }

    // You must find this somehow similar to Type Declaration function, try to figure things out yourself
    FuncInfo func;
    func.Name = Token.Str;
    func.Type = Type;
    func.ParamSize=0;
    vector<string> rec;

    NextToken; // Got the "("
    NextToken;
    if (Token.Str!=")")
    while (true)
    {
        VarInfo var;
        ParseDeclare(var);
        if (Token.Str!=")" && Token.Str!=",")   Error("Expected comma as parameters separator", Token.LineStart, Token.Pos);

        for (unsigned i=0;i<rec.size();i++)
            if (rec[i]==var.Name) Error("Already got this parameter", Token.LineStart, Token.Pos);
        rec.push_back(var.Name);
        func.Params.push_back(var);
        func.ParamSize += GetTypeSize(var.Type);

        if (Token.Str==")") break;
        if (Token.Str==",") NextToken;

    }
    NextToken;
    return func;
}
// Parse a normal function declaration
void ParseFunction()
{
    // Add function to the database
    AddFunction(ParseFuncDecl(false,false));
    // The definition part is skipped
    SkipBlock();
}
// Parse out an operator type of function
void ParseOprFunction()
{
    NextToken;
    // NextToken to skip OPR keyqord, call ParseFuncDecl to get the function info
    FuncInfo func = ParseFuncDecl(true,false);

    bool Unary=false, Pre=false;    // Every operator is binary by default
    if (Token.Str=="PREUNARY") {NextToken;Unary=true;Pre=true;} // unless, of course specified
    else if (Token.Str=="POSTUNARY") {NextToken;Unary=true;}
    // Precedence level
    int PD;
    // Unary operators need to have only one operand/parameter and should not be defined as SERIES
    if (Unary)
    {
        PD=-1;
        if (func.Params.size()!=1)
            Error("Only one parameter for unary operator function", Token.LineStart);
        if (func.Type<0) // The SERIES keyword in ParseFuncDecl returns the type of function as -1 < 0
            Error("Function type invalid", Token.LineStart);
    }
    // Binary operator has two operands
    else if (func.Params.size()!=2)
        Error("Two, and only two, parameters are used in operator function", Token.LineStart);

    // If not unary, then a precedence level must be specified
    if (!Unary)
    {
        if (Token.Type!=NUMBER)
            Error("Expected a number after operator function declaration, determining its precedence level", Token.LineStart, Token.Pos);
        PD = (int)ToNum(Token.Str);
        if (PD>MAX_PD || PD<0)
            Error("Invalid precedence level: must be between 0 and "+ToStr(MAX_PD), Token.LineStart, Token.Pos);
        NextToken;
    }

    // Now for the storage of operator information
    OprInfo opr;
    opr.Unary=Unary;
    // SERIES operator needs first parameter to be INT type and second to be POINTER type
    if (func.Type==-1)
    {
        if (func.Params[0].Type!=GetType("INT"))
            Error("First parameter of series operator function must be of type INT", Token.LineStart);
        if (!IsPointer(func.Params[1].Type))
            Error("Second parameter of series operator function must be of a pointer type", Token.LineStart);
        opr.RetType=opr.LType=opr.RType=GetPointedType(func.Params[1].Type);
    }
    // PreUnary operators has LTYPE "VOID" and RTYPE the type of parameter/operand, RETTYPE is func.Type
    else if (Unary && Pre)
    {
        opr.LType = GetType("VOID");
        opr.RType = func.Params[0].Type;
        opr.RetType = func.Type;
    }
    // Not PreUnary and still UNARY means PostUnary, set RTYPE to "VOID"
    else if (Unary)
    {
        opr.LType = func.Params[0].Type;
        opr.RType = GetType("VOID");
        opr.RetType = func.Type;
    }
    // For others,...
    else
    {
        opr.LType = func.Params[0].Type;
        opr.RType = func.Params[1].Type;
        opr.RetType = func.Type;
    }

    opr.Name = func.Name;
    opr.Precedence = PD;
    opr.OprFunc = func;

    // Check for validity : if the operator with given LType, RType and Precedence Level has already been defined
    if (!CheckValidOpr(opr.Name,opr.LType,opr.RType,opr.Precedence))
         Error("Invalid Operator Name", Token.LineStart);
    // Add the operator to the database
    AddOpr(opr);
    // Skip the definition part
    SkipBlock();
}
// Parse NEW and DELETE functions in a TYPE
void ParseTypeFuncDecl(int Type)
{
    FuncInfo func = ParseFuncDecl(false,true);
    // Both NEW & DELETE receives as parameter, the pointer to current Type
    // Make sure the PointerType exist
    int PTRType = SetPointerType(Type,1);
    // Check if valid parameters are passed
    if (func.Name=="NEW")
    {
        if (!CheckTypes(func.Type,Type))
            Error("A type's NEW function must return same type", Token.LineStart);
        if (func.Params.size()!=1 || !CheckTypes(func.Params[0].Type,PTRType))
            Error("A type's NEW function must have parameter of pointer to same type", Token.LineStart);
        AddTypeNewFunc(Type,func);
    }
    else if (func.Name=="DELETE")
    {
        if (GetTypeName(Type)!="VOID")
            Error("A type's DELETE function must return VOID type", Token.LineStart);
        if (func.Params.size()!=1 || !CheckTypes(func.Params[0].Type,PTRType))
            Error("A type's DELETE function must have parameter of pointer to same type", Token.LineStart);
        AddTypeDelFunc(Type,func);
    }
    else
        Error("Only NEW and DELETE functions are allowed for a type", Token.LineStart);
    // Skip the definition
    SkipBlock();
}

// Parse out all types of functions
void ParseFuncsDecl()
{
    // If inside a type, this variable will contain the type name
    string InType = "";
    for (ln=0;ln<lines;ln++)
    {
        if (Token.Type!=EOL)
        {
            if (Token.Str=="TYPE")  {NextToken;InType=Token.Str;}   // Got inside a type, set InType
            if (Token.Str=="}" && InType!="") InType="";    // Got out of a type, reset InType

            if (Token.Str=="OPR") ParseOprFunction();   // An operator function, parse the declaration
            else if (IncomingFunction())    // Else a function is incoming
            {
                // ...
                if (InType!="") ParseTypeFuncDecl(GetType(InType));
                else ParseFunction();
            }
            // skip any other statement
            else SkipLine();
        }
        if (Token.Type!=EOL)    Error("Expected End Of Line !!!", Token.LineStart);
        NextToken;
    }
}
