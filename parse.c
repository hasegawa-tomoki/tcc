#include "tcc.h"

Node *code[100];
Token *token;
char *user_input;
LVar *locals;

void error(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
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
    error_at(token->str, "expected a number (ERR1)");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(){
  return token->kind == TK_EOF;
}

LVar *find_lvar(Token *tok){
  for (LVar *var = locals; var->next != NULL; var = var->next){
    if (var->len == tok->len && (! memcmp(tok->str, var->name, var->len))){
      return var;
    }
  }
  return NULL;
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
    
    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")){
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    } 

    if ('a' <= *p && *p <= 'z'){
      char *q = p;
      do {
        q++;
      } while ('a' <= *q && *q <= 'z');
      cur = new_token(TK_IDENT, cur, p, q - p);
      p = q;
      continue;
    }

    if (strchr("+-*/()<>;=", *p)){
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)){
      char *q = p;
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "expected a number (ERR2)");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;

  return node;
}

Node *new_node_num(int val){
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;

  return node;
}

// program    = stmt*
void program(){
  int i = 0;

  LVar head;
  head.next = NULL;
  head.offset = -8;
  locals = &head;

  while (! at_eof()){
    code[i++] = stmt();
  }
  code[i] = NULL;
}

// stmt       = expr ";"
Node *stmt(){
  Node *node = expr();
  consume(";");
  return node;
}

// expr       = assign
Node *expr(){
  return assign();
}

// assign     = equality ("=" assign)?
Node *assign(){
  Node *node = equality();
  if (consume("=")){
    node = new_node(ND_ASSIGN, node, assign());
  }
  return node;
}

// equality   = relational ("==" relational | "!=" relational)*
Node *equality (){
  Node *node = relational();

  for (;;){
    if (consume("==")){
      node = new_node(ND_EQ, node, relational());
    } else if (consume("!=")){
      node = new_node(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational (){
  Node *node = add();

  for (;;){
    if (consume("<=")){
      node = new_node(ND_LE, node, add());
    } else if (consume("<")){
      node = new_node(ND_LT, node, add());
    } else if (consume(">=")){
      node = new_node(ND_LE, add(), node);
    } else if (consume(">")){
      node = new_node(ND_LT, add(), node);
    } else {
      return node;
    }
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add(){
  Node *node = mul();

  for (;;){
    if (consume("+")){
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")){
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(){
  Node *node = unary();
  
  for (;;){
    if (consume("*")){
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/")){
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

// unary = ("+" | "-")* primary
Node *unary(){
  if (consume("+")){
    return unary();
  }
  if (consume("-")){
    return new_node(ND_SUB, new_node_num(0), unary());
  }
  return primary();
}

// primary    = num | ident | "(" expr ")"
Node *primary(){
  if (consume("(")){
    Node *node = expr();
    expect(")");

    return node;
  }

  Token *tok = consume_ident();
  if (tok){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar){
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals->offset + 8;
      node->offset = lvar->offset;
      locals = lvar;

      // for debug
      // char t[64];
      // strncpy(t, lvar->name, lvar->len);
      // t[lvar->len] = '\0';
      // fprintf(stderr, "  Local variable %s added.\n", t);
    }

    return node;
  }

  return new_node_num(expect_number());
}
