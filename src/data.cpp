
#include "stdinc.h"
#include "common.h"

unsigned long tk;

void DeleteTree(Node*& pt)
{
    if (!pt) return;
    DeleteTree(pt->Left);
    DeleteTree(pt->Right);
    delete pt;
    pt=0;
}
void CreateTree(Node*& pt)
{
    pt = new Node();
    pt->Value="";
    pt->Attribs[0]=pt->Attribs[1]=pt->Attribs[2]="";
    pt->Left=pt->Right=0;
}

Node* ParseTree=0;

Scope GlobalScope;
Scope* CurrentScope;

vector<TypeInfo> Types;
vector<FuncInfo> Funcs;
int RetType = -1;
vector<OprInfo> Oprs;

#include "setup.h"

// Sorry, but no code is explained here
// See the HEADER file for some short explaination
// If help required, comment on: learntocompile.blogspot.com

void InitData()
{
    GlobalScope.Id = "_";
    GlobalScope.Parent=0;
    CurrentScope = &GlobalScope;

    Setup();
}

bool IsNumber(int Type)
{
    string nm = Types[Type].Name;
    if (nm=="DOUBLE" || nm=="INT" || nm=="CHAR" || nm=="FLOAT")
        return true;
    return false;
}
bool CheckTypes(int Type1, int Type2)
{
    if (Type1==Type2) return true;
    if (IsPointer(Type1) && IsPointer(Type2)) return CheckTypes(GetPointedType(Type1), GetPointedType(Type2));
    if (IsNumber(Type1) && IsNumber(Type2))
        return true;
    return false;
}
bool CheckAssgnTypes(int Type1, int Type2)
{
    if (CheckTypes(Type1, Type2)) return true;
    for (unsigned i=0;i<Types[Type1].NewFuncs.size();i++)
    {
        int tp = GetPointedType(Types[Type1].NewFuncs[i].Params[0].Type);
        if (CheckTypes(tp, Type2))  return true;
    }
    return false;
}
bool CheckFuncArrParam(int Type1, int Type2)
{
    if (GetArrayIdCnt(Type1)!=GetArrayIdCnt(Type2)) return false;
    string str1 = GetTypeName(Type1);
    string str2 = GetTypeName(Type2);
    size_t found1 = str1.find('@', 1);   //Find second occurence of '@'
    size_t found2 = str2.find('@', 1);

    str1 = str1.substr(found1);
    str2 = str2.substr(found2);
    if (str1.substr(found1)==str2.substr(found2))   return true;

    return false;
}
bool IsPointer(int Type)
{
    if (Type==-1) return false;
    string str = GetTypeName(Type);
    if (str.at(str.length()-1)=='*')    return true;
    return false;
}
int GetPointedType(int Type)
{
    string str = GetTypeName(Type);
    return GetType(str.substr(0,str.length()-1));
}
int GetArrType(int Type)
{
    string str = GetTypeName(Type);
    size_t found;
    found = str.find('@');
    size_t lf=string::npos;
    while (found!=string::npos)
    {
        lf=found;
        found = str.find('@', found+1);
    }
    if (lf==string::npos)   return -1;
    return GetType(str.substr(lf+2));

}
int GetArrayIdCnt(int Type)
{
    string str = GetTypeName(Type);
    size_t found;
    int cnt=0;
    found = str.find('@');
    while (found!=string::npos)
    {
        cnt++;
        found = str.find('@', found+1);
    }
    return cnt;
}

bool CheckVarInScope(string Name, Scope *scope)
{
    for (unsigned i=0;i<scope->Vars.size();i++)
        if (scope->Vars[i].Name==Name) return true;
    if (scope->Parent)  return CheckVarInScope(Name, scope->Parent);
    return false;
}

bool CheckReserved(string Name)
{
    if (Name=="NEW" || Name=="DELETE" || Name=="IF" || Name=="ELSE"
         || Name=="WHILE" || Name=="RETURN" || Name=="TYPE")
        return true;
    return false;
}
bool CheckValidName(string Name)
{
    if (CheckVarInScope(Name, CurrentScope))    return false;
    if (CheckFunction(Name))    return false;
    if (CheckType(Name))    return false;
    if (CheckReserved(Name))    return false;
    if (CheckOpr(Name)) return false;
    return true;
}
bool CheckValidOpr(string Name, int LType, int RType, int PD)
{
    if (CheckVarInScope(Name, CurrentScope))    return false;
    if (CheckFunction(Name))    return false;
    if (CheckType(Name))    return false;
    if (CheckReserved(Name))    return false;

    for (unsigned i=0;i<Oprs.size();i++)
        if (Oprs[i].Name==Name  &&  PD==Oprs[i].Precedence
            && CheckTypes(Oprs[i].LType,LType) && CheckTypes(Oprs[i].RType,RType))
            return false;
    return true;
}

