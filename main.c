#include "tcc.h"

int count_lvars(){
  int count = 0;
  for (LVar *var = locals; var->next != NULL; var = var->next){
    count++;
  }
  return count;
}

int main(int argc, char **argv){
  if (argc != 2){
    error("引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];
  token = tokenize();
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", count_lvars() * 8);

  for (int i = 0; code[i]; i++){
    gen(code[i]);
    printf("  pop rax\n");
  }

  // epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp;\n");
  printf("  ret\n");
  return 0;
}

