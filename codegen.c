#include "tcc.h"

static char *argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static char *funcname;

void gen_lval(Node *node){
  if (node->kind == TY_ARRAY){
    show_node(node, "gen_lval", 0);
    error("not an lvalue");
  }
  gen_addr(node);
}

void gen_addr(Node *node){
  asmcomment("# --- gen_addr %s\n", node_name(node->kind));
  switch (node->kind){
    case ND_VAR:
      printf("  lea rax, [rbp - %d]\n", node->var->offset);
      printf("  push rax\n");
      break;
    case ND_DEREF:
      gen(node->lhs);
      break;
    default:
      error("not an lvalue");
  }
}

void gen_stack_addr2value(){
  asmcomment("# --- gen_stack_addr2value\n");
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

int local_label_no(){
  static int no = 0;
  return no++;
}

void gen(Node *node){
  asmcomment("# --- gen %s\n", node_name(node->kind));
  switch (node->kind){
    case ND_NULL:
      return;
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_VAR:
      gen_addr(node);
      if (node->type->kind != TY_ARRAY){
        gen_stack_addr2value();
      }
      return;
    case ND_ASSIGN:
      asmcomment("  # --- gen %s: lhs \n", node_name(node->kind));
      gen_lval(node->lhs);
      asmcomment("  # --- gen %s: rhs \n", node_name(node->kind));
      gen(node->rhs);
      asmcomment("  # --- gen %s: assign\n", node_name(node->kind));
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  jmp .Lreturn.%s\n", funcname);
      return;
    case ND_IF: {
      int label_no = local_label_no();
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
    }
    case ND_WHILE: {
      int label_no = local_label_no();
      printf(".Lbegin_%d:\n", label_no);
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend_%d\n", label_no);
      gen(node->then);
      printf("  jmp .Lbegin_%d\n", label_no);
      printf(".Lend_%d:\n", label_no);
      return;
    }
    case ND_FOR: {
      int label_no = local_label_no();
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
    case ND_BLOCK:
      for (Node *n = node->body; n; n = n->next){
        gen(n);
      }
      return;
    case ND_FUNCCALL: {
      asmcomment("# --- gen %s :args\n", node_name(node->kind));
      int argcnt = 0;
      for (Node *arg = node->args; arg; arg = arg->next){
        gen(arg);
        argcnt++;
      }
      for (int i = argcnt - 1; i >= 0; i--){
        printf("  pop %s\n", argregs[i]);
      }
      asmcomment("# --- gen %s :call\n", node_name(node->kind));

      int label_no = local_label_no();
      printf("  mov rax, rsp\n");
      printf("  and rax, 15\n");
      printf("  jnz .Lcall_%d\n", label_no);
      printf("  mov rax, 0\n");
      printf("  call %s\n", node->funcname);
      printf("  jmp .Lend_%d\n", label_no);
      printf(".Lcall_%d:\n", label_no);
      printf("  sub rsp, 8\n");
      printf("  mov rax, 0\n");
      printf("  call %s\n", node->funcname);
      printf("  add rsp, 8\n");
      printf(".Lend_%d:\n", label_no);
      printf("  push rax\n");
      return;
    }
    // &hoge
    case ND_ADDR:
      gen_addr(node->lhs);
      return;
    // *hoge
    case ND_DEREF:
      gen(node->lhs);
      if (node->type->kind != TY_ARRAY){
        gen_stack_addr2value();
      }
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
    case ND_PTR_ADD:
      printf("  imul rdi, %d\n", node->lhs->type->ptr_to->size);
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_PTR_SUB:
      printf("  imul rdi, %d\n", node->lhs->type->ptr_to->size);
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

void codegen(Function *prog){
  printf(".intel_syntax noprefix\n");

  for (Function *fn = prog; fn; fn = fn->next){
    funcname = fn->name;

    printf(".global %s\n", fn->name);
    printf("%s:\n", fn->name);

    // prologue
    asmcomment("# --- prologue\n");
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", fn->stack_size);
    printf("  # --- write function parameters to stack\n");
    int idx = 0;
    for (VarList *vl = fn->params; vl; vl = vl->next){
      printf("  mov [rbp - %d], %s\n", vl->var->offset, argregs[idx]);
      idx++;
    }
    asmcomment("# --- \n\n");

    for (Node *node = fn->node; node; node = node->next){
      gen(node);
    }

    // epilogue
    printf(".Lreturn.%s:\n", fn->name);
    asmcomment("# --- epilogue\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp;\n");
    printf("  ret\n");
    asmcomment("# --- \n\n");
  }
}