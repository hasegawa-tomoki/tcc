#include "tcc.h"

Var *find_var(VarList *var_list, Token *tok){
  for (VarList *vl = var_list; vl; vl = vl->next){
    if (strlen(vl->var->name) == tok->len && !strncmp(tok->str, vl->var->name, tok->len)){
      return vl->var;
    }
  }
}

Var *find_scope_var(Token *tok){
  return find_var(scope, tok);
}

Var *find_local_var(Token *tok){
  return find_var(locals, tok);
}

Var *find_global_var(Token *tok){
  return find_var(globals, tok);
}

Var *new_var(){
  Type *type = expect_type();
  Token* token = consume_ident();
  char *var_name = substr(token->str, token->len);
  if (peek("[")){
    // array
    while(consume("[")){
      int length = expect_number();
      type = new_array_type(type, length);
      expect("]");
    }
  }
  Var *var = calloc(1, sizeof(Var));
  var->name = var_name;
  var->type = type;

  // Add to scope
  VarList *svl = calloc(1, sizeof(VarList));
  svl->var = var;
  svl->next = scope;
  scope = svl;

  return var;
}

void declare_gvar(){
  Var *var = new_var();
  var->is_global = true;
  expect(";");
  add_var2globals(var);
}

void declare_lvar(){
  Var *var = new_var();
  expect(";");
  add_var2locals(var);
}