#include "tcc.h"

char *filename;

int main(int argc, char **argv){
  if (argc != 2){
    error("引数の個数が正しくありません");
    return 1;
  }

  filename = argv[1];
  user_input = read_file(argv[1]);
  token = tokenize();
  //show_tokens(token);
  Function *prog = program();

  // Calc offset
  for (Function *fn = prog; fn; fn = fn->next){
    int offset = 0;
    for (VarList *vl = fn->locals; vl; vl = vl->next){
      offset = align_to(offset, vl->var->type->align);
      offset += vl->var->type->size;
      vl->var->offset = offset;
    }
    fn->stack_size = offset;
    //show_variables(fn->locals);
  }
  //show_nodes(prog);

  codegen(prog);

  return 0;
}

