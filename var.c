#include "tcc.h"

VarScope *find_var(Token *tok){
  for (VarScope *vsc = var_scope; vsc; vsc = vsc->next){
    if (strlen(vsc->name) == tok->len && !strncmp(tok->str, vsc->name, tok->len)){
      return vsc;
    }
  }
  return NULL;
}

void push_var(Var *var){
  VarScope *vsc = calloc(1, sizeof(VarScope));
  vsc->var = var;
  vsc->name = var->name;
  vsc->next = var_scope;
  var_scope = vsc;
}

void push_typedef(char *name, Type *type_def){
  VarScope *vsc = calloc(1, sizeof(VarScope));
  vsc->type_def = type_def;
  vsc->name = name;
  vsc->next = var_scope;
  var_scope = vsc;
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

  push_var(var);

  return var;
}

Var *declare_gvar(){
  Var *var = new_var();
  if (var){
    var->is_global = true;
    expect(";");
    add_var2globals(var);
    return var;
  }
  return NULL;
}

Var *declare_lvar(){
  Var *var = new_var();
  if (var){
    add_var2locals(var);
    return var;
  }
  return NULL;
}