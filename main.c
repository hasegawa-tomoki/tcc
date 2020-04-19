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
  int offset = 0;
  for (Var *var = prog->locals; var; var = var->next){
    offset += 8;
    var->offset = offset;
  }
  prog->stack_size = offset;
  //show_nodes(prog);


  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // prologue
  printf("# --- prologue\n");
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", prog->stack_size);
  printf("# --- \n\n");

  for (Node *node = prog->node; node; node = node->next){
    gen(node);
    printf("  pop rax\n");
    printf("# --- end %s\n\n", node_name(node->kind));
  }

  // epilogue
  printf("# --- epilogue\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp;\n");
  printf("  ret\n");
  printf("# --- \n\n");
  return 0;
}

