#include "tcc.h"

static char *argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static char *funcname;
static int indent;

void asmc(char *fmt, ...){
  bool debug = false;

  va_list ap;
  va_start(ap, fmt);
  if (debug){
    for (int i = 0; i < (indent + 1) * 2; i++){
      fprintf(stderr, " ");
    }
    vfprintf(stderr, fmt, ap);
  } else {
    for (int i = 0; i < (indent + 1) * 2; i++){
      fprintf(stdout, " ");
    }
    vfprintf(stdout, fmt, ap);
  }
}

void gen_lval(Node *node){
  indent++;
  asmc("# --- gen_lval start %s\n", node2str(node));
  if (node->kind == TY_ARRAY){
    //show_node(node, "gen_lval", 0);
    error("not an lvalue");
  }
  gen_addr(node);
  indent--;
}

void gen_addr(Node *node){
  indent++;
  asmc("# --- gen_addr ( %s ){\n", node2str(node));
  switch (node->kind){
    case ND_VAR:
      if (node->var->is_global){
        printf("  lea rax, [%s]\n", node->var->name);
      } else {
        printf("  lea rax, [rbp - %d]\n", node->var->offset);
      }
      printf("  push rax\n");
      break;
    case ND_DEREF:
      gen(node->lhs);
      break;
    case ND_MEMBER:
      gen_addr(node->lhs);
      printf("  pop rax\n");
      printf("  add rax, %d\n", node->member->offset);
      printf("  push rax\n");
      break;
    default:
      error("not an lvalue");
  }
  asmc("# --- gen_addr }\n");
  indent--;
}

// value < [address]
void load(Type *type){
  indent++;
  asmc("# --- load {\n");
  printf("  pop rax\n");
  if (type->size == 1){
    printf("  movsx rax, byte ptr [rax]\n");
  } else {
    // size = 8 (64bit)
    printf("  mov rax, [rax]\n");
  }
  printf("  push rax\n");
  asmc("# --- load }\n");
  indent--;
}

// value > [addres]
void store(Type *type){
  indent++;
  asmc("# --- store {\n");
  printf("  pop rdi\n");
  printf("  pop rax\n");
  if (type->size == 1){
    printf("  mov [rax], dil\n");
  } else {
    // size = 8 (64bit)
    printf("  mov [rax], rdi\n");
  }
  printf("  push rdi\n");
  asmc("# --- store }\n");
  indent--;
}

int new_label_no(){
  static int no = 0;
  return no++;
}

char *new_text_literal_label(){
  static int no = 0;

  char label[20];
  sprintf(label, ".L.text.%d", no++);

  return substr(label, 20);
}

