#include "tcc.h"

Token *token;
char *user_input;
VarList *locals;
VarList *globals;

bool consume(char *op){
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
    return false;
  }
  token = token->next;
  return true;
}

Token *consume_ident(){
  if (token->kind != TK_IDENT){
    return NULL;
  }
  Token *t = token;
  token = token->next;
  return t;
}

bool expect(char *op){
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
    error_at(token->str, "expected '%s'", op);
  }
  token = token->next;
}

int expect_number(){
  if (token->kind != TK_NUM){
    error_at(token->str, "expected a number (ERR1)");
  }
  int val = token->val;
  token = token->next;
  return val;
}

char *expect_ident(){
  if (token->kind != TK_IDENT){
    error_at(token->str, "expected an identifier");
  }
  char *name = substr(token->str, token->len);
  token = token->next;
  return name;
}

bool peek(char *op){
  char *name = substr(op, token->len);
  if (token->kind != TK_RESERVED || strlen(name) != token->len || strncmp(token->str, name, token->len)){
    return false;
  }
  return true;
}

bool at_eof(){
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q){
  return memcmp(p, q, strlen(q)) == 0;
}

bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool is_alnum(char c) {
  return is_alpha(c) || ('0' <= c && c <= '9');
}

char *starts_with_reserved(char *p){
  static char *kw[] = {
    "return", "if", "else", "while", "for", "int", "sizeof", 
  };
  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++){
    int len = strlen(kw[i]);
    if (startswith(p, kw[i]) && !is_alnum(p[len])){
      return kw[i];
    }
  }

  static char *ops[] = {
    "==", "!=", "<=", ">=", 
  };
  for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++){
    if (startswith(p, ops[i])){
      return ops[i];
    }
  }
  return NULL;
}

Token *tokenize(){
  char *p = user_input;

  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p){
    if (isspace(*p)){
      p++;
      continue;
    }
    
    // Keywords or multi-letter punctuators
    char *kw = starts_with_reserved(p);
    if (kw){
      int len = strlen(kw);
      cur = new_token(TK_RESERVED, cur, p, len);
      p += len;
      continue;
    }

    // Identifier
    if (is_alpha(*p)){
      char *q = p++;
      while (is_alnum(*p)) {
        p++;
      }
      cur = new_token(TK_IDENT, cur, q, p - q);
      continue;
    }

    // Single-letter punctuators
    if (ispunct(*p)){
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // Integer literal
    if (isdigit(*p)){
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "expected a number (ERR2)");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
