#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>

#include <unordered_map>

using namespace std;

class Token
{
public:
  enum Type
  {
    LPAREN = 0,
    RPAREN,
    PLUS,
    MINUS,
    MULT,
    DIV,
    EXP,
    LESS,
    ELESS,
    MORE,
    EMORE,
    AND,
    OR,
    EQUAL,
    NUM,
    ID,
    PRINT,
    IF,
    SEMICOLON,
    ASSIGN,
    COMA,
    ERR,
    END
  };

  static const char *token_names[23];

  Type type;
  string lexema;

  explicit Token(Type);
  Token(Type, const string &source);
};

const char *Token::token_names[23] = {"LPAREN", "RPAREN", "PLUS", "MINUS", "MULT", "DIV", "EXP", "LESS","ELESS", "MORE", "EMORE", "AND", "OR", "EQUAL"
                                      "NUM", "ID", "PRINT", "IF", "SEMICOLON", "ASSIGN","COMA","ERR", "END"};

Token::Token(Type type) : type(type) { lexema = ""; }

Token::Token(Type type, const string &source) : type(type)
{
  lexema = source;
}

std::ostream &operator<<(std::ostream &outs, const Token &tok)
{
  if (tok.lexema.empty())
    return outs << Token::token_names[tok.type];
  else
    return outs << Token::token_names[tok.type] << "(" << tok.lexema << ")";
}

std::ostream &operator<<(std::ostream &outs, const Token *tok)
{
  return outs << *tok;
}

class Scanner
{
public:
  Scanner(const char *in_s);
  Token *nextToken();
  ~Scanner();

private:
  string input;
  int first, current;
  unordered_map<string, Token::Type> reserved;
  char nextChar();
  void rollBack();
  void startLexema();
  string getLexema();
  Token::Type checkReserved(const string &);
};

Scanner::Scanner(const char *s) : input(s), first(0), current(0)
{
  reserved["print"] = Token::PRINT;
  reserved["if"] = Token::IF;
  reserved["or"] = Token::OR;
  reserved["and"] = Token::AND;
}

Token *Scanner::nextToken()
{
  Token *token;
  char c;
  // consume whitespaces
  c = nextChar();
  while (c == ' ')
    c = nextChar();
  if (c == '\0')
    return new Token(Token::END);
  startLexema();
  if (isdigit(c))
  {
    c = nextChar();
    while (isdigit(c))
      c = nextChar();
    rollBack();
    token = new Token(Token::NUM, getLexema());
  }
  else if (isalpha(c))
  {
    c = nextChar();
    while (isalpha(c) || isdigit(c) || c == '_')
      c = nextChar();
    rollBack();
    string lex = getLexema();
    Token::Type ttype = checkReserved(lex);
    if (ttype != Token::ERR)
      token = new Token(ttype);
    else
      token = new Token(Token::ID, getLexema());
  }
  else if (strchr("()+-*/;=,><", c))
  {
    switch (c)
    {
    case '(':
      token = new Token(Token::LPAREN);
      break;
    case ')':
      token = new Token(Token::RPAREN);
      break;
    case '+':
      token = new Token(Token::PLUS);
      break;
    case '-':
      token = new Token(Token::MINUS);
      break;
    case '*':
      c = nextChar();
      if (c == '*')
        token = new Token(Token::EXP);
      else{
        rollBack();
        token = new Token(Token::MULT);
      }
      break;
    case '/':
      token = new Token(Token::DIV);
      break;
    case ';':
      token = new Token(Token::SEMICOLON);
      break;
    case '=':
      c = nextChar();
      if (c == '=')
        token = new Token(Token::EQUAL);
      else{
        rollBack();
        token = new Token(Token::ASSIGN);
      }
      break;
    case ',':
      token = new Token(Token::COMA);
      break;
    case '<':
      c = nextChar();
      if (c == '=')
        token = new Token(Token::ELESS);
      else {
        rollBack();
        token = new Token(Token::LESS);
      }
      break;
    case '>':
      c = nextChar();
      if (c == '=')
        token = new Token(Token::EMORE);
      else{
        rollBack();
        token = new Token(Token::MORE);
      }
      break;
    default:
      cout << "No deberia llegar aca" << endl;
    }
  }
  else
  {
    token = new Token(Token::ERR, getLexema());
  }
  return token;
}

