#include "stdinc.h"
#include "lex.h"
#include "common.h"
#include "stageII.h"


// For all factors and operators,
//  Attribs[2] is data type of factor
//  Attribs[0] is the kind
// For e.g.: for an operator adding two numbers
//  Attribs[2]="DOUBLE"
//  Attribs[0]="OPERATOR"


// Parse a parameter for a function call
// foo(2+3*6-8,"hello",...)
//     ^^^^^^^   each parameter is an expression
// PId is the id of the parameter to be parsed
void ParseParameters(Node* &nd, string Function, int PId)
{
    // EOL or ) means end to parameter parsing
    if (Token.Type==EOL || Token.Str==")") return;

    // The node's value is PARAMETER
    CreateTree(nd);
    nd->Value="PARAMETER";

    // Parse the expression is node's left child
    int Type;
    // Arrays on expressions need indices, but not if array needs to be passed as parameter
    if (GetArrayIdCnt(GetParamType(Function, PId))>0)
    {
        // Similar to initial code of ParseVarFactor:
        if (Token.Type!=IDENTIFIER) Error("Invalid Factor", Token.LineStart, Token.Pos);
        if (!CheckVar(Token.Str))   Error("Invalid Variable Name", Token.LineStart, Token.Pos);

        CreateTree(nd->Left);

        string var = Token.Str;
        nd->Left->Value = GetVarId(var);
        Type = GetVarType(var);
        nd->Left->Attribs[0]="ARR_PARAM_VAR";
        nd->Left->Attribs[2]=GetTypeName(Type);
        NextToken;

        // We also need to check if the dimensions of array types besides the first dimention match
        if (!CheckFuncArrParam(GetParamType(Function, PId), Type))
            Error("Invalid parameter type",Token.LineStart,Token.Pos);
    }
    else
    {
        Type = ParseExpression(nd->Left, 0);
        // Check if types of parameter and expression match
        if (!CheckAssgnTypes(GetParamType(Function, PId), Type))
            Error("Invalid parameter type",Token.LineStart,Token.Pos);
    }

    // DONE for this parameter, check if more parameters exist and if yes, parse another parameter
    if (PId != GetNoOfParam(Function)-1)
    {
        // Two parameters must be separated by a comma
        if (Token.Str!=",") Error("Expected comma to separate more parameters", Token.LineStart, Token.Pos);
        NextToken;
        // If EOL or ")" before another parameter, ERROR
        if (Token.Type==EOL || Token.Str==")") Error("Expected more parameters", Token.LineStart, Token.Pos);
        ParseParameters(nd->Right, Function, PId+1);
    }
}
//A function is called
void ParseCall(Node* &nd)
{
    // Node's name is CALL, first attribute is function's name and on its right are parameters
    CreateTree(nd);
    nd->Value ="CALL";
    nd->Attribs[0] = Token.Str;

    NextToken;
    if (Token.Str!="(") Error("Expected parameters list for function: " + Token.Str, Token.LineStart, Token.Pos);
    NextToken;

    if (GetNoOfParam(nd->Attribs[0])!=0) ParseParameters(nd->Right, nd->Attribs[0],0);
    if (Token.Str!=")") Error("Expected ending paranthesis: )", Token.LineStart, Token.Pos);
    NextToken;
    return;
}

