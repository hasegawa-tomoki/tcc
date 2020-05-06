#include "tcc.h"

Token *token;

char *user_input;
VarList *locals;
VarList *globals;
VarList *var_scope;
TagScope *tag_scope;

char *keywords[] = {
  "return", "if", "else", "while", "for", "sizeof", "struct", 
};

Type *consume_pointer(Type *type){
  while (consume("*")){
    type = pointer_to(type);
  }
  return type;
}

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
    error_at(token->str, "expected a number (ERR1: expect_number())");
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
  for (int i = 0; i < sizeof(keywords) / sizeof(*keywords); i++){
    int len = strlen(keywords[i]);
    if (startswith(p, keywords[i]) && !is_alnum(p[len])){
      return keywords[i];
    }
  }
  
  for (int i = 0; i < sizeof(typenames) / sizeof(*typenames); i++){
    int len = strlen(typenames[i]);
    if (startswith(p, typenames[i]) && !is_alnum(p[len])){
      return typenames[i];
    }
  }
  
  static char *ops[] = {
    "==", "!=", "<=", ">=", "->", 
  };
  for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++){
    if (startswith(p, ops[i])){
      return ops[i];
    }
  }
  return NULL;
}

char get_escape_char(char c){
  switch (c){
      case 'a':
        return '\a';
      case 'b':
        return '\b';
      case 't':
        return '\t';
      case 'n':
        return '\n';
      case 'v':
        return '\v';
      case 'f':
        return '\f';
      case 'r':
        return '\r';
      case '0':
        return '\0';
      default:
        return c;
  }
}

Token *read_string_literal(Token *cur, char *start){
  char *p = start + 1;
  char buf[1024];
  int len = 0;

  for (;;) {
    if (len == sizeof(buf))
      error_at(start, "string literal too large");
    if (*p == '\0')
      error_at(start, "unclosed string literal");
    if (*p == '"')
      break;

    if (*p == '\\') {
      p++;
      buf[len++] = get_escape_char(*p++);
    } else {
      buf[len++] = *p++;
    }
  }

  Token *tok = new_token(TK_STR, cur, start, p - start + 1);
  tok->str = malloc(len + 1);
  memcpy(tok->str, buf, len);
  tok->str[len] = '\0';
  tok->len = len + 1;
  return tok;
}

Token *tokenize(){
  char *p = user_input;

  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p){
    // Skip whie space
    if (isspace(*p)){
      p++;
      continue;
    }

    // Skip line comment
    if (strncmp(p, "//", 2) == 0){
      p += 2;
      while (*p != '\n'){
        p++;
      }
      continue;
    }

    // Skip bock comment
    if (strncmp(p, "/*", 2) == 0){
      char *q = strstr(p + 2, "*/");
      if (! q){
        error_at(p, "Unclosed block comment");
      }
      p = q + 2;
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

    // Character literal
    if (*p == '\''){
      cur = new_token(TK_NUM, cur, p, 0);
      p++;
      char c;
      c = *p;
      p++;
      if (*p != '\''){
        error_at(p, "char literal too long");
      }
      cur->val = c;
      cur->len = 3;
      p++;
      continue;
    }

    // String literal
    if (*p == '"'){
      cur = read_string_literal(cur, p);
      p += cur->len + 1;
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