Scanner::~Scanner() {}

char Scanner::nextChar()
{
  int c = input[current];
  current++;
  return c;
}

void Scanner::rollBack()
{ // retract
  current--;
}

void Scanner::startLexema()
{
  first = current - 1;
}

string Scanner::getLexema()
{
  return input.substr(first, current - first);
}

Token::Type Scanner::checkReserved(const string &lexema)
{
  std::unordered_map<std::string, Token::Type>::const_iterator it = reserved.find(lexema);
  if (it == reserved.end())
    return Token::ERR;
  else
    return it->second;
}

/* ***************** Parser ********************* */

class Parser
{
private:
  Scanner *scanner;
  Token *current, *previous;
  bool match(Token::Type ttype);
  bool check(Token::Type ttype);
  bool advance();
  bool isAtEnd();
  void parseStatementList();
  void parseStatement();
  int parseExpression();
  int parseTerm();
  int parseFactor();
  int parseFExp();
  int parseCexp();
  unordered_map<string, int> memoria;

public:
  Parser(Scanner *scanner);
  void parse();
  void memoria_update(string x, int v);
  bool memoria_check(string x);
  int memoria_lookup(string x);
};

void Parser::memoria_update(string x, int v)
{
  memoria[x] = v;
}

bool Parser::memoria_check(string x)
{
  std::unordered_map<std::string, int>::const_iterator it = memoria.find(x);
  if (it == memoria.end())
    return false;
  else
    return true;
}

int Parser::memoria_lookup(string x)
{
  std::unordered_map<std::string, int>::const_iterator it = memoria.find(x);
  if (it == memoria.end())
    return 0;
  else
    return it->second;
}

// match and consume next token
bool Parser::match(Token::Type ttype)
{
  if (check(ttype))
  {
    advance();
    return true;
  }
  return false;
}

bool Parser::check(Token::Type ttype)
{
  if (isAtEnd())
    return false;
  return current->type == ttype;
}

bool Parser::advance()
{
  if (!isAtEnd())
  {
    Token *temp = current;
    if (previous)
      delete previous;
    current = scanner->nextToken();
    previous = temp;
    if (check(Token::ERR))
    {
      cout << "Parse error, unrecognised character: " << current->lexema << endl;
      exit(0);
    }
    return true;
  }
  return false;
}

bool Parser::isAtEnd()
{
  return (current->type == Token::END);
}

Parser::Parser(Scanner *sc) : scanner(sc)
{
  previous = current = nullptr;
};

void Parser::parse()
{
  current = scanner->nextToken();
  if (check(Token::ERR))
  {
    cout << "Error en scanner - caracter invalido" << endl;
    exit(0);
  }
  parseStatementList();
  if (current->type != Token::END)
    cout << "Esperaba fin-de-input, se encontro " << current << endl;
  else
    cout << "Fin de ejecucion" << endl;
  if (current)
    delete current;
  return;
}

void Parser::parseStatementList()
{
  parseStatement();

  while (match(Token::SEMICOLON))
  {
    parseStatement();
  }
}

/*
  id = exp
  print(x)
 */
void Parser::parseStatement()
{
  if (match(Token::ID))
  {
    string id = previous->lexema;

    if (match(Token::ASSIGN))
    {
      int v = parseExpression();
      memoria_update(id, v);
    }
    else
    {
      exit(0);
    }
  }
  else if (match(Token::PRINT))
  {
    if (match(Token::LPAREN))
    {
      int v = parseExpression();

      if (match(Token::RPAREN))
      {
        cout << v << endl;
      }
      else
      {
        exit(0);
      }
    }
  }
  else
  {
    cout << "No se encontro Statement" << endl;
    exit(0);
  }
}

int Parser::parseExpression()
{
  int accum, v;
  accum = parseTerm();

  while (match(Token::MINUS) || match(Token::PLUS) || match(Token::MORE) || match(Token::EMORE) || match(Token::LESS) || match(Token::ELESS) || match(Token::AND) || match(Token::OR) || match(Token::EQUAL))
  {
    Token::Type op = previous->type;
    v = parseTerm();
    if (op == Token::MINUS)
      accum = accum - v;
    else if (op == Token::PLUS)
      accum = accum + v;
    else if (op == Token::MORE)
      accum = accum > v;
    else if (op == Token::EMORE)
      accum = accum >= v;
    else if (op == Token::LESS)
      accum = accum < v;
    else if (op == Token::ELESS)
      accum = accum <= v;
    else if (op == Token::AND)
      accum = accum && v;
    else if (op == Token::OR)
      accum = accum || v;
    else
      accum = accum == v;
  }
  return accum;
}

