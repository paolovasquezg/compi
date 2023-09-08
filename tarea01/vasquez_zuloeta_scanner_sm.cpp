#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <memory>

#include <list>
#include <stack>

using namespace std;

/*
  Autores:
  Paolo Vásquez
  Marcelo Zuloeta
*/

// Estructura de datos utilizada: Unordered map --> <string, Token>
// Tambien implementado una función hash para este map customizado

// Cambios:
// Clase Token --> Enum type
// Clase Scanner --> Dos metodos adicionales (setState y getState)
// Token* Scanner::nextToken --> Funciona ahora como un autómata, y se emplea la estructura para la busqueda de palabras reservadas

class Token {
public:
  enum Type {NUM, ID, LABEL, EOL, PUSH, JMPEQ, JMPGT, JMPGE, JMPLT, JMPLE, GOTO, SKIP, POP, DUP, SWAP, ADD, SUB, MUL, DIV, STORE, LOAD, ERR, END };
  static const char* token_names[25];
  Type type;
  string lexema;
  Token(Type);
  Token(Type, char c);
  Token(Type, const string source);
};

template <>
struct hash<Token>
{
  size_t operator()(const Token &token) const
  {
    return static_cast<size_t>(token.type) ^ std::hash<std::string>()(token.lexema);
  }
};

std::unordered_map<string, Token> keywords = {
    {"push", Token(Token::PUSH)},
    {"jmpeq", Token(Token::JMPEQ)},
    {"jmpgt", Token(Token::JMPGT)},
    {"jmpge", Token(Token::JMPGE)},
    {"jmplt", Token(Token::JMPLT)},
    {"jmple", Token(Token::JMPLE)},
    {"goto", Token(Token::GOTO)},
    {"skip", Token(Token::SKIP)},
    {"pop", Token(Token::POP)},
    {"dup", Token(Token::DUP)},
    {"swap", Token(Token::SWAP)},
    {"add", Token(Token::ADD)},
    {"sub", Token(Token::SUB)},
    {"mul", Token(Token::MUL)},
    {"div", Token(Token::DIV)},
    {"store", Token(Token::STORE)},
    {"load", Token(Token::LOAD)}};

const char *Token::token_names[25] = {"NUM", "ID", "LABEL", "EOL", "PUSH", "JMPEQ", "JMPGT", "JMPGE", "JMPLT", "JMPLE", "GOTO", "SKIP", "POP", "DUP", "SWAP", "ADD", "SUB", "MUL", "DIV", "STORE", "LOAD", "ERR", "END"};

Token::Token(Type type):type(type){ lexema = ""; }

Token::Token(Type type, char c): type(type) { lexema = c; }

Token::Token(Type type, const string source) : type(type)
{
  lexema = source;
}

std::ostream& operator << ( std::ostream& outs, const Token & tok )
{
  
  if (tok.lexema.empty())
      return outs << Token::token_names[tok.type];
  else
    return outs << Token::token_names[tok.type] << "(" << tok.lexema << ")";
}

std::ostream& operator << ( std::ostream& outs, const Token* tok ) {
  return outs << *tok;
}

class Scanner {
public:
  Scanner(const char *in_s);
  Scanner(const string s);
  Token* nextToken();
  ~Scanner();  
private:
  string input;
  int first, current;
  int state;
  char nextChar();
  void rollBack();
  void startLexema();
  void incrStartLexema();
  string getLexema();
  int getState();
  void setState(int st);

};

Scanner::Scanner(const char *s) : input(s), first(0), current(0) {}
Scanner::Scanner(const string s) : input(s), first(0), current(0) {}

Token* Scanner::nextToken() {
  Token* token;
  char c = nextChar();
  while (c==' ') c = nextChar();
  if (c == '\0') return new Token(Token::END);
  setState(0);
  startLexema();

  while (1){
    switch(getState()){
      case 0:
        if (isalpha(c)) setState(1);
        else if (isdigit(c)) setState(4);
        else if (c == '\n') setState(6);
        else return new Token(Token::ERR);
        break;
      case 1:
        c = nextChar();
        if (isalpha(c) || isdigit(c) || c == '_') setState(1);
        else if (c == ':') setState(3);
        else setState(2);
        break;
      case 2:
        if (keywords.find(getLexema().substr(0,getLexema().size()-1)) != keywords.end())
          token = &(keywords.find(getLexema().substr(0, getLexema().size() - 1))->second);
        else
          token =  new Token(Token::ID, getLexema());
        rollBack();
        return token;
        break;
      case 3:
        return new Token(Token::LABEL, getLexema().substr(0, getLexema().size() - 1));
        break;
      case 4:
        c = nextChar();
        if (isdigit(c)) setState(4);
        else setState(5);
        break;
      case 5:
        rollBack();
        return new Token(Token::NUM, getLexema());
        break;
      case 6:
        c = nextChar();
        if (c == '\n') setState(6);
        else setState(7);
        break;
      case 7:
        rollBack();
        return new Token(Token::EOL);
        break;
    } 
  }
}

Scanner::~Scanner() { }

char Scanner::nextChar() {
  int c = input[current];
  if (c != '\0') current++;
  return c;
}

void Scanner::rollBack() {
  if (input[current] != '\0')
    current--;
}

void Scanner::startLexema() {
  first = current-1;  
  return;
}

void Scanner::incrStartLexema() {
  first++;
}

string Scanner::getLexema() {
  return input.substr(first,current-first);
}

int Scanner::getState(){
  return state;
}

void Scanner::setState(int st){
  state = st;
}

// ---------------------------------------------------

int main(int argc, const char* argv[]) {


  if (argc != 2) {
    cout << "File name missing" << endl;
    exit(1);
  }

  cout << "Reading program from file " << argv[1] << endl;
  std::ifstream t(argv[1]);
  std::stringstream buffer;
  buffer << t.rdbuf();

  Scanner scanner(buffer.str());

  Token* tk = scanner.nextToken();

  while (tk->type != Token::END) {
    cout << tk << endl;
    tk =  scanner.nextToken();
  }
  cout << tk << endl;
  delete tk;

  return 0;

}