bool CheckVar(string Name)
{
    return CheckVarInScope(Name, CurrentScope);
}
int GetVarType(string Name, Scope* scope)
{
    for (unsigned i=0;i<scope->Vars.size();i++)
        if (scope->Vars[i].Name==Name) return scope->Vars[i].Type;
    if (scope->Parent)  return GetVarType(Name, scope->Parent);
    return -1;
}
string GetVarId(string Name, Scope* scope)
{
    for (unsigned i=0;i<scope->Vars.size();i++)
        if (scope->Vars[i].Name==Name) return scope->Vars[i].Id;
    if (scope->Parent)  return GetVarId(Name, scope->Parent);
    return "";
}

bool CheckFunction(string Name)
{
    for (unsigned i=0;i<Funcs.size();i++)
        if (Funcs[i].Name==Name) return true;
    return false;
}
int GetFuncType(string Name)
{
    for (unsigned i=0;i<Funcs.size();i++)
        if (Funcs[i].Name==Name) return Funcs[i].Type;
    return -1;
}
unsigned long GetParamSize(string Function)
{
    for (unsigned i=0;i<Funcs.size();i++)
        if (Funcs[i].Name==Function) return Funcs[i].ParamSize;
    return -1;
}
int GetNoOfParam(string Function)
{
    for (unsigned i=0;i<Funcs.size();i++)
        if (Funcs[i].Name==Function) return Funcs[i].Params.size();
    return -1;
}

bool CheckParameter(string Function, string Name)
{
    for (unsigned i=0;i<Funcs.size();i++)
        if (Funcs[i].Name==Function)
        {
            for (unsigned j=0;j<Funcs[i].Params.size();j++)
                if (Funcs[i].Params[j].Name==Name) return true;
            return false;
        }
    return false;
}
int GetParamType(string Function, int ParamId)
{
    for (unsigned i=0;i<Funcs.size();i++)
        if (Funcs[i].Name==Function)    return Funcs[i].Params[ParamId].Type;
    return -1;
}
int GetParamType(string Function, string Param)
{
    for (unsigned i=0;i<Funcs.size();i++)
        if (Funcs[i].Name==Function)
        {
            for (unsigned j=0;j<Funcs[i].Params.size();j++)
                if (Funcs[i].Params[j].Name==Param) return Funcs[i].Params[j].Type;
            return -1;
        }
    return -1;
}

bool CheckType(string Name)
{
    for (unsigned i=0;i<Types.size();i++)
        if (Types[i].Name==Name) return true;
    return false;
}
int GetType(string Type)
{
    for (unsigned i=0;i<Types.size();i++)
        if (Types[i].Name==Type) return i;
    return -1;
}
string GetTypeName(int Type)
{
    if (Type<0 || Type>=(signed)Types.size())return "";
    return Types[Type].Name;
}
unsigned long GetTypeSize(int Type)
{
    if (Type<0 || Type>=(signed)Types.size())return -1;
    return Types[Type].Size;
}
int GetNoOfMember(int Type)
{
    if (Type<0 || Type>=(signed)Types.size())return -1;
    return Types[Type].Members.size();
}


bool CheckMember(int Type, string Name)
{
    if (Type<0 || Type>=(signed)Types.size())return false;
    for (int j=0;j<(signed)Types[Type].Members.size();j++)
        if (Types[Type].Members[j].Name==Name) return true;
    return false;
}
int GetMemberType(int Type, string Member)
{
    if (Type<0 || Type>=(signed)Types.size())return -1;
    for (unsigned j=0;j<Types[Type].Members.size();j++)
        if (Types[Type].Members[j].Name==Member) return Types[Type].Members[j].Type;
    return -1;
}
int GetMemOffset(int Type, string Member)
{
    if (Type<0 || Type>=(signed)Types.size())return -1;
    int offset=0;
    for (unsigned j=0;j<Types[Type].Members.size();j++)
    {
        if (Types[Type].Members[j].Name==Member) break;
        offset+= GetTypeSize(Types[Type].Members[j].Type);
    }
    return offset;
}

