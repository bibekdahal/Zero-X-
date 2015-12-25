#include "stdinc.h"
#include "stageI.h"
#include "stageII.h"
#include "parser.h"


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
}
