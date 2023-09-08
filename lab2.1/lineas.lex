%{
        #include <stdio.h>
        #include <stdlib.h>
        #include <iostream>
        #include <fstream>

        using namespace std;

        int a = 0;
%}

%option c++
%option outfile="Scanner.cpp"
%option case-insensitive
%option noyywrap

DIGITO  [0-9]
LETRA   [a-zA-Z]

ENTERO  {DIGITO}+
REAL    {DIGITO}+"."{DIGITO}+
IDENTIFICADOR {LETRA}({LETRA}|{DIGITO}|"_")*

DELIMITADOR [\t\r\f" "]

%%
{ENTERO}        {printf("Numero Entero %s\n",yytext); a += atoi(yytext);}
{REAL}          {printf("Numero Real %s\n",yytext);}
{IDENTIFICADOR} {printf("Indentificador %s\n",yytext);}

{DELIMITADOR} {}
.             {printf("ERROR LEXICO");}
%%

int main(){
        ifstream in("entrada.txt");
        FlexLexer* lexer = new yyFlexLexer(&in);
        while (lexer->yylex() != 0);

        cout << a;

        return 0;
        
}
