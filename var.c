#include "tcc.h"

Var *find_var(VarList *var_list, Token *tok){
  for (VarList *vl = var_list; vl; vl = vl->next){
    if (strlen(vl->var->name) == tok->len && !strncmp(tok->str, vl->var->name, tok->len)){
      return vl->var;
    }
  }
  return NULL;
}

Var *find_scope_var(Token *tok){
  return find_var(var_scope, tok);
}

Var *find_global_var(Token *tok){
  return find_var(globals, tok);
}

void push_var2scope(Var *var){
  VarList *svl = calloc(1, sizeof(VarList));
  svl->var = var;
  svl->next = var_scope;
  var_scope = svl;
}

Type *read_type_suffix(Type *type){
  if (peek("[")){
    // array
    while(consume("[")){
      int length = expect_number();
      type = new_array_type(type, length);
      expect("]");
    }
  }
  return type;
}

Var *new_var(){
  Type *type = expect_type();
  Token* token = consume_ident();
  if (! token){
    return NULL;
  }
  char *var_name = substr(token->str, token->len);
  type = read_type_suffix(type);

  Var *var = calloc(1, sizeof(Var));
  var->name = var_name;
  var->type = type;

  push_var2scope(var);

  return var;
}

void declare_gvar(){
  Var *var = new_var();
  var->is_global = true;
  expect(";");
  add_var2globals(var);
}

Var *declare_lvar(){
  Var *var = new_var();
  if (var){
    add_var2locals(var);
    return var;
  }
  return NULL;
}