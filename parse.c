#include "tcc.h"

//Node *code[100];

void set_lhs(Node *node, Node *lhs){
  node->lhs = lhs;
  
  switch (node->kind){
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_FUNCCALL:
    case ND_NUM:
      node->type = new_type(TY_INT);
      break;
    case ND_PTR_ADD:
    case ND_PTR_SUB:
    case ND_ASSIGN:
      node->type = node->lhs->type;
      break;
    case ND_VAR:
      node->type = node->var->type;
      break;
    case ND_ADDR:
      // chibicc のコミットではこうなっているけど、アドレス計算は TY_INT では？
      //node->type = pointer_to(node->lhs->type);
      node->type = new_type(TY_INT);
      break;
    case ND_DEREF:
      if (node->lhs->type->kind != TY_PTR){
        error("invalid pointer dereference.");
      }
      node->type = node->lhs->type->ptr_to;
      break;
  }
}
void set_rhs(Node *node, Node *rhs){
  node->rhs = rhs;
}

Node *new_node(NodeKind kind){
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;

  return node;
}

Node *new_lr_node(NodeKind kind, Node *lhs, Node *rhs){
  Node *node = new_node(kind);
  set_lhs(node, lhs);
  set_rhs(node, rhs);

  return node;
}

Node *new_add_node(Node *lhs, Node *rhs){
  if (! lhs->type){
    show_node(lhs, "@new_add_node lhs", 0);
    error("type not found in lhs");
  }
  if (! rhs->type){
    show_node(rhs, "@new_add_node rhs", 0);
    error("type not found in rhs");
  }
  if (lhs->type->kind == TY_INT && rhs->type->kind == TY_INT){
    return new_lr_node(ND_ADD, lhs, rhs);
  } else if (lhs->type->kind == TY_PTR && rhs->type->kind == TY_INT){
    return new_lr_node(ND_PTR_ADD, lhs, rhs);
  } else if (lhs->type->kind == TY_INT && rhs->type->kind == TY_PTR){
    return new_lr_node(ND_PTR_ADD, rhs, lhs);
  }
  error_tok(token, "invalid operands");
}

Node *new_sub_node(Node *lhs, Node *rhs){
  if (! lhs->type){
    show_node(lhs, "@new_sub_node lhs", 0);
    error("type not found in lhs");
  }
  if (! rhs->type){
    show_node(rhs, "@new_sub_node rhs", 0);
    error("type not found in rhs");
  }
  if (lhs->type->kind == TY_INT && lhs->type->kind == TY_INT){
    return new_lr_node(ND_SUB, lhs, rhs);
  } else if (lhs->type->kind == TY_PTR && lhs->type->kind == TY_INT){
    return new_lr_node(ND_PTR_SUB, lhs, rhs);
  } else if (lhs->type->kind == TY_INT && lhs->type->kind == TY_PTR){
    return new_lr_node(ND_PTR_SUB, rhs, lhs);
  }
  show_node(lhs, "lhs", 0);
  show_node(rhs, "rhs", 0);
  error_tok(token, "Invalid operands");
}

Node *new_num_node(int val){
  Node *node = new_node(ND_NUM);
  node->val = val;
  node->type = new_type(TY_INT);

  return node;
}

Node *new_var_node(Var *var){
  Node *node = new_node(ND_VAR);
  node->var = var;
  node->type = var->type;
  return node;
}

Var *new_lvar(char *name, Type *type){
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->type = type;

  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = var;
  vl->next = locals;
  locals = vl;

  return var;
}

Type *new_type(TypeKind kind){
  Type *type = calloc(1, sizeof(Type));
  type->kind = kind;

  switch (kind){
    case TY_INT:
      type->size = 4;
      break;
    case TY_PTR:
      type->size = 8;
      break;
    default:
      error("Undefined type %d in new_type", kind);
  }

  return type;
}

Type *pointer_to(Type *type){
  Type *ty = new_type(TY_PTR);
  ty->ptr_to = type;
  return ty;
}

Type *expect_type(){
  expect("int");
  Type *ty = new_type(TY_INT);
  while (consume("*")){
    ty = pointer_to(ty);
  }
  return ty;
}

VarList *read_func_param(){
    VarList *vl = calloc(1, sizeof(VarList));
    Type *type = expect_type();
    vl->var = new_lvar(expect_ident(), type);
    return vl;
}

VarList *read_func_params(){
  if (consume(")")){
    return NULL;
  }

  VarList *head = read_func_param();
  VarList *cur = head;
  while (! consume(")")){
    expect(",");
    cur->next = read_func_param();
    cur = cur->next;
  }

  return head;
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

  Type *fn_type = expect_type();
  char *name = expect_ident();
  // args
  expect("(");
  VarList *params = read_func_params();
  // body
  expect("{");

  Node head = {};
  Node *cur = &head;
  while (! consume("}")){
    cur->next = stmt();
    cur = cur->next;
  }

  Function *fn = calloc(1, sizeof(Function));
  fn->name = name;
  fn->type = fn_type;
  fn->params = params;
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
    set_lhs(node, expr());
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
    Type *type = expect_type();
    Token* token = consume_ident();
    Var *lvar = new_lvar(substr(token->str, token->len), type);
    expect(";");
    return new_node(ND_NULL);
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
    node = new_lr_node(ND_ASSIGN, node, assign());
  }
  return node;
}

// equality   = relational ("==" relational | "!=" relational)*
Node *equality (){
  Node *node = relational();

  for (;;){
    if (consume("==")){
      node = new_lr_node(ND_EQ, node, relational());
    } else if (consume("!=")){
      node = new_lr_node(ND_NE, node, relational());
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
      node = new_lr_node(ND_LE, node, add());
    } else if (consume("<")){
      node = new_lr_node(ND_LT, node, add());
    } else if (consume(">=")){
      node = new_lr_node(ND_LE, add(), node);
    } else if (consume(">")){
      node = new_lr_node(ND_LT, add(), node);
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
      node = new_add_node(node, mul());
    } else if (consume("-")){
      node = new_sub_node(node, mul());
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
      node = new_lr_node(ND_MUL, node, unary());
    } else if (consume("/")){
      node = new_lr_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

// unary = ("+" | "-")* primary
//      | "*" unary (= *hoge)
//      | "&" unary (= &hoge)
//      | "sizeof" unary
Node *unary(){
  if (consume("+")){
    return unary();
  }
  if (consume("-")){
    return new_lr_node(ND_SUB, new_num_node(0), unary());
  }
  if (consume("*")){
    Node *node = new_node(ND_DEREF);
    set_lhs(node, unary());
    return node;
  }
  if (consume("&")){
    Node *node = new_node(ND_ADDR);
    set_lhs(node, unary());
    return node;
  }
  if (consume("sizeof")){
    Node *n = unary();
    return new_num_node(n->type->size);
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
