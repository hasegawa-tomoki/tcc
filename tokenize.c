#include "tcc.h"

Token *token;
char *user_input;
LVar *locals;

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

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
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
    
    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])){
      //cur = new_token(TK_RETURN, cur, p, 6);
      cur = new_token(TK_RESERVED, cur, p, 6);
      p += 6;
      continue;
    }

    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")){
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    } 

    if ('a' <= *p && *p <= 'z'){
      char *q = p;
      do {
        q++;
      } while (is_alnum(*q));
      cur = new_token(TK_IDENT, cur, p, q - p);
      p = q;
      continue;
    }

    if (strchr("+-*/()<>;=", *p)){
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

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
