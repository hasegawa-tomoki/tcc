#include "tcc.h"

Node *code[100];

Node *new_node(NodeKind kind){
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;

  return node;
}

Node *new_node_with_lrs(NodeKind kind, Node *lhs, Node *rhs){
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;

  return node;
}

Node *new_node_num(int val){
  Node *node = new_node(ND_NUM);
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

// stmt = expr ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt

Node *stmt(){
  Node *node;

  if (consume("return")){
    node = new_node(ND_RETURN);
    node->lhs = expr();
    expect(";");
    return node;
  }

  if (consume("if")){
    expect("(");
    node = new_node(ND_IF);
    node->cond = expr();
    expect(")");
    node->then = stmt();

    if (consume("else")){
      node->els = stmt();
    }
    return node;
  }

  if (consume("while")){
    expect("(");
    node = new_node(ND_WHILE);
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  }

  if (consume("for")){
    expect("(");
    node = new_node(ND_FOR);
    node->init = expr();
    expect(";");
    node->cond = expr();
    expect(";");
    node->iterate = expr();
    expect(")");
    node->then = stmt();
    return node;
  }

  node = expr();
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
    node = new_node_with_lrs(ND_ASSIGN, node, assign());
  }
  return node;
}

// equality   = relational ("==" relational | "!=" relational)*
Node *equality (){
  Node *node = relational();

  for (;;){
    if (consume("==")){
      node = new_node_with_lrs(ND_EQ, node, relational());
    } else if (consume("!=")){
      node = new_node_with_lrs(ND_NE, node, relational());
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
      node = new_node_with_lrs(ND_LE, node, add());
    } else if (consume("<")){
      node = new_node_with_lrs(ND_LT, node, add());
    } else if (consume(">=")){
      node = new_node_with_lrs(ND_LE, add(), node);
    } else if (consume(">")){
      node = new_node_with_lrs(ND_LT, add(), node);
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
      node = new_node_with_lrs(ND_ADD, node, mul());
    } else if (consume("-")){
      node = new_node_with_lrs(ND_SUB, node, mul());
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
      node = new_node_with_lrs(ND_MUL, node, unary());
    } else if (consume("/")){
      node = new_node_with_lrs(ND_DIV, node, unary());
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
    return new_node_with_lrs(ND_SUB, new_node_num(0), unary());
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

char *node_name(int kind){
    static char *node_kinds[] = {
      "ND_ADD", 
      "ND_SUB", 
      "ND_MUL", 
      "ND_DIV", 
      "ND_EQ", 
      "ND_NE", 
      "ND_LT", 
      "ND_LE", 
      "ND_ASSIGN", 
      "ND_LVAR", 
      "ND_NUM", 
      "ND_RETURN", 
      "ND_IF", 
      "ND_WHILE", 
      "ND_FOR", 
    };
    return node_kinds[kind];
}

void show_node(Node *node, char *name, int indent){
    for (int i = 0; i < (indent + 1) * 2; i++){
      fprintf(stderr, " ");
    }

    fprintf(stderr, "-- %s  kind: %-20s  ", name, node_name(node->kind));
    if (node->kind == ND_LVAR){
      fprintf(stderr, "  offset: %d", node->offset);
    }
    if (node->kind == ND_NUM){
      fprintf(stderr, "  val: %d", node->val);
    }
    fprintf(stderr, "\n");
    if (node->lhs){
      show_node(node->lhs, "lhs", indent + 1);
    }
    if (node->rhs){
      show_node(node->rhs, "rhs", indent + 1);
    }
    if (node->cond){
      show_node(node->cond, "cond", indent + 1);
    }
    if (node->then){
      show_node(node->then, "then", indent + 1);
    }
    if (node->els){
      show_node(node->els, "els", indent + 1);
    }
    if (node->init){
      show_node(node->init, "init", indent + 1);
    }
    if (node->iterate){
      show_node(node->iterate, "iterate", indent + 1);
    }
}

void show_nodes(Node *code[]){
  for (int i = 0; code[i]; i++){
      show_node(code[i], "node", 0);
  }
}