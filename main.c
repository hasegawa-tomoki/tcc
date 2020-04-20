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
    for (Var *var = fn->locals; var; var = var->next){
      offset += 8;
      var->offset = offset;
    }
    fn->stack_size = offset;
  }
  //show_nodes(prog);

  codegen(prog);

  return 0;
}