int Parser::parseCexp()
{
  int accum, v;
  accum = parseTerm();

  if (match(Token::MORE) || match(Token::EMORE) || match(Token::LESS) || match(Token::ELESS) || match(Token::AND) || match(Token::OR) || match(Token::EQUAL)){
    Token::Type op = previous->type;
    v = parseTerm();

      if (op == Token::MORE)
        accum = accum > v;
      if (op == Token::EMORE)
        accum = accum >= v;
      if (op == Token::LESS)
        accum = accum < v;
      if (op == Token::ELESS)
        accum = accum <= v;
      if (op == Token::AND)
        accum = accum && v;
      if (op == Token::OR)
        accum = accum || v;
      else
        accum = accum == v;
  }
  else{
      cout << "Expecting comparator operator" << endl;
      exit(0);
  }

  if (match(Token::MORE) || match(Token::EMORE) || match(Token::LESS) || match(Token::ELESS) || match(Token::AND) || match(Token::OR) || match(Token::EQUAL))
  {
      Token::Type op = previous->type;
      v = parseTerm();

      if (op == Token::MORE)
        accum = accum > v;
      if (op == Token::EMORE)
        accum = accum >= v;
      if (op == Token::LESS)
        accum = accum < v;
      if (op == Token::ELESS)
        accum = accum <= v;
      if (op == Token::AND)
        accum = accum && v;
      if (op == Token::OR)
        accum = accum || v;
      else
        accum = accum == v;
  }

  return accum;
}

int Parser::parseTerm()
{
  int accum, v;
  accum = parseFExp();
  while (match(Token::MULT) || match(Token::DIV))
  {
    Token::Type op = previous->type;
    v = parseFExp();
    if (op == Token::MULT)
      accum *= v;
    else
      accum /= v;
  }
  return accum;
}

int Parser::parseFExp(){
  int v, rhs, res;
  v = parseFactor();
  while (match(Token::EXP)){
    rhs = parseFExp();
    res = 1;
    while (rhs > 0){
      res *= v;
      rhs --;
    }
    return res;
  }
  return v;
}

int Parser::parseFactor()
{
  if (match(Token::NUM))
  {
    // el token NUM, luego del match, esta la variable previous
    return stoi(previous->lexema);
  }
  if (match(Token::ID))
  {
    // previous->lexema;
    // inspeccionar memoria - error si no esta!!
    if (memoria_check(previous->lexema))
    {
      return memoria_lookup(previous->lexema);
    }
    else
    {
      cout << "Variable no inicializada" << previous->lexema << endl;
    }
    return 0;
  }
  if (match(Token::LPAREN))
  {
    int v = parseExpression();
    if (!match(Token::RPAREN))
    {
      cout << "Expecting right parenthesis" << endl;
      exit(0);
    }
    return v;
  }

  if (match(Token::IF)){

    if (match(Token::LPAREN)){
      int a = parseCexp();

      if (match(Token::COMA)){
        int b = parseExpression();

        if (match(Token::COMA)){

          int c = parseExpression();

          if (match(Token::RPAREN)){
            if (a)
              return b;
            return c;
          }
          cout << "Expected right parentesis" << endl;
          exit(0);
        }
        cout << "Expected coma" << endl;
        exit(0);
      }
      cout << "Expected coma1" << endl;
      exit(0);
    }
    cout << "Expected left parentesis" << endl;
    exit(0);
  }
  cout << "Couldn't find match for token: " << current << endl;
  exit(0);
}

// ---------------------------------------------------

int main(int argc, const char *argv[])
{

  if (argc != 2)
  {
    cout << "Incorrect number of arguments" << endl;
    exit(1);
  }

  Scanner scanner(argv[1]);

  Parser parser(&scanner);
  parser.parse();

  /*
  Token* tk = scanner.nextToken();
  while (tk->type != Token::END) {
    cout << "next token " << tk << endl;
    delete tk;
    tk =  scanner.nextToken();
  }
  cout << "last token " << tk << endl;
  delete tk;
  */
}