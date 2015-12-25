#include "stdinc.h"
#include "lex.h"

// 'Display error' functions
void Error(string err)
{
    cout << "Error:\n\t";
    cout << err << "\n\t\t";
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
    getchar();
    exit(-1);

}

// 'Str<-->Num' functions
string ToStr(long unsigned Num)
{
    char buffer[20];
    sprintf(buffer, "%lu",Num);
    return buffer;
}
int ToInt(string str)
{
    return atoi(str.c_str());
}

// The MAIN function
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
    PrintOutTokens();

    cout << "\nDone!!!";
    getchar();
    return 0;
}
