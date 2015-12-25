#ifndef TRICODE_H_INCLUDED
#define TRICODE_H_INCLUDED
extern vector<string> tosave;
void GetAll();

extern vector<OprInfo>  Oprs;
extern vector<TypeInfo> Types;
string GetExpr(Node* nd);

void MakeTriCode(Node* nd);




#define Assign(dest, src1, opr, src2) AddTriCode("ASSIGN", opr, src1, src2, dest);
#define Copy(dest, src) AddTriCode("ASSIGN", "COPY", src, "", dest);
#define CopyIdSrc(dest, src, id)  AddTriCode("ASSIGN", "COPY_ID_SRC", src, id, dest);
#define CopyIdDest(dest, src, id)  AddTriCode("ASSIGN", "COPY_ID_DEST", src, id, dest);
#define Push(src)  AddTriCode("PUSH", "", src, "", "");
#define Pop(dest)  AddTriCode("POP", "", "", "", dest);
#define Return(a)   AddTriCode("RETURN", "", a, "", "");

#define CopyCall(dest, func, n)\
    NewBlock("NEW_BLOCK","CALL_FUNC");  AddTriCode("ASSIGN", "CALL", func, n, dest);    \
    AddTmp(dest);
#define Call(func,n)\
    NewBlock("NEW_BLOCK","CALL_FUNC");    AddTriCode("CALL", "", func, n, "");
#define Allocate(dest, size)\
    NewBlock("NEW_BLOCK","ALLOC_FUNC"); AddTriCode("ALLOCATE", "", size, "", dest);     \
    AddTmp(dest);
#define JmpIfFalse(expr, lbl)\
    AddTriCode("JMP_IF_FALSE", "", expr, "", lbl);  NewBlock("NEW_BLOCK","JMP_LBL");
#define Jmp(lbl)\
    AddTriCode("JMP", "", "", "", lbl); NewBlock("NEW_BLOCK","JMP_LBL");
#define Label(lbl)\
    AddTriCode("LABEL", "", lbl, "", "");  NewBlock("NEW_BLOCK","LBL");





#endif // TRICODE_H_INCLUDED