// An array variable needs to be followed by indices, parse each index
int ParseArray(Node* &nd, int Type, int id)
{
    // Check '[' which indicate array index is incoming
    while (Token.Str=="[")
    {
        NextToken;
        // Node's value is ARRAY with left child the array variable ....
        Node tmp = *nd;
        CreateTree(nd->Right);
        CreateTree(nd->Left);
        *nd->Left = tmp;
        nd->Value = "ARRAY";
        nd->Attribs[0] = "";
        // .... and right child the index expression
        if (!IsNumber(ParseExpression(nd->Right,0)))
            Error("Invalid array index", Token.LineStart, Token.Pos);
        // Must be ended with ']'
        if (Token.Str!="]")
            Error("Expected ']'",Token.LineStart, Token.Pos);
        NextToken;
        id++;
    }

    if (id!=GetArrayIdCnt(Type))
        Error("Expected more indices", Token.LineStart, Token.Pos);
    Type = GetArrType(Type);
    nd->Attribs[2]=GetTypeName(Type);
    return Type;
}
// We have got a pointer variable, so check if followed by array indices, if yes, parse them
// a[2][3][b*c-3]
//         ^^^^^ Each index is a numeric expression
int ParsePtrArray(Node* &nd, int Type)
{
    // Check '[' which indicate array index is incoming
    if (Token.Str=="[")
    {
        NextToken;
        // Node's value is PTR_ARRAY with left child the pointer variable ....
        Node tmp = *nd;
        CreateTree(nd->Right);
        CreateTree(nd->Left);
        *nd->Left = tmp;
        nd->Value = "PTR_ARRAY";
        nd->Attribs[0] = "";
        // .... and right child the index expression
        if (!IsNumber(ParseExpression(nd->Right,0)))
            Error("Invalid array index", Token.LineStart, Token.Pos);
        // Must be ended with ']'
        if (Token.Str!="]")
            Error("Expected ']'",Token.LineStart, Token.Pos);
        NextToken;
        // And type of factor is the type pointed by the variable
        Type = GetPointedType(Type);
        nd->Attribs[2]=GetTypeName(Type);
        // If the pointed type is also a pointer, we can have another array index, parse it
        if (IsPointer(Type))    Type = ParsePtrArray(nd, Type);
    }
    return Type;
}
// We have got a '.' following a factor, meaning the programmer wants a member of the type
// type.mem1.mem2...
//      ^^^^^ a member can have further member
int ParseMember(Node* &nd, int Type)
{
    // Node's value is "MEMBER", the left is the parent factor, right the member
    Node tmp = *nd;
    CreateTree(nd->Right);
    CreateTree(nd->Left);
    *nd->Left = tmp;
    nd->Value = "MEMBER";
    nd->Attribs[0] = "";

    // Check if it is valid member and get the data type of member
    NextToken;
    if (!CheckMember(Type, Token.Str))  Error("Invalid member", Token.LineStart, Token.Pos);
    int tp = GetMemberType(Type, Token.Str);    //                                              <<<<< ---|
                                                //                                                       |
    // The right value is the member's name     //                                                       |
    nd->Right->Value = Token.Str;               //                                                       |
    string mem = Token.Str;                     //                                                       |
    NextToken;                                  //                                                       |
    // Check for indices data                   //                                                       |
    if (GetArrayIdCnt(tp)>0) tp = ParseArray(nd->Right, tp, 0);                     //          <<<<< ---|
    if (IsPointer(tp))    tp = ParsePtrArray(nd->Right,tp);                         //          <<<<< ---|
                                                //                                                       |
    // See if another member is incoming        //                                                       |
                                                //                                                       |
    if (Token.Str==".")                         //                                                       |
        tp = ParseMember(nd->Right,tp);         //                                              <<<<< ---|
                                                //                                                       |
    // The factor type is either the type of member or the type of member of member, stored in 'tp' as --^
    nd->Attribs[2]=GetTypeName(tp);
    return tp;
}
// Parse a factor starting with a variable name
int ParseVarFactor(Node* &nd)
{
    // Not an identifier or a variable name, ERROR
    if (Token.Type!=IDENTIFIER) Error("Invalid Factor", Token.LineStart, Token.Pos);
    if (!CheckVar(Token.Str))   Error("Invalid Variable Name", Token.LineStart, Token.Pos);

    // Id node's value is the variable's unique ID, factor type is variable's data type
    string var = Token.Str;
    nd->Value = GetVarId(var);
    int Type = GetVarType(var);
    nd->Attribs[0]="VARIABLE";
    nd->Attribs[2]=GetTypeName(Type);
    NextToken;

    // Variable might be an array
    if (GetArrayIdCnt(Type)>0) Type = ParseArray(nd, Type, 0);

    // A pointer type can also be followed by indices
    if (IsPointer(Type))    Type = ParsePtrArray(nd,Type);

    // A '.' means a member is incoming
    if (Token.Str==".") Type = ParseMember(nd, Type);

    return Type;
}

