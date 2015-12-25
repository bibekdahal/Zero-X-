#include "stdinc.h"
#include "lex.h"
#include "parser.h"

void Error(string err)
{
    cout << "Error:\n\t";
    cout << err << "\n\t\t";
    Deallocate();
    getchar();
    exit(-1);
}
void Error(string err, long unsigned LineStart, long Pos)
{
    cout << "Error:\n\t";
    cout << err << "\n\t\t";
    cout << GetLine(LineStart);
    if (Pos!=-1)
    {
        cout<<"\n\t\t";
        for (int i=0;i<Pos-1;i++)
            cout << " ";
        cout<<"^";
    }
    cout << "\nLine Number:" << ln+1;
    Deallocate();
    getchar();
    exit(-1);

}
string ToStr(double Num)
{
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%f",Num);
    return buffer;
}
string ToStr(int Num)
{
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d",Num);
    return buffer;
}
string ToStr(long Num)
{
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%ld",Num);
    return buffer;
}
double ToNum(string str)
{
    return atof(str.c_str());
}
bool IsStrNum(string str)
{
    std::string::const_iterator it = str.begin();
    while (it != str.end() && std::isdigit(*it)) ++it;
    return !str.empty() && it == str.end();
}
int main(int argc, char **argv)
{

    // Ask for filename if not supplied as argument
    string FileName;
    if (argc != 2){
        cout << "Usage: ZXP <filename>\nEnter Filename: ";
        getline(cin,FileName);
        cout << "\r\n";
    }
    else
        FileName = argv[1];
    // Display message that compilation is going on and open the file
    cout << "Compiling file: " << FileName << "\n";
    OpenProgram(FileName);
                                                                        cout << ".";
    //Prepare the list of tokens and display them
    PrepareTokensList();                                                cout << ".";
    //Initialize the database
    InitData();
    //Parse the series of tokens
    Parse();

    //Print out every thing we got by parsing
    PrintOutTypes();    cout<<"\n";
    PrintOutGlobals();  cout<<"\n";
    PrintOutFuncs();    cout<<"\n";
    PrintOutOprs();     cout<<"\n";
    cout<<"\n";
    PrintOutTree(ParseTree);

    //Deallocate the tree
    Deallocate();
    cout << "\nDone!!!";

    getchar();
    return 0;
}
