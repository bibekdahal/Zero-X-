#include "stdinc.h"
#include "tricode.h"

// Some useful functions
bool Final;
vector<tcode> Finals;
void AddFinalCode()
{
    for (unsigned i=0;i<Finals.size();i++)
        AddTCode(Finals[i]);
}
void AddTriCode(string Type, string Opr, string a, string b, string c)
{
    tcode tc;
    tc.Type=Type;
    tc.Opr=Opr;
    tc.c=c;
    tc.a=a;
    tc.b=b;
    if (Final)  Finals.push_back(tc);
    else    AddTCode(tc);
}
void SaveAll()
{
    for (unsigned i=0;i<tosave.size();i++)
        Push(tosave[i]);
}
void GetAll()
{
    for (int i=tosave.size()-1;i>=0;i--)
    {
        string str = tosave[i];
        Pop(str);
        AddTmp(str);
    }
}


// Get the offset for var factors of type:
// var.mem1.mem2.....
// See tutorial post on learntocompile.blogspot.com for more information
int GetOffset(int Type, Node* nd)
{
    if (nd->Value=="MEMBER")
        return (GetMemOffset(Type, nd->Left->Value) + GetOffset(GetType(nd->Left->Attribs[2]), nd->Right));
    else
        return GetMemOffset(Type, nd->Value);
}

// Pass the parameters
void PassParams(Node* nd)
{
    // Push each parameter till no node exists
    if (!nd)    return;
    Push(GetExpr(nd->Left));
    PassParams(nd->Right);
}

