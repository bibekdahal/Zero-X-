
#include "stdinc.h"

unsigned long tk;

// Don't thing any code here needs to be explained


string GetTokenTypeName(int Type)
{
    switch(Type)
    {
        case IDENTIFIER:    return "IDENTIFIER";
        case NUMBER:        return "NUMBER";
        case STRING:        return "STRING";
        case EOL:           return "EOL";
        case CHARACTER:     return "CHARACTER";
        case SYMBOL:        return "SYMBOL";
        default:            return "UNKNOWN";
    }
}
void PrintOutTokens()
{
    tk=0;
    for (unsigned i=0;i<Tokens.size();i++)
    {
        cout << "\n\n\tFound a token";
        if (Token.Type!=EOL)    cout << ":\n\t\t" << Token.Str;
        cout << "\n\t\tType: " << GetTokenTypeName(Token.Type);

        NextToken;
    }
}
