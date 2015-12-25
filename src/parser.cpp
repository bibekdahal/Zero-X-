#include "stdinc.h"
#include "stageI.h"
#include "stageII.h"
#include "parser.h"

// Calculate the result of operation on two children constants
string Calculate(Node* nd)
{
    double a, b;
    a = ToNum(nd->Left->Value);
    b = ToNum(nd->Right->Value);

    // Make sure INTEGRAL operations are performed for operators that do not return DOUBLE or FLOAT
    if (nd->Attribs[2]!="DOUBLE" && nd->Attribs[2]!="FLOAT")
    {
        if (nd->Value=="+")
            return ToStr((int)a+(int)b);
        else if (nd->Value=="-")
            return ToStr((int)a-(int)b);
        else if (nd->Value=="*")
            return ToStr((int)a*(int)b);
        else if (nd->Value=="/")
            return ToStr((int)a/(int)b);
    }
    else
    {
        if (nd->Value=="+")
            return ToStr(a+b);
        else if (nd->Value=="-")
            return ToStr(a-b);
        else if (nd->Value=="*")
            return ToStr(a*b);
        else if (nd->Value=="/")
            return ToStr(a/b);
    }

    return "";
}
// A predefined numerical operator
bool IsPreNumber(Node* nd)
{
    if (!IsNumber(GetType(nd->Attribs[2])))  return false;
    // Operators are always used in parse tree using their IDs
    // For the predefined operators, their IDs are the operator's names themselves
    if (nd->Value=="+" || nd->Value=="-" || nd->Value=="*" || nd->Value=="/")
        return true;
    return false;
}
// Optimize the parse tree by calculating the operators with children numerical constants by the compiler itself
// Also make sure that numerical OPERATOR nodes have been assigned proper type
void OptmConstExpr(Node* &nd)
{
    if (!nd)    return;
    OptmConstExpr(nd->Left);
    OptmConstExpr(nd->Right);

    // Translate the CHARACTER constant ('A') to numerical counterpart (65)
    if (nd->Attribs[0]=="VALUE" && nd->Attribs[2]=="CHAR")
        nd->Value = ToStr((int)nd->Value[0]);

    // For operators
    if (nd->Attribs[0]=="OPERATOR")
    {
        // In case of predefined numerical operators only
        if (IsPreNumber(nd))
        {
            // First change the type of the operator according to the types of children
            // GreaterNumType choses the proper type: for e.g.: in case of DOUBLE and INT, DOUBLE is chosen, whereas
            //                                                  in case of INT and CHAR, INT is chosen
            nd->Attribs[2] = GetTypeName(GetGreaterNumType(GetType(nd->Left->Attribs[2]), GetType(nd->Right->Attribs[2])));

            // If both children are constants
            if (nd->Left->Attribs[0]=="VALUE" && nd->Right->Attribs[0]=="VALUE")
            {
                // Calculate the node
                string str = Calculate(nd);
                if (str!="")
                {
                    // Change the node's value (which is currently an operator) to the calculated value
                    nd->Value=str;
                    // Node's type is now VALUE (constant), not an operator
                    nd->Attribs[0]="VALUE";
                    // We no more need the children
                    DeleteTree(nd->Left); DeleteTree(nd->Right);
                }
            }
        }
    }
}

void Parse()
{
    tk=0;
    ParseTypesDecl();
    tk=0;
    ParseFuncsGlobalsDecl();

    // VOID MAIN(){...} must exist
    bool GotMain=false;
    if (CheckFunction("MAIN"))
        if (GetFuncType("MAIN")==GetType("VOID"))
            if (GetNoOfParam("MAIN")==0)    GotMain = true;
    if (!GotMain) Error("Must have a \"void main()\" function");

    tk=0;
    ParseFuncsDefn(ParseTree);

    // Get a better parse tree
    OptmConstExpr(ParseTree);
}