// A factor of an expression
// Returns the variable/tmp containing the factor
string GetVarFactor(Node* nd)
{
    // Variable is returned as it is
    if (nd->Attribs[0]=="VARIABLE")
        return nd->Value;
    // For &a, &a is returned
    else if (nd->Value=="PTR")
        return "&"+nd->Right->Value;
    // Values are also returned as it is
    else if (nd->Attribs[0]=="VALUE")
    {
        if (nd->Attribs[2]=="CHAR*")    return "~" + nd->Value;
        return nd->Value;
    }
    // Array Parameter
    else if (nd->Attribs[0]=="ARR_PARAM_VAR")
    {
        // We have to pass the pointer to the array variable
        return "&"+nd->Value;
    }
    //var.mem1.mem2
    else if (nd->Value=="MEMBER")
    {
        string var = GetExpr(nd->Left);
        string id=ToStr(GetOffset(GetType(nd->Left->Attribs[2]), nd->Right));
        string tmp = GetTmp();
        CopyOffSrc(tmp,var, id);
        return tmp;
    }
    // Pointer indices
    else if (nd->Value=="PTR_ARRAY")
    {
        // Get the pointer and the indices
        string var = GetExpr(nd->Left);
        ToSave(var);
            string id = GetExpr(nd->Right);
        ClearSaved();
        // Multiply the indices with the type size to get true offest in bytes
        string tmp;
        if (IsStrNum(id))
            tmp = ToStr((int)(ToNum(id) * GetTypeSize(GetType(nd->Attribs[2]))));
        else
        {
            tmp = GetTmp();
            AddTriCode("ASSIGN", "*", id, ToStr((long)GetTypeSize(GetType(nd->Attribs[2]))),tmp);
        }
        // tmp1 = var[tmp]
        // return tmp1
        string tmp1 = GetTmp();
        CopyIdSrc(tmp1,var, tmp);
        return tmp1;
    }
    // Array Indices
    else if (nd->Value=="ARRAY")
    {
        // The top of ARRAY tree has right child as the last index : for a[i1][i2][i3], the topmost right child is i3
        string id = GetExpr(nd->Right);

        // Find out the type of the array variable, the type lies at the "bottommost" left child of the array node
        Node* _nd = nd;
        // Get to the bottommost Array node
        while(_nd->Left->Value=="ARRAY")
            _nd=_nd->Left;
        // Get the type from the left child
        string arrtp = _nd->Left->Attribs[2];

        _nd = nd;
        // Get the no. of dimensions
        int i=GetArrayIdCnt(GetType(arrtp));
        long dsz=1;
        // To get the total offset from indices: a[i1][i2][i3] where size of each dimension is D1, D2, D3
        // offset = i3 + i2*D3 + i1*D2*D3
        // Note that the first dimension (D1) is never used
        // We already got the 'i3'
        // Note that 'i' is the current dimension we are working at
        //      for 3 dimensions, 'i' is currently set to '3'
        // Get remaining indices
        while (_nd->Left->Value=="ARRAY")
        {
            // Get the i-th dimension size and multiply it with previous dimension sizes
            //          Why? Check the formula - for i2 we multiply by D3, for i1, we have to multiply by D2*D3
            dsz *= GetDimSize(arrtp, i);
            // We have to work on next dimension now, decrement 'i'
            i--;
            _nd = _nd->Left;
            // Get the next index
            ToSave(id);
                string idd = GetExpr(_nd->Right);
            ClearSaved();
            // Multiply the index with 'dsz'
            if (IsStrNum(idd))  idd = ToStr((long)ToNum(idd) * dsz);
            else
            {
                string tmp = GetTmp();
                AddTriCode("ASSIGN","*", idd, ToStr(dsz), tmp);
                idd = tmp;
            }

            // Add to get updated offset, stored in 'id'
            // i.e. (i2) + (i1*D2)
            if (IsStrNum(id) && IsStrNum(idd))
                id = ToStr((long) (ToNum(idd)+ToNum(id)));
            else
            {
                string tmp = GetTmp();
                AddTriCode("ASSIGN","+", id, idd, tmp);
                id = tmp;
            }
        }

        // idt is total offset, we get by multiplying above offset (id) with type size
        string idt;
        if (IsStrNum(id))
            idt = ToStr((int)(ToNum(id) * GetTypeSize(GetType(nd->Attribs[2]))));
        else
        {
            idt = GetTmp();
            AddTriCode("ASSIGN", "*", id, ToStr((long)GetTypeSize(GetType(nd->Attribs[2]))),idt);
        }
        // Then use that address with the offset to get to the required data
        string tmp = GetTmp();
        // An array as parameter is pointer to array and uses index
        // while normal array variable uses offset
        if (_nd->Left->Attribs[1]=="PARAM") {CopyIdSrc(tmp, _nd->Left->Value, idt);}
        else    {CopyOffSrc(tmp, _nd->Left->Value, idt);}
        return tmp;
    }
    // ****a
    else if (nd->Value=="DEREF")
    {
        // *a is same as a[0]
        string var = GetVarFactor(nd->Right);
        string id = "0";
        // NewTmp = var[0]
        // return NewTmp
        string tmp = GetTmp();
        CopyIdSrc(tmp,var, id);
        return tmp;
    }
    // Calling a function in parameter
    else if (nd->Attribs[0]=="FUNCTION")
    {
        // Pass the parameters
        PassParams(nd->Right);
        // NewTmp = func(parameters)
        // return NewTmp
        string tmp = GetTmp();
        SaveAll();
            CopyCall(tmp,nd->Value, ToStr((long)GetParamSize(nd->Value)));
        GetAll();
        return tmp;
    }

    return "INVALID_FACTOR";
}

int OprIndex(string OprId)
{
    for (unsigned i=0;i<Oprs.size();i++)
        if (Oprs[i].Id==OprId)   return i;
    return -1;
}

// Get the number of expressions to passed for a series type of operator
int GetNOprSeries(Node* nd)
{
    // 1 for the right expression
    int N=1;
    // In case the left child is same operator, the children of the left child is also the part of the series
    if (nd->Left->Attribs[0]=="OPERATOR" && nd->Left->Value==nd->Value)
        N+=GetNOprSeries(nd->Left);
    // Else add 1 for the left expression
    else
        N+=1;
    return N;

}
// Store the series of expressions in the tmp variable
void StoreOprSeries(Node* nd, int N, string tmp)
{
    OprInfo opr = Oprs[OprIndex(nd->Value)];
    // If further same operator exists at the left child, store the expression children of that operator as well
    if (nd->Left->Attribs[0]=="OPERATOR" && nd->Left->Value==nd->Value)
        StoreOprSeries(nd->Left, N-1, tmp);
    // If we have got expressions on both left and right children to store at tmp
    else
    {
        // Store the left child at the (N-2)th position
        // tmp[(N-2)*TypeSize] = Expr
        ToSave(tmp);
            CopyIdDest(tmp,GetExpr(nd->Left),ToStr((long)GetTypeSize(opr.RetType)*(N-2)));
        ClearSaved();
    }
    // Stoe the right child at (N-1)th position... i.e. after the left child
    ToSave(tmp);
        CopyIdDest(tmp,GetExpr(nd->Right),ToStr((long)GetTypeSize(opr.RetType)*(N-1)));
    ClearSaved();
}