void gen(Node *node){
  asmc("# --- gen ( %s ){\n", node2str(node));
  switch (node->kind){
    case ND_NULL:
      asmc("# --- gen } %s\n", node_name(node->kind));
      return;
    case ND_NUM:
      printf("  push %d\n", node->val);
      asmc("# --- gen } %s\n", node_name(node->kind));
      return;
    case ND_MEMBER:
    case ND_VAR:
      gen_addr(node);
      if (node->type->kind != TY_ARRAY){
        load(node->type);
      }
      asmc("# --- gen } %s\n", node_name(node->kind));
      return;
    case ND_ASSIGN:
      indent++;
      asmc("# --- gen %s: lhs \n", node_name(node->kind));
      gen_lval(node->lhs);
      asmc("# --- gen %s: rhs \n", node_name(node->kind));
      indent++;
      gen(node->rhs);
      indent--;
      asmc("# --- gen %s: assign\n", node_name(node->kind));
      store(node->lhs->type);
      indent--;
      asmc("# --- gen } %s\n", node_name(node->kind));
      return;
    case ND_RETURN:
      indent++;
      gen(node->lhs);
      asmc("# --- gen %s: return\n", node_name(node->kind));
      printf("  pop rax\n");
      printf("  jmp .Lreturn.%s\n", funcname);
      indent--;
      asmc("# --- gen } %s\n", node_name(node->kind));
      return;
    case ND_IF: {
      int label_no = new_label_no();
      indent++;
      asmc("# --- gen %s: cond {\n", node_name(node->kind));
      indent++;
      gen(node->cond);
      indent--;
      asmc("# --- gen %s: cond }\n", node_name(node->kind));
      asmc("# --- gen %s: branch {\n", node_name(node->kind));
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      asmc("# --- gen %s: branch }\n", node_name(node->kind));
      if (node->els){
        asmc("# --- gen %s: then {\n", node_name(node->kind));
        printf("  je .Lels_%d\n", label_no);
        indent++;
        gen(node->then);
        indent--;
        printf("  jmp .Lend_%d\n", label_no);
        asmc("# --- gen %s: then }\n", node_name(node->kind));
        asmc("# --- gen %s: else {\n", node_name(node->kind));
        printf(".Lels_%d:\n", label_no);
        indent++;
        gen(node->els);
        indent--;
        asmc("# --- gen %s: else }\n", node_name(node->kind));
      } else {
        asmc("# --- gen %s: then {\n", node_name(node->kind));
        printf("  je .Lend_%d\n", label_no);
        indent++;
        gen(node->then);
        indent--;
        asmc("# --- gen %s: then }\n", node_name(node->kind));
      }
      indent--;
      printf(".Lend_%d:\n", label_no);
      asmc("# --- gen } %s\n", node_name(node->kind));
      return;
    }
    case ND_WHILE: {
      int label_no = new_label_no();
      indent++;
      printf(".Lbegin_%d:\n", label_no);
      asmc("# --- gen %s: cond {\n", node_name(node->kind));
      indent++;
      gen(node->cond);
      indent--;
      asmc("# --- gen %s: cond }\n", node_name(node->kind));
      asmc("# --- gen %s: branch {\n", node_name(node->kind));
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend_%d\n", label_no);
      asmc("# --- gen %s: branch }\n", node_name(node->kind));
      asmc("# --- gen %s: then {\n", node_name(node->kind));
      indent++;
      gen(node->then);
      indent--;
      asmc("# --- gen %s: then }\n", node_name(node->kind));
      printf("  jmp .Lbegin_%d\n", label_no);
      indent--;
      printf(".Lend_%d:\n", label_no);
      asmc("# --- gen } %s\n", node_name(node->kind));
      return;
    }
    case ND_FOR: {
      int label_no = new_label_no();
      indent++;
      asmc("# --- gen %s: init {\n", node_name(node->kind));
      indent++;
      gen(node->init);
      indent--;
      asmc("# --- gen %s: init }\n", node_name(node->kind));
      printf(".Lbegin_%d:\n", label_no);
      asmc("# --- gen %s: cond {\n", node_name(node->kind));
      indent++;
      gen(node->cond);
      indent--;
      asmc("# --- gen %s: cond }\n", node_name(node->kind));
      asmc("# --- gen %s: branch {\n", node_name(node->kind));
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend_%d\n", label_no);
      asmc("# --- gen %s: branch }\n", node_name(node->kind));
      asmc("# --- gen %s: then {\n", node_name(node->kind));
      indent++;
      gen(node->then);
      indent--;
      asmc("# --- gen %s: then }\n", node_name(node->kind));
      asmc("# --- gen %s: iterate {\n", node_name(node->kind));
      indent++;
      gen(node->iterate);
      indent--;
      asmc("# --- gen %s: iterate }\n", node_name(node->kind));
      
      indent--;
      printf(".Lend_%d:\n", label_no);
      asmc("# --- gen } %s\n", node_name(node->kind));
      return;
    }
    case ND_BLOCK:
      for (Node *n = node->body; n; n = n->next){
        asmc("# --- {\n");
        indent++;
        gen(n);
        indent--;
        asmc("# --- }\n");
      }
      asmc("# --- gen } %s\n", node_name(node->kind));
      return;
    case ND_FUNCCALL: {
      indent++;
      asmc("# --- gen %s: args push {\n", node_name(node->kind));
      indent++;
      int argcnt = 0;
      for (Node *arg = node->args; arg; arg = arg->next){
        gen(arg);
        argcnt++;
      }
      indent--;
      asmc("# --- gen %s: args push }\n", node_name(node->kind));
      asmc("# --- gen %s: args pop {\n", node_name(node->kind));
      indent++;
      for (int i = argcnt - 1; i >= 0; i--){
        printf("  pop %s\n", argregs[i]);
      }
      indent--;
      asmc("# --- gen %s: args pop }\n", node_name(node->kind));
      asmc("# --- gen %s: call {\n", node_name(node->kind));
      indent++;
      int label_no = new_label_no();
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
      indent--;
      asmc("# --- gen %s: call }\n", node_name(node->kind));
      indent--;
      asmc("# --- gen end %s\n", node_name(node->kind));
      return;
    }
    // &hoge
    case ND_ADDR:
      gen_addr(node->lhs);
      asmc("# --- gen end %s\n", node_name(node->kind));
      return;
    // *hoge
    case ND_DEREF:
      indent++;
      gen(node->lhs);
      indent--;
      if (node->type->kind != TY_ARRAY){
        load(node->type);
      }
      asmc("# --- gen end %s\n", node_name(node->kind));
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
  indent = 0;

  printf("# --- global variables\n");
  printf(".data\n");
  for (VarList *vl = globals; vl; vl = vl->next){
    printf("%s:\n", vl->var->name);
    if (vl->var->contents){
      printf("  .string \"%s\"", vl->var->contents);
    } else {
      printf("  .zero %d\n", vl->var->type->size);
    }
  }
  printf("\n");
  
  printf(".text\n");
  for (Function *fn = prog; fn; fn = fn->next){
    funcname = fn->name;

    printf(".global %s\n", fn->name);
    printf("# %s\n\n", user_input);

    printf("%s:\n", fn->name);

    // prologue
    asmc("# --- prologue\n");
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", fn->stack_size);
    printf("  # --- write function parameters to stack\n");
    int idx = 0;
    for (VarList *vl = fn->params; vl; vl = vl->next){
      printf("  mov [rbp - %d], %s\n", vl->var->offset, argregs[idx]);
      idx++;
    }
    asmc("# --- \n\n");

    for (Node *node = fn->node; node; node = node->next){
      gen(node);
    }

    // epilogue
    printf(".Lreturn.%s:\n", fn->name);
    asmc("# --- epilogue\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    asmc("# --- \n\n");
  }
}