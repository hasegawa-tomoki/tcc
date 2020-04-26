#include "tcc.h"

Var *find_var(VarList *var_list, Token *tok){
  for (VarList *vl = var_list; vl; vl = vl->next){
    if (strlen(vl->var->name) == tok->len && !strncmp(tok->str, vl->var->name, tok->len)){
      return vl->var;
    }
  }
}

Var *find_lvar(Token *tok){
  return find_var(locals, tok);
}

Var *find_gvar(Token *tok){
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
      Type *arr = new_type(TY_ARRAY);
      arr->array_len = length;
      arr->size = length * type->size;
      arr->ptr_to = type;
      type = arr;
      expect("]");
    }
  }
  Var *var = calloc(1, sizeof(Var));
  var->name = var_name;
  var->type = type;
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