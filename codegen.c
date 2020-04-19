#include "tcc.h"

void gen_lval(Node *node){
  if (node->kind != ND_LVAR){
    error("not an lvalue");
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

int local_label_no(){
  static int no = 0;
  return no++;
}

void gen(Node *node){
  int label_no;
  printf("# --- %s\n", node_name(node->kind));
  switch (node->kind){
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_IF:
      label_no = local_label_no();
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      if (node->els){
        printf("  je .Lels_%d\n", label_no);
        gen(node->then);
        printf("  jmp .Lend_%d\n", label_no);
        printf(".Lels_%d:\n", label_no);
        gen(node->els);
      } else {
        printf("  je .Lend_%d\n", label_no);
        gen(node->then);
      }
      printf(".Lend_%d:\n", label_no);
      return;
    case ND_WHILE:
      label_no = local_label_no();
      printf(".Lbegin_%d:\n", label_no);
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend_%d\n", label_no);
      gen(node->then);
      printf("  jmp .Lbegin_%d\n", label_no);
      printf(".Lend_%d:\n", label_no);
      return;
    case ND_FOR:
      label_no = local_label_no();
      gen(node->init);
      printf(".Lbegin_%d:\n", label_no);
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend_%d\n", label_no);
      gen(node->then);
      gen(node->iterate);
      printf(".Lend_%d:\n", label_no);
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind){
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  div rdi\n");
      break;
    case ND_EQ:  // == 
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      break;
    case ND_NE:  // !=
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      break;
    case ND_LT:  // <
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      break;
    case ND_LE:  // <=
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      break;
  }

  printf("  push rax\n");
}