// Parse a factor
int ParseFactor(Node* &nd)
{
    // A factor can be another expression in brackets: (2+3)*6 , (2+3) is a factor
    if (Token.Str=="(")
    {
        NextToken;
        int Type = ParseExpression(nd, 0);
        if (Token.Str!=")") Error("Expected ending paranthesis: )", Token.LineStart, Token.Pos);
        NextToken;

        // AND a bracketed expression can also have a member or array index
        if (IsPointer(Type))   Type=ParsePtrArray(nd,Type);
        if (Token.Str==".")    Type = ParseMember(nd, Type);
        return Type;
    }

    CreateTree(nd);
    // STRING type of token is a pointer to CHAR type
    if (Token.Type==STRING)
    {
        nd->Value = Token.Str;
        nd->Attribs[0] = "VALUE";
        nd->Attribs[2]="CHAR*";
        NextToken;
        return GetType(nd->Attribs[2]);
    }
    // NUMBER are either INT or DOUBLE depending on presence or absence of decimal points
    else if (Token.Type==NUMBER)
    {
        nd->Value = Token.Str;
        nd->Attribs[0] = "VALUE";
        if (Token.Str.find('.')==string::npos) nd->Attribs[2]="INT";
        else nd->Attribs[2]="DOUBLE";
        NextToken;
        return GetType(nd->Attribs[2]);
    }
    // CHARACTER are either CHAR types
    else if (Token.Type==CHARACTER)
    {
        nd->Value = Token.Str;
        nd->Attribs[0] = "VALUE";
        nd->Attribs[2]="CHAR";
        NextToken;
        return GetType(nd->Attribs[2]);
    }
    // TRUE and FALSE are BOOL constants
    else if (Token.Str=="TRUE" || Token.Str=="FALSE")
    {
        nd->Value = Token.Str;
        nd->Attribs[0] = "VALUE";
        nd->Attribs[2]="BOOL";
        NextToken;
        return GetType(nd->Attribs[2]);
    }
    // & is always followed by a variable factor (plain variable/with member/with array index)
    // denotes a pointer type
    else if (Token.Str=="&")
    {
        nd->Value = "PTR";
        CreateTree(nd->Right);
        NextToken;
        int Type = ParseVarFactor(nd->Right);
        nd->Attribs[0] = "ADDRESS";
        Type = SetPointerType(Type, 1);
        nd->Attribs[2] = GetTypeName(Type);
        return Type;
    }
    // Function can be factors
    else if (CheckFunction(Token.Str))
    {
        nd->Value = Token.Str;

        NextToken;
        if (Token.Str!="(") Error("Expected parameters list for function: " + Token.Str, Token.LineStart, Token.Pos);
        NextToken;

        if (GetNoOfParam(nd->Value)!=0) ParseParameters(nd->Right, nd->Value,0);
        if (Token.Str!=")") Error("Expected ending paranthesis: )", Token.LineStart, Token.Pos);
        NextToken;
        nd->Attribs[0] = "CALL_FACTOR";
        nd->Attribs[2] = GetTypeName(GetFuncType(nd->Value));
        return GetType(nd->Attribs[2]);
    }
    // Only left kind of factor is variable factor
    else
        return ParseVarFactor(nd);

    // Not too hard, are they?
    return -1;
}

