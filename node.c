#include "tcc.h"

void set_lhs(Node *node, Node *lhs){
  node->lhs = lhs;
  
  switch (node->kind){
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_FUNCCALL:
    case ND_NUM:
      node->type = new_int_type();
      break;
    case ND_PTR_ADD:
    case ND_PTR_SUB:
    case ND_ASSIGN:
      node->type = node->lhs->type;
      break;
    case ND_VAR:
      node->type = node->var->type;
      break;
    case ND_ADDR:
      // chibicc のコミットではこうなっているけど、アドレス計算は TY_INT では？
      //node->type = pointer_to(node->lhs->type);
      node->type = new_int_type();
      break;
    case ND_DEREF:
      if (! node->lhs->type->ptr_to){
        error("Invalid pointer dereference.");
      }
      if (node->lhs->type->ptr_to->kind == TY_VOID){
        error("Dereferencing a void pointer.");
      }
      node->type = node->lhs->type->ptr_to;
      break;
  }
}
void set_rhs(Node *node, Node *rhs){
  node->rhs = rhs;
}

Node *new_node(NodeKind kind){
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->token = token;

  return node;
}

Node *new_deref_node(Node *lhs){
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_DEREF;
  node->token = token;
  set_lhs(node, lhs);

  return node;
}

Member *find_member(Type *type, char *name){
  for (Member *mem = type->members; mem; mem = mem->next){
    if (! strcmp(mem->name, name)){
      return mem;
    }
  }
  return NULL;
}

Node *new_struct_ref_node(Node *lhs){
  if (lhs->type->kind != TY_STRUCT){
    error_token(token, "Not a struct");
  }

  Member *mem = find_member(lhs->type, expect_ident());
  if (! mem){
    error_token(token, "Member not found");
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_MEMBER;
  set_lhs(node, lhs);
  node->member = mem;
  node->type = mem->type;

  return node;
}

Node *new_lr_node(NodeKind kind, Node *lhs, Node *rhs){
  Node *node = new_node(kind);
  set_lhs(node, lhs);
  set_rhs(node, rhs);

  return node;
}

Node *new_add_node(Node *lhs, Node *rhs){
  if (! lhs->type){
    show_node(lhs, "@new_add_node lhs", 0);
    error("type not found in lhs");
  }
  if (! rhs->type){
    show_node(rhs, "@new_add_node rhs", 0);
    error("type not found in rhs");
  }
  if (lhs->type->kind == TY_INT && rhs->type->kind == TY_INT){
    return new_lr_node(ND_ADD, lhs, rhs);
  } else if (lhs->type->ptr_to && rhs->type->kind == TY_INT){
    return new_lr_node(ND_PTR_ADD, lhs, rhs);
  } else if (lhs->type->kind == TY_INT && rhs->type->ptr_to){
    return new_lr_node(ND_PTR_ADD, rhs, lhs);
  }
  error_token(token, "invalid operands");
}

Node *new_sub_node(Node *lhs, Node *rhs){
  if (! lhs->type){
    show_node(lhs, "@new_sub_node lhs", 0);
    error("type not found in lhs");
  }
  if (! rhs->type){
    show_node(rhs, "@new_sub_node rhs", 0);
    error("type not found in rhs");
  }
  if (lhs->type->kind == TY_INT && rhs->type->kind == TY_INT){
    return new_lr_node(ND_SUB, lhs, rhs);
  } else if (lhs->type->ptr_to && rhs->type->kind == TY_INT){
    return new_lr_node(ND_PTR_SUB, lhs, rhs);
  } else if (lhs->type->kind == TY_INT && rhs->type->ptr_to){
    return new_lr_node(ND_PTR_SUB, rhs, lhs);
  }
  show_node(lhs, "lhs", 0);
  show_node(rhs, "rhs", 0);
  error_token(token, "Invalid operands");
}

Node *new_num_node(int val){
  Node *node = new_node(ND_NUM);
  node->val = val;
  node->type = new_int_type();

  return node;
}

Node *new_var_node(Var *var){
  Node *node = new_node(ND_VAR);
  node->var = var;
  node->type = var->type;
  return node;
}
