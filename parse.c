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

Node *new_num_node(int val){
  Node *node = new_node(ND_NUM);
  node->val = val;

  return node;
}

Node *new_var_node(Var *var){
  Node *node = new_node(ND_VAR);
  node->var = var;
  return node;
}

Var *new_lvar(char *name){
  Var *var = calloc(1, sizeof(Var));
  var->next = locals;
  var->name = name;
  locals = var;
  return var;
}

Type *new_type(TypeKind kind){
  Type *type = calloc(1, sizeof(Type));
  type->kind = kind;
  return type;
}

// program    = function*
Function *program(){
  Function head = {};
  Function *cur = &head;

  while (! at_eof()){
    cur->next = function();
    cur = cur->next;
  }
  return head.next;
}

// function = ident "(" ")" "{" stmt* "}"
Function *function(){
  locals = NULL;

  char *name = expect_ident();
  expect("(");
  expect(")");
  expect("{");

  Node head = {};
  Node *cur = &head;

  while (! consume("}")){
    cur->next = stmt();
    cur = cur->next;
  }

  Function *fn = calloc(1, sizeof(Function));
  fn->name = name;
  fn->node = head.next;
  fn->locals = locals;
  return fn;
}

// stmt = expr ";"
//      | "{" stmt* "}"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | declaration
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

  if (consume("{")){
    Node head = {};
    Node *cur = &head;
    while (!consume("}")){
      cur->next = stmt();
      cur = cur->next;
    }
    node = new_node(ND_BLOCK);
    node->body = head.next;
    return node;
  }

  if (peek("int")){
    consume("int");
    
    Var *lvar = new_lvar(substr(token->str, token->len));
    lvar->type = new_type(TY_INT);
    return new_var_node(lvar);
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
//      | "*" unary (= *hoge)
//      | "&" unary (= &hoge)
Node *unary(){
  if (consume("+")){
    return unary();
  }
  if (consume("-")){
    return new_node_with_lrs(ND_SUB, new_num_node(0), unary());
  }
  if (consume("*")){
    Node *node = new_node(ND_DEREF);
    node->lhs = unary();
    return node;
  }
  if (consume("&")){
    Node *node = new_node(ND_ADDR);
    node->lhs = unary();
    return node;
  }
  return primary();
}

// primary    = num
//          | ident ("(" ")")?
//          | "(" expr ")"
Node *primary(){
  if (consume("(")){
    Node *node = expr();
    expect(")");

    return node;
  }

  Token *tok = consume_ident();
  if (tok){
    // Function call
    Node *head = NULL;
    if (consume("(")){
      while (! consume(")")){
        head = expr();
        Node *cur = head;
        while (consume(",")){
          cur->next = expr();
          cur = cur->next;
        }
      }
      Node *node = new_node(ND_FUNCCALL);
      node->funcname = substr(tok->str, tok->len);
      node->args = head;
      return node;
    }

    // Variable
    Var *lvar = find_lvar(tok);
    if (! lvar){
      error_at(token->str, "Undefined varible.");
    }
    return new_var_node(lvar);
  }

  return new_num_node(expect_number());
}