// Factor's can also be preceded by PreUnary operators and followed by PostUnary operators
// These are parsed here
int ParseFactorEx(Node* &nd)
{
    // If a PreUnary operator exists
    if (CheckUnaryOpr(Token.Str,true))
    {
        // Node's value is the operator, right child is another factor, return type is got from database
        CreateTree(nd);
        nd->Value = Token.Str;
        nd->Attribs[0] = "UNARY_OPERATOR";
        NextToken;
        int Type = ParseFactorEx(nd->Right);    //FactorEx again, since more PreUnary operators may exits: ***a
        nd->Attribs[2] = GetTypeName(GetRetType(nd->Value, Type,true));
        return GetType(nd->Attribs[2]);
    }

    // No more pre operators, parse the actual factor
    ParseFactor(nd);

    // We used recursive function to parse more than one PreUnary operators
    // We shall now use loop to parse more than one PostUnary operators
    // This time the factor is in left child of the operator node
    while (CheckUnaryOpr(Token.Str,false))
    {
        Node tmp;
        tmp = *nd;
        CreateTree(nd->Left);
        (*nd->Left) = tmp;

        nd->Value = Token.Str;
        nd->Attribs[0] = "UNARY_OPERATOR";
        NextToken;
        int Type = GetRetType(nd->Value, GetType(nd->Left->Attribs[2]),false);
        nd->Attribs[2] = GetTypeName(Type);
    }
    return GetType(nd->Attribs[2]);
}

// Parse the expression
// PrecL is the precedence level of the operator(s) to parse
// In first call, PrecL should be '0'
//
// Note that '+' has less precedence than '*' but is parent of '*' operator in expression: 2*3+7
// So the left and right child node of every operator are expressions with operators of higher precedence
int ParseExpression(Node* &nd, int PrecL)
{
    // If we have reached over the maximum precedence level, time to parse a factor
    if (PrecL>MAX_PD) return ParseFactorEx(nd);
    // Parse another expression of higher precedence and set it to LType
    int LType = ParseExpression(nd, PrecL+1);

    unsigned long tmpTK;
    // Check if an operator of current precedence level exists
    while (CheckOpr(Token.Str, PrecL))
    {
        // And though an operator exists, it may not be what we want;
        // so mark the token position to return back in case we need to
        tmpTK = tk;

        // Then the operator is the Node and the previously parsed expression is the left child node
        string opr = Token.Str;

        Node tmp;
        tmp = *nd;
        CreateTree(nd->Left);
        (*nd->Left) = tmp;

        nd->Attribs[0] = "OPERATOR";
        NextToken;
        // Right child node is the another expression of higher precedence level
        int RType = ParseExpression(nd->Right, PrecL+1);

        // Return type is got from database
        int RetType = GetRetType(opr, LType, RType, PrecL);

        // Wait!!!!!! If RetType==-1 then
        // The operator with such LType and RType and Precedence level DO NOT exist
        // May be, such operator with such LType and RType exists but Precedence level do not match
        // In such case, we parsed the operator in wrong precedence level
        // We have to reset the previously saved token's position and also the expression tree
        if (RetType==-1)
        {
            // Don't display error as it may not be one
            // A real error will display "End Of Line" error
            DeleteTree(nd->Right);
            *nd = tmp;
            tk=tmpTK;
            break;
        }
        //Get OprId
        nd->Value = GetOprId(opr,LType,RType,PrecL);
        // Valid return type is the type of the expression
        nd->Attribs[2] = GetTypeName(RetType);
        LType = RetType;
    }
    // LType is returned as type of the expression
    return LType;
}

