#include "tcc.h"

Var *find_lvar(Token *tok){
  for (VarList *var_list = locals; var_list; var_list = var_list->next){
    if (strlen(var_list->var->name) == tok->len && !strncmp(tok->str, var_list->var->name, tok->len)){
      return var_list->var;
    }
  }
  return NULL;
}