string GetOprId(string Name, int LType, int RType, int PD)
{
    for (unsigned i=0;i<Oprs.size();i++)
        if (Oprs[i].Name==Name  &&  PD==Oprs[i].Precedence
            && CheckTypes(Oprs[i].LType,LType) && CheckTypes(Oprs[i].RType,RType))
            return (Oprs[i].Id=="") ? Name : Oprs[i].Id;
    return "";
}
bool CheckOpr(string Name)
{
    for (unsigned i=0;i<Oprs.size();i++)
        if (Oprs[i].Name==Name) return true;
    return false;
}
bool CheckUnaryOpr(string Name, bool Pre)
{
    for (unsigned i=0;i<Oprs.size();i++)
        if (Oprs[i].Name==Name && Oprs[i].Unary &&
            (Pre?Oprs[i].LType==GetType("VOID"):Oprs[i].RType==GetType("VOID"))
            ) return true;
    return false;
}
bool CheckOpr(string Name, int PD)
{
    for (unsigned i=0;i<Oprs.size();i++)
        if (Oprs[i].Name==Name && Oprs[i].Precedence == PD && !Oprs[i].Unary) return true;
    return false;
}
int GetRetType(string Opr, int LType, int RType, int PD)
{
    for (unsigned i=0;i<Oprs.size();i++)
        if (Oprs[i].Name==Opr  &&  PD==Oprs[i].Precedence
            && CheckTypes(Oprs[i].LType,LType) && CheckTypes(Oprs[i].RType,RType))
            return Oprs[i].RetType;
    return -1;
}
int GetRetType(string Opr, int Type, bool Pre)
{
    for (unsigned i=0;i<Oprs.size();i++)
        if (Oprs[i].Name==Opr &&
            (Pre?
                (Oprs[i].LType==GetType("VOID") && CheckTypes(Oprs[i].RType,Type)):
                (Oprs[i].RType==GetType("VOID")&& CheckTypes(Oprs[i].LType,Type))
            ))
            return Oprs[i].RetType;
    return -1;
}

int AddType(TypeInfo Type)
{
    Types.push_back(Type);
    return Types.size()-1;
}
void ChangeType(int tp, TypeInfo Type)
{
    Types[tp]=Type;
}
void AddTypeNewFunc(int Type, FuncInfo Func)
{
    Types[Type].NewFuncs.push_back(Func);
}
void AddTypeDelFunc(int Type, FuncInfo Func)
{
    Types[Type].Del=true;
    Types[Type].DelFunc=Func;
}
string GetNewTypeId(int Type, int ParamTp)
{
    for (unsigned i=0;i<Types[Type].NewFuncs.size();i++)
         if (CheckTypes(Types[Type].NewFuncs[i].Params[0].Type, ParamTp))
            return Types[Type].NewFuncs[i].Id;
    return "";
}

void AddOpr(OprInfo Opr)
{
    string id="_";
    for (unsigned i=0;i<Oprs.size();i++)
        if (Oprs[i].Name==Opr.Name)
            id=id+"_";
    Opr.Id = Opr.Name + id;

    Oprs.push_back(Opr);
}
void AddFunction(FuncInfo Func)
{
    Funcs.push_back(Func);
}
void AddVar(VarInfo Var, Scope* scope)
{
    Var.Id = scope->Id+Var.Name;
    scope->Vars.push_back(Var);
}