// Do assignment to a variable
// a = 32
// or
// **b = 32
// ^^ Dereference operators
// Also, a[22] = 32
// And, a.mem = 32
void ParseAssign(Node* &nd)
{
    // Node's value is ASSIGN, left child is the variable to be assigned to, right is the expression
    CreateTree(nd);
    nd->Value = "ASSIGN";
    CreateTree(nd->Left);

    // Check out if dereference operators are coming
    int PTRS=0;
    while (Token.Str=="*")
    {
        NextToken;
        PTRS++;
    }

    // Parse the variable: VarFactor parses all 3: plain variables/with members/with array indices
    int Type = ParseVarFactor(nd->Left);

    // If dereference operators defined,
    while (PTRS>0)
    {
        // Make sure the variable can be dereferenced
        if (IsPointer(Type))
        {
            // Data type is pointed type by the pointer variable
            Type=GetPointedType(Type);
            // Node's name is "DEREF" and previously parsed pointer variable is on the right child
            Node tmp = *(nd->Left);
            nd->Left->Value="DEREF";
            nd->Left->Attribs[0]="";
            nd->Left->Attribs[2]=GetTypeName(Type);
            CreateTree(nd->Left->Right);
            *(nd->Left->Right)=tmp;
            // Now decrease PTRS value, and redo above if PTRS!=0
            PTRS--;
        }
        else
            Error("Dereference operator * is only for pointer", Token.LineStart);
    }
    // The data type is stored in Type variable
    nd->Attribs[2] = GetTypeName(Type);

    // Assignment statement must be followed by an equal to sign
    if (Token.Str!="=") Error("Expected assignment operator: =", Token.LineStart, Token.Pos);
    NextToken;

    // Expression is on the right child
    int extp = ParseExpression(nd->Right, 0);
    // May not have reached end of line if wrong operator at the middle of the expression
    if (Token.Type!=EOL)    Error("Expected End Of Line", Token.LineStart, Token.Pos);
    // Also check if expression type can be assigned to assignment type
    if (!CheckAssgnTypes(Type, extp))
        Error("Assignment types mismatched", Token.LineStart);
}

// Parse a block after IF or WHILE statements
void ParseBlock(Node* &nd)
{
    // Skip EOL's
    NextStatement();
    // A block can be a number of statements enclosed with in '{...}'
    if (Token.Str=="{")
    {
        // Each such block starts a new scope
        NewScope();
        // NextToken to get after '{' and NextStatement to skip EOL's
        NextToken;  NextStatement();
        // Parse all the statements
        ParseStatements(nd);
        // Skip EOL's
        NextStatement();
        // If no ending brace, ERROR
        if (Token.Str!="}") Error("Expected ending of the block: }", Token.LineStart, Token.Pos);
        NextToken;
        // Get back to the parent scope
        ParentScope();
    }
    // Or it can be simply a single statement
    else
        ParseStatement(nd);
}

// A While Statement
// while (condition)
//      block of statement(s)
void ParseWhile(Node* &nd)
{
    // Node's value is WHILE, left child is condition and right child is the series of statements
    CreateTree(nd);
    nd->Value = "WHILE";

    NextToken;
    if (Token.Str!="(") Error("Enclose the condition in '(...)'", Token.LineStart,Token.Pos);
    NextToken;
    if (ParseExpression(nd->Left,0) != GetType("BOOL"))
        Error("WHILE expressions must be boolean in type", Token.LineStart);
    if (Token.Str!=")") Error("Enclose the condition in '(...)'", Token.LineStart,Token.Pos);
    NextToken;

    ParseBlock(nd->Right);
}

// A If statement
// if (condition)
//      statements
// else if (condition)
//      statements
// ...
// else
//      statements
void ParseIf(Node* &nd)
{
    //Node's value is IF, left child is condition and right child is the series of statements
    CreateTree(nd);
    nd->Value = "IF";

    NextToken;
    if (Token.Str!="(") Error("Enclose the condition in '(...)'", Token.LineStart,Token.Pos);
    NextToken;
    if (ParseExpression(nd->Left,0) != GetType("BOOL"))
        Error("IF expressions must be boolean in type", Token.LineStart);
    if (Token.Str!=")") Error("Enclose the condition in '(...)'", Token.LineStart,Token.Pos);
    NextToken;

    ParseBlock(nd->Right);

    // If followed by an ELSE statement, parse the ELSE tree
    // Node's value is ELSE
    // If Else If, left and right are both IF trees
    // If only Else, left child is previous IF tree and right child is the series of Else statements
    if (Token.Type==EOL && Tokens[tk+1].Str=="ELSE")
    {
        NextToken;NextToken;  // We are now at token "IF" || Else Block
        Node tmp = *nd;
        CreateTree(nd->Left);
        nd->Right = 0;
        *(nd->Left) = tmp;
        nd->Value = "ELSE";
        if (Token.Str=="IF")
            ParseIf(nd->Right);
        else
            ParseBlock(nd->Right);
    }
}

