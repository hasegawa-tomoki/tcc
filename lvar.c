#include "tcc.h"

int count_lvars(){
  int count = 0;
  for (Var *var = locals; var->next != NULL; var = var->next){
    count++;
  }
  return count;
}

Var *find_lvar(Token *tok){
  for (Var *var = locals; var; var = var->next){
    if (strlen(var->name) == tok->len && !strncmp(tok->str, var->name, tok->len)){
      return var;
    }
  }
  return NULL;
}