bool IsPredefinedOpr(string opr)
{
    // Predefined operators donot have '_' at the end
    if (opr[opr.size()-1]!='_')
        return true;
    return false;
}

// Generate tcodes out of expression tree
// Returns a tmp or variable which has the result of the expression
string GetExpr(Node* nd)
{
    // No tree, return empty string as result
    if (!nd)    return "";

    // An operator
    else if (nd->Attribs[0]=="OPERATOR" || nd->Attribs[0]=="UNARY_OPERATOR")
    {
        // A dereference operator
        if (nd->Value[0]=='*' && !(nd->Left) && IsPointer(GetType(nd->Right->Attribs[2])))
        {
            // *a is same as a[0]
            // NewTmp = a[0]
            // return NewTmp
            string var = GetExpr(nd->Right);
            string id = "0";
            string tmp = GetTmp();
            CopyIdSrc(tmp,var, id);
            return tmp;
        }
        // Predefined Numerical or Boolean operators
        else if (IsPredefinedOpr(nd->Value))
        {
            // Get the two expression in src1 and src2
            string src1 = GetExpr(nd->Left);
            // Make sure we save src1 before doing src2, since src2 may contain code to reset tmps
            ToSave(src1);
                string src2 = GetExpr(nd->Right);
            ClearSaved();
            // NewTmp = src1 opr src2
            // return NewTmp
            string opr = nd->Value;
            string tmp = GetTmp();
            Assign(tmp, src1, opr, src2);
            return tmp;
        }
        // For programmed operators, we need to call the operator function
        else
        {
            OprInfo opr = Oprs[OprIndex(nd->Value)];
            // If not series type of operator
            if (!opr.Series)
            {
                // Push the results of two expression children

                // N is the size in bytes of the LType + RType
                int N;
                // For pre/post operators push only one expression
                if (opr.LType==GetType("VOID")) {Push(GetExpr(nd->Right)); N=GetTypeSize(opr.RType);}
                else if (opr.RType==GetType("VOID")) {Push(GetExpr(nd->Left)); N=GetTypeSize(opr.LType);}
                // For others, push both expressions
                else
                {
                    N=GetTypeSize(opr.LType);
                    N+=GetTypeSize(opr.RType);
                    Push(GetExpr(nd->Right));
                    Push(GetExpr(nd->Left));
                }

                // NewTmp = opr_func(LExpr, RExpr)
                // return NewTmp
                string tmp = GetTmp();
                SaveAll();
                    CopyCall(tmp,nd->Value, ToStr(N));
                GetAll();
                return tmp;
            }
            // For series type of operator
            else
            {
                // Get the no of the expressions in the series
                int N = GetNOprSeries(nd);
                // stmp = Allocate(N*TypeSize_in_bytes)
                string stmp = GetTmp();
                SaveAll();
                    Allocate(stmp, ToStr((long)GetTypeSize(opr.RetType)*N));
                GetAll();
                // Store the N no. of expressions in the stmp variable
                StoreOprSeries(nd, N, stmp);
                // Push stmp, the array
                Push(stmp);
                // Push the no. of elements as well
                Push(ToStr(N));
                // Tmp = opr_func(N, stmp)
                // return Tmp
                string tmp = GetTmp();
                SaveAll();
                    CopyCall(tmp,nd->Value, "8");
                GetAll();
                return tmp;
            }
        }
    }
    else return GetVarFactor(nd);
}