// Return from a function by RETURN statement
// For VOID functions, syntax is only RETURN
// For others, RETURN expression
void ParseReturn(Node* &nd)
{
    CreateTree(nd);
    nd->Value = "RETURN";

    NextToken;
    if (RetType!=GetType("VOID"))
    if (!CheckTypes(ParseExpression(nd->Right,0), RetType))
        Error("Return and Function types mismatched!!!",Token.LineStart);
}

// Parse a variable declaration
// There has been change about array dimensions
// Arrays are no longer same as initialized pointers - they are now same as C/C++ arrays
void ParseVarDecl(Node* &nd)
{
    VarInfo var;
    // Parse the declarance info: type ***... var_name
    ParseDeclare(var);

    // Node's value is DECLARE with the variable's unique id as attribute
    CreateTree(nd);
    nd->Value="DECLARE";

    // Add the variable to current scope
    AddVar(var);
    nd->Attribs[0]=GetVarId(var.Name);
    nd->Attribs[2]=GetTypeName(var.Type);
}

// Check if a variable, that can be assigned to, is coming up
// For e.g.: a or ***a
// Any statement starting with these can only be assignment statements:
// a = 32
// ***a = 32
bool CheckAssgnComing()
{
    // If a variable return true
    if (CheckVar(Token.Str)) return true;
    // Else record the token position, pass through dereference operators, check if variable, reset the token and
    // return true and false accordingly
    unsigned long ptk = tk;
    while (Token.Str=="*")
        NextToken;
    if (CheckVar(Token.Str)) {tk=ptk;return true;}
    tk=ptk;
    return false;
}
// Parse a single statement
void ParseStatement(Node * &nd)
{
    // Check the first token in the statement to determine the type and pass to corresponding function to parse
    if (Token.Str=="IF") ParseIf(nd);
    else if (Token.Str=="WHILE") ParseWhile(nd);
    else if (Token.Str=="RETURN")   ParseReturn(nd);
    else if (CheckType(Token.Str)) ParseVarDecl(nd);
    else if (CheckAssgnComing())   ParseAssign(nd);
    else if (CheckFunction(Token.Str))  ParseCall(nd);
    else
        Error("Unknown Statement !!!", Token.LineStart);
}
// Parse a number of stataments
void ParseStatements(Node * &nd)
{
    // ln>=lines mean we are passed beyond the end of file
    // An ending brace can end a block of statements
    if (Token.Str=="}" || ln>=lines) return;
    // Each STATEMENT node can have a statement on its left and another STATEMENT node on its right
    CreateTree(nd);
    nd->Value = "STATEMENT";
    if (Token.Type!=EOL)
        ParseStatement(nd->Left);
    if (Token.Type!=EOL)    Error("Expected End Of Line", Token.LineStart, Token.Pos);
    // Skip EOL's to get to next statement
    NextStatement();
    ParseStatements(nd->Right);
}

// Parse a function definition
void ParseFunction(Node * &nd)
{
    RetType = ParseType();  // Set the current Return Type so that RETURN statement can be parsed correctly
    // Node's value is FUNCTION with statements for the function on Left and possibly another function node on right
    nd->Value="FUNCTION";
    nd->Attribs[0]=Token.Str;

    // Add the parameters of the function to the scope so that they are treated as normal variables
    AddFuncToScope(Token.Str);

    // Skip to the end of parameters list
    while (Token.Str!=")")  NextToken;
    // Get to next token after ')' and skip EOL's to get to '{'
    NextToken;
    NextStatement();
    // '{' must exist
    if (Token.Str!="{") Error("Expected function block: '{...}'", Token.LineStart, Token.Pos);
    NextToken;
    NextStatement();

    // Parse the statements
    ParseStatements(nd->Left);
    //...
    if (Token.Str!="}") Error("Invalid ending: expected '}'", Token.LineStart, Token.Pos);
    NextToken;
}

