#include "tcc.h"

Node *code[100];

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
// stmt = expr ";" | "return" expr ";"
Node *stmt(){
  Node *node;

  if (consume("return")){
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
  } else {
    node = expr();
    consume(";");
  }
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
