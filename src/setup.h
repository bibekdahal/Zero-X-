
////// Notes on Precedence /////////////
// An operator with higher precedence is evaluated before the lowed precedent operators
// For e.g.: 2-3*4 needs 3*4 to be calculated as (2)-(3*4) i.e. * should be operated before -
// So * should have higher precedence than -
////////////////////////////////////////





// Set-Up database with default types and operators
void Setup()
{
    // Data Types
    TypeInfo tp;
    tp.Del=false;
    tp.Name="INT"; tp.Size=4; Types.push_back(tp);
    tp.Name="CHAR"; tp.Size=1; Types.push_back(tp);
    tp.Name="FLOAT"; tp.Size=4; Types.push_back(tp);
    tp.Name="BOOL"; tp.Size=4; Types.push_back(tp);
    tp.Name="DOUBLE"; tp.Size=8; Types.push_back(tp);
    tp.Name="VOID"; tp.Size=0; Types.push_back(tp);

    SetPointerType(GetType("CHAR"),1);    //string : a pointer to char

    // Operators
    OprInfo opr;
    opr.Unary=false;
    opr.Name="+";
    opr.RetType=opr.RType=opr.LType=GetType("DOUBLE");  //CheckNumber shall make sure parser works for all number types
    opr.Precedence=4;                                   // not just DOUBLE
    Oprs.push_back(opr);
    opr.Name="-";
    Oprs.push_back(opr);
    opr.Name="*";
    opr.Precedence=5;
    Oprs.push_back(opr);
    opr.Name="/";
    Oprs.push_back(opr);


    opr.Name="<";
    opr.Precedence=3;
    opr.RetType=GetType("BOOL");
    opr.LType=opr.RType=GetType("DOUBLE");
    Oprs.push_back(opr);
    opr.Name=">";
    Oprs.push_back(opr);
    opr.Name="<=";
    Oprs.push_back(opr);
    opr.Name=">=";
    Oprs.push_back(opr);
    opr.Name="!=";
    Oprs.push_back(opr);
    opr.LType=opr.RType=GetType("BOOL");
    Oprs.push_back(opr);
    opr.Name="==";
    Oprs.push_back(opr);
    opr.LType=opr.RType=GetType("DOUBLE");
    Oprs.push_back(opr);

    opr.Name="AND";
    opr.Precedence=2;
    opr.RetType=opr.LType=opr.RType=GetType("BOOL");
    Oprs.push_back(opr);
    opr.Name="OR";
    Oprs.push_back(opr);

    opr.Unary=true;
    opr.Precedence=-1;
    opr.Name="-";
    opr.LType=GetType("VOID");
    opr.RetType=opr.RType=GetType("DOUBLE");
    Oprs.push_back(opr);
    opr.Name="NOT";
    opr.RetType=opr.RType=GetType("BOOL");
    Oprs.push_back(opr);
}