// Assignment variable of assignment statement
// tc is the tcode for assignment
// tc.c is the assignent variable
void AsgnTo(Node* nd, tcode &tc)
{
    if (nd->Attribs[0]=="VARIABLE")
    {
        // The variable is itself is to be assigned to
        tc.c = nd->Value;
        // And the operation is simple COPY
        tc.Opr="COPY";
    }
    else if (nd->Value=="MEMBER")
    {
        // var.m1.m2 = ...
        tc.c = nd->Left->Value;
        tc.b = ToStr(GetOffset(GetType(nd->Left->Attribs[2]), nd->Right));
        // tc.c contains variable to be assigned to and tc.b contains the offset to reach the member
        // Since the destination variable is using an offset, COPY_OFF_DEST is used
        tc.Opr="COPY_OFF_DEST";
    }
    else if (nd->Value=="PTR_ARRAY")
    {
        // For pointer indices
        // Get the variable to be assigned to
        string var = GetVarFactor(nd->Left);
        // Get the index
        ToSave(var);
            string id = GetExpr(nd->Right);
        ClearSaved();

        string tmp;
        // Multiply the ID with the size of the type and store in tmp
        // That's because for int * a
        // a[0] is at 0*4=0 position
        // a[1] is at 1*4=4 position
        // ... since each data occupies 4 bytes as integer
        if (IsStrNum(id))
            tmp = ToStr((int)(ToNum(id) * GetTypeSize(GetType(nd->Attribs[2]))));
        else
        {
            tmp = GetTmp();
            AddTriCode("ASSIGN", "*", id, ToStr((long)GetTypeSize(GetType(nd->Attribs[2]))),tmp);
        }
        // The var pointing destination to copy to
        tc.c = var;
        // The id of where to copy is tmp
        tc.b = tmp;
        // Copy to destination variable with index
        tc.Opr="COPY_ID_DEST";
    }
    else if (nd->Value=="ARRAY")
    {
        // Plz check out corresponding code in GetVarFactor for the commented part of this code
        string id = GetExpr(nd->Right);

        Node* _nd = nd;
        while(_nd->Left->Value=="ARRAY")
            _nd=_nd->Left;
        string arrtp = _nd->Left->Attribs[2];

        _nd = nd;
        int i=GetArrayIdCnt(GetType(arrtp));
        long dsz=1;

        while (_nd->Left->Value=="ARRAY")
        {
            dsz *= GetDimSize(arrtp, i);
            i--;
            _nd = _nd->Left;

            ToSave(id);
                string idd = GetExpr(_nd->Right);
            ClearSaved();

            if (IsStrNum(idd))  idd = ToStr((long)ToNum(idd) * dsz);
            else
            {
                string tmp = GetTmp();
                AddTriCode("ASSIGN","*", idd, ToStr(dsz), tmp);
                idd = tmp;
            }

            if (IsStrNum(id) && IsStrNum(idd))
                id = ToStr((long) (ToNum(idd)+ToNum(id)));
            else
            {
                string tmp = GetTmp();
                AddTriCode("ASSIGN","+", id, idd, tmp);
                id = tmp;
            }
        }

        string idt;
        if (IsStrNum(id))
            idt = ToStr((int)(ToNum(id) * GetTypeSize(GetType(nd->Attribs[2]))));
        else
        {
            idt = GetTmp();
            AddTriCode("ASSIGN", "*", id, ToStr((long)GetTypeSize(GetType(nd->Attribs[2]))),idt);
        }

        tc.c = _nd->Left->Value;
        tc.b = idt;
        if (_nd->Left->Attribs[1]=="PARAM") tc.Opr = "COPY_ID_DEST";
        else tc.Opr = "COPY_OFF_DEST";
    }
    else if (nd->Value=="DEREF")
    {
        // Dereference * operator at the assignment variable
        // *a is same as a[0]... copy to variable with index 0
        tc.c = GetVarFactor(nd->Right);
        tc.b = "0";
        tc.Opr="COPY_ID_DEST";
    }

    else
    {
        tc.c = "INVALID_ASGN_VAR";
        tc.Opr="COPY";
    }

}

// For If... Else... statements
void GetIfElse(Node* nd, string X) // Note that: X is a label after the whole IF..ELSE_If...ELSE... code
{
    // No ELSE, only IF, jump to the end(X) if the condition is false
    if (nd->Value=="IF")
    {
        string expr = GetExpr(nd->Left);
        JmpIfFalse(expr,X);     // Jump to X if false
        MakeTriCode(nd->Right);
    }
    // ELSE node with left and right children
    else
    {
        // Get a new label for the end of left child node
        string Y = GetLabel();
        // Since left child is always a IF statement, Y is jumped to when left IF condition is false
        GetIfElse(nd->Left, Y);
        // Supposing the left IF condition to be true, we don't jump to Y, but get up to here
        Jmp(X);     //Jump to X(the end of whole IF..Else..code) if left child is true
        // Here is the label Y.. jumped to only when left IF of current ELSE is false
        Label(Y);
        // In case the right child is ELSE or IF node, do GetIfElse again
        if (nd->Right->Value=="ELSE" || nd->Right->Value=="IF")
            GetIfElse(nd->Right,X); // X is the end of If..ElseIf..Else.. code
        // Otherwise, this is the last ELSE statement with no right child, just generate the tcodes
        else
            MakeTriCode(nd->Right);
    }
}

