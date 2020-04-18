#include "tcc.h"

int count_lvars(){
  int count = 0;
  for (LVar *var = locals; var->next != NULL; var = var->next){
    count++;
  }
  return count;
}

LVar *find_lvar(Token *tok){
  for (LVar *var = locals; var->next != NULL; var = var->next){
    if (var->len == tok->len && (! memcmp(tok->str, var->name, var->len))){
      return var;
    }
  }
  return NULL;
}
