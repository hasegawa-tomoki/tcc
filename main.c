#include "tcc.h"

int main(int argc, char **argv){
  if (argc != 2){
    error("引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];
  token = tokenize();
  //show_tokens(token);
  program();
  //show_nodes(code);

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // prologue
  printf("# --- prologue\n");
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", count_lvars() * 8);
  printf("# --- \n\n");

  for (int i = 0; code[i]; i++){
    gen(code[i]);
    printf("  pop rax\n");
    printf("# --- end %s\n\n", node_name(code[i]->kind));
  }

  // epilogue
  printf("# --- epilogue\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp;\n");
  printf("  ret\n");
  printf("# --- \n\n");
  return 0;
}