// Construct a variable
void Alloc(string var, int type)
{
    // In case the constructor NEW is defined
    // which must be declared with the first parameter as pointer to the same type where it is declared
    string ctor = GetCTor(type);
    if (ctor!="")
    {
        // The parameter is NULL (since we don't have anything to copy from), which is interpreted as '0'
        Push("0");
        // Call the tp_new function and copy it to the newly created variable
        // Number of parameters is 1 and the size for it is 4 bytes
        CopyCall(var,ctor,"4");
    }
}
// Destroy a variable
void Dealloc(string var, int type)
{
    // Make sure this code goes at the end
    Final=true;
    // Only if DEL function is defined
    if (Types[type].Del)
    {
        // Push the pointer to the variable
        Push("&"+var);
        // Call the destructor function
        Call(Types[type].Name + "_DEL","4");
    }
    // Reset Final
    Final=false;
}

// TCode for Declare tree
void Declare(Node* nd)
{
    // Declare the variable
    string var = nd->Attribs[0];
    string type = nd->Attribs[2];
    AddTriCode("DECLARE", "", var, type, "");

    // We may need to allocate and deallocate the variable if appropriate NEW and DEL functions are defined
    int tp = GetType(type);
    Alloc(var, tp);
    Dealloc(var, tp);
}

// Make 3-address code out of the given tree node
void MakeTriCode(Node* nd)
{
    // No node, don't do anything
    if (!nd)    return;

    // Functions, start a new block of tcodes...
    if (nd->Value=="FUNCTION" || nd->Value=="OPR_FUNCTION" || nd->Value=="TYPE_FUNCTION")
    {
        NewBlock(nd->Attribs[0],nd->Value);
        // The tcodes for the function definition in this new block
        MakeTriCode(nd->Left);
        // Add the tcodes that are to be at the end of the function definition
        AddFinalCode();
        Finals.clear();
        // Now make tcodes for next function
        MakeTriCode(nd->Right);
    }
    // Statement node ... is easier
    else if (nd->Value=="STATEMENT")
    {
        MakeTriCode(nd->Left);
        MakeTriCode(nd->Right);
    }
    // Declare a variable
    else if (nd->Value=="DECLARE")
    {
        Declare(nd);
    }
    // Assign to a variable
    else if (nd->Value=="ASSIGN")
    {
        // A new tcode of type ASSIGN
        tcode tc;
        tc.Type="ASSIGN";
        // Left child contains the variable to be assigned to
        AsgnTo(nd->Left, tc);
        // c = Assignment variable
        // a = Expression
        tc.a = GetExpr(nd->Right);
        // Add this tcode to current block
        AddTCode(tc);
    }
    // IF....
    else if (nd->Value=="IF")
    {
        // Get the label that is pasted at the end of the whole IF...ELSE... statements
        string lbl = GetLabel();
        // Generate tcodes for If...Else... blocks
        // Also pass the label that is going to be at the end
        GetIfElse(nd,lbl);
        // Paste the label at the end
        Label(lbl);
    }
    // While...
    else if (nd->Value=="WHILE")
    {
        // Get two labels,  one at the top of the WHILE loop
        //                  another at the end of the WHILE loop
        string X = GetLabel();
        string Y = GetLabel();
        // Paste one of the labels before the loop
        Label(Y);
        // Get the CONDITION expression
        string expr = GetExpr(nd->Left);
        // If the epxression turns out to be false, we need to get out of the loop
        JmpIfFalse(expr,X);     // Jump to X if false
        // Generate tcodes for the WHILE block
        MakeTriCode(nd->Right);
        // Jump to the beginning of the loop, re-do everything
        Jmp(Y);
        // The label at the end of the WHILE loop
        Label(X);

    }
    // Calling a function
    else if (nd->Value=="CALL")
    {
        // Pass the parameters
        PassParams(nd->Right);
        // No tmps to save.. still making sure
        SaveAll();
            Call(nd->Attribs[0], ToStr((long)GetParamSize(nd->Attribs[0])));
        GetAll();
    }
    // Returning from a function...
    else if (nd->Value=="RETURN")
        Return(GetExpr(nd->Right));
}
