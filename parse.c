#include "tcc.h"

void add_var2locals(Var *var){
  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = var;
  vl->next = locals;
  locals = vl;
}

void add_var2globals(Var *var){
  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = var;
  vl->next = globals;
  globals = vl;
}

VarList *read_func_param(){
    VarList *vl = calloc(1, sizeof(VarList));
    vl->var = new_var();
    add_var2locals(vl->var);
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

bool is_function(){
  Token *saved = token;
  Type *type = expect_type();
  char *ident = expect_ident();
  bool is_func = consume("(");
  token = saved;
  return is_func;
}

// program    = function*
Function *program(){
  Function head = {};
  Function *cur = &head;

  while (! at_eof()){
    if (is_function()){
      cur->next = function();
      cur = cur->next;
    } else {
      // global variable declaration
      declare_gvar();
    }
  }

  return head.next;
}

// function = ident "(" ")" "{" stmt* "}"
Function *function(){
  locals = NULL;

  VarList *sc = scope;

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

  scope = sc;
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
    VarList *sc = scope;

    Node head = {};
    Node *cur = &head;
    while (!consume("}")){
      cur->next = stmt();
      cur = cur->next;
    }
    node = new_node(ND_BLOCK);
    node->body = head.next;

    scope = sc;
    return node;
  }

  if (peek_type()){
    declare_lvar();
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

// unary = ("+" | "-")* unary
//      | ("*" | "&") unary (= *hoge, &hoge)
//      | "sizeof" unary
//      | postfix
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
  return postfix();
}

// postfix = primary ("[" expr "]")*
Node *postfix(){
  Node *node = primary();

  // x[y] = *(x + y)
  while(consume("[")){
    Node *exp = new_add_node(node, expr());
    expect("]");
    node = new_deref_node(exp);
  }
  return node;
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
    Node *gvar = new_global_var_node(tok);
    if (gvar){
      return gvar;
    }

    return new_local_var_node(tok);
  }

  if (token->kind == TK_STR){
    Var *var = calloc(1, sizeof(Var));
    var->name = new_text_literal_label();
    var->type = new_array_type(new_type(TY_CHAR), token->len);
    var->is_global = true;

    var->contents = token->str;
    var->contents_len = token->len;

    add_var2globals(var);

    Node *node = new_node(ND_VAR);
    node->var = var;
    node->type = var->type;

    token = token->next;
    
    return node;
  }

  return new_num_node(expect_number());
}