void NewScope()
{
    Scope * scope = new Scope;
    scope->Parent = CurrentScope;
    string id="";
    for (unsigned i=0;i<scope->Parent->Children.size();i++)
        id=id+"_";
    scope->Id = scope->Parent->Id + id;
    CurrentScope->Children.push_back(scope);
    CurrentScope = scope;
}
void ParentScope()
{
    CurrentScope = CurrentScope->Parent;
}
void AddFuncToScope(string Function)
{
    for (unsigned i=0;i<Funcs.size();i++)
        if (Funcs[i].Name==Function)
            for (unsigned j=0;j<Funcs[i].Params.size();j++)
            {
                Funcs[i].Params[j].Id = CurrentScope->Id + Funcs[i].Params[j].Name;
                CurrentScope->Vars.push_back(Funcs[i].Params[j]);
            }
}
void AddOprFuncToScope(string Opr)
{
    for (unsigned i=0;i<Oprs.size();i++)
        if (Oprs[i].Name==Opr)
            for (unsigned j=0;j<Oprs[i].OprFunc.Params.size();j++)
            {
                Oprs[i].OprFunc.Params[j].Id = CurrentScope->Id + Oprs[i].OprFunc.Params[j].Name;
                CurrentScope->Vars.push_back(Oprs[i].OprFunc.Params[j]);
            }
}
void AddTypeFuncToScope(int Type, string FuncId)
{
    if (FuncId==GetTypeName(Type)+"_DEL")
        for (unsigned j=0;j<Types[Type].DelFunc.Params.size();j++)
        {
            Types[Type].DelFunc.Params[j].Id = CurrentScope->Id + Types[Type].DelFunc.Params[j].Name;
            CurrentScope->Vars.push_back(Types[Type].DelFunc.Params[j]);
        }
    else
    {
        size_t id = -1;
        for (unsigned i=0;i<Types[Type].NewFuncs.size();i++)
            if (Types[Type].NewFuncs[i].Id==FuncId) id = i;
        for (unsigned j=0;j<Types[Type].NewFuncs[id].Params.size();j++)
        {
            Types[Type].NewFuncs[id].Params[j].Id = CurrentScope->Id + Types[Type].NewFuncs[id].Params[j].Name;
            CurrentScope->Vars.push_back(Types[Type].NewFuncs[id].Params[j]);
        }
    }
}




void Deallocate()
{
    DeleteTree(ParseTree);

}

void PrintOutTypes()
{
    for (unsigned i=0; i<Types.size();i++)
    {
        cout << "\n\tFOUND TYPE: " << Types[i].Name;
        if (Types[i].Members.size()>0)  cout << "\n\t\tMembers:";
        for (unsigned j=0;j<Types[i].Members.size();j++)
            cout    << "\n\t\t\t" << Types[i].Members[j].Name << "==>"
                    << GetTypeName(Types[i].Members[j].Type);
    }
}
void PrintOutGlobals()
{
    for (unsigned j=0;j<CurrentScope->Vars.size();j++)
    {
        cout << "\n\tFOUND GLOBAL VARIABLE";
        cout << "\n\t\t" << CurrentScope->Vars[j].Name << "==>"
             << GetTypeName(CurrentScope->Vars[j].Type);

    }

}
void PrintOutFuncs()
{
    for (unsigned i=0; i<Funcs.size();i++)
    {
        cout << "\n\tFOUND FUNCTION: " << Funcs[i].Name;
        cout << "\n\t\tType:"+GetTypeName(Funcs[i].Type);
        if (Funcs[i].Params.size()>0)  cout << "\n\t\tParameters:";
        for (unsigned j=0;j<Funcs[i].Params.size();j++)
            cout    << "\n\t\t\t" << Funcs[i].Params[j].Name << "==>"
                    << GetTypeName(Funcs[i].Params[j].Type);
    }
}
void PrintOutOprs()
{
    for (unsigned i=0; i<Oprs.size();i++)
    {
        cout << "\n\tFOUND OPERATOR: " << Oprs[i].Name;
        cout << "\n\t\tRetType:"+GetTypeName(Oprs[i].RetType);
        cout << "\n\t\tLType:"+GetTypeName(Oprs[i].LType);
        cout << "\n\t\tRType:"+GetTypeName(Oprs[i].RType);
        cout << "\n\t\tPrecedence:"+ToStr(Oprs[i].Precedence);
    }
}
void PrintOutTree(Node * nd, string dash)
{
    if (!nd)    return;

    cout << "\n" << dash+"-> " << nd->Value;
    string Atrbs="";
    if (nd->Attribs[0]!="") Atrbs += "  "+nd->Attribs[0];
    if (nd->Attribs[1]!="") Atrbs += "  "+nd->Attribs[1];
    if (nd->Attribs[2]!="") Atrbs += "  "+nd->Attribs[2];
    if (Atrbs!="") cout << "\tAttributes:" << Atrbs;

    PrintOutTree(nd->Left, dash+"-");
    PrintOutTree(nd->Right, dash+"-");
}