// Parse an operator function definition, similar to above with few differences
void ParseOprFunction(Node* &nd)
{
    NextToken;
    bool Series=false;
    if (Token.Str=="SERIES")
    {
        NextToken;
        Series=true;
    }
    else RetType=ParseType();  // Return type is at the beginning if SERIES not defined
    nd->Value="OPR_FUNCTION";
    string name = Token.Str;
    AddOprFuncToScope(Token.Str);

    int LType, RType, PD;

    while (Token.Str!="(")  NextToken;
    NextToken;
    LType = ParseType();
    NextToken;
    if (Token.Str!=")")
    {
        NextToken;
        RType=ParseType();
    }
    if (Series)
        RetType=LType=RType=GetPointedType(RType);

    while (Token.Str!=")")  NextToken;
    NextToken;
    if (Token.Str=="PREUNARY")
    {
        PD = -1;
        RType=LType;
        LType=GetType("VOID");
    }
    else if (Token.Str=="POSTUNARY")
    {
        PD = -1;
        RType=GetType("VOID");
    }
    else PD = ToNum(Token.Str);
    NextToken;

    nd->Attribs[0]=GetOprId(name,LType,RType,PD);

    if (Token.Str=="COMMUTATIVE")   NextToken;

    NextStatement();
    if (Token.Str!="{") Error("Expected function block: '{...}'", Token.LineStart, Token.Pos);
    NextToken;
    NextStatement();

    ParseStatements(nd->Left);
    if (Token.Str!="}") Error("Invalid ending: expected '}'", Token.LineStart, Token.Pos);
    NextToken;
}

// Parse NEW and DELETE function definitions of a type, again similar to those above
void ParseTypeFunction(Node* &nd, int Type)
{
    RetType=ParseType();
    nd->Value="TYPE_FUNCTION";
    string fnc = Token.Str;
    //nd->Attribs[0]=Token.Str;
    nd->Attribs[1]=GetTypeName(Type);

    NextToken;
    NextToken;
    int tp = ParseType();

    if (fnc=="DELETE")
        nd->Attribs[0]=nd->Attribs[1]+"_DEL";
    else
        nd->Attribs[0]=GetNewTypeId(Type, tp);

    AddTypeFuncToScope(Type,nd->Attribs[0]);

    while (Token.Str!=")")  NextToken;
    NextToken;
    NextStatement();
    if (Token.Str!="{") Error("Expected function block: '{...}'", Token.LineStart, Token.Pos);
    NextToken;
    NextStatement();

    ParseStatements(nd->Left);
    if (Token.Str!="}") Error("Invalid ending: expected '}'", Token.LineStart, Token.Pos);
    NextToken;
}

// Parse all the function definitions
void ParseFuncsDefn(Node * &_nd)
{
    RetType=-1;
    Node * nd = 0;
    string InType="";

    bool firstFunc=true;    // is first function
    for (ln=0;ln<lines;ln++)
    {
        if (Token.Type!=EOL)
        {
            if (Token.Str=="TYPE")  {NextToken; InType=Token.Str; SkipLine();}
            if (Token.Str=="}" && InType!="") InType="";
            if (Token.Str=="OPR" || IncomingFunction())
            {
                // Every function is a new scope: child to GlobalScope
                NewScope();
                // For first function, parse at _nd (ParseTree), for rest at nd->Right
                if (firstFunc) {CreateTree(_nd); nd = _nd; firstFunc=false;}
                else {CreateTree(nd->Right); nd = nd->Right;}

                // InType!="" then NEW and DELETE functions
                if (InType!="") ParseTypeFunction(nd, GetType(InType));
                // OPR then operator function
                else if (Token.Str=="OPR") ParseOprFunction(nd);
                // Else normal function
                else ParseFunction(nd);

                //Don't forget to return to GlobalScope
                ParentScope();
            }
            else SkipLine();
        }
        if (Token.Type!=EOL)    Error("Expected End Of Line !!!", Token.LineStart, Token.Pos);
        NextToken;
    }
}
