#ifndef STAGEII_H_INCLUDED
#define STAGEII_H_INCLUDED

int ParseExpression(Node* &nd, int PrecL);
void ParseStatement(Node * &nd);
void ParseStatements(Node * &nd);

void ParseFuncsDefn(Node * &nd);

#endif // STAGEI_H_INCLUDED
