#include "tcc.h"

int main(int argc, char **argv){
  if (argc != 2){
    error("引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];
  token = tokenize();
  //show_tokens(token);
  Function *prog = program();
  
  // Calc offset
  for (Function *fn = prog; fn; fn = fn->next){
    int offset = 0;
    for (VarList *var_list = fn->locals; var_list; var_list = var_list->next){
      offset += 8;
      var_list->var->offset = offset;
    }
    fn->stack_size = offset;
    //show_variables(fn->locals);
  }
  show_nodes(prog);

  codegen(prog);

  return 0;
}

