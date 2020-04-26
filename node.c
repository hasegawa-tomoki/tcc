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
      node->type = new_type(TY_INT);
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
      node->type = new_type(TY_INT);
      break;
    case ND_DEREF:
      if (! node->lhs->type->ptr_to){
        error("invalid pointer dereference.");
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
  error_tok(token, "invalid operands");
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
  error_tok(token, "Invalid operands");
}

Node *new_num_node(int val){
  Node *node = new_node(ND_NUM);
  node->val = val;
  node->type = new_type(TY_INT);

  return node;
}

Node *new_var_node(Var *var){
  Node *node = new_node(ND_VAR);
  node->var = var;
  node->type = var->type;
  return node;
}

Node *new_local_var_node(Token *tok){
  Var *lvar = find_lvar(tok);
  if (! lvar){
    error_at(token->str, "Undefined local variable.");
  }
  return new_var_node(lvar);
}

Node *new_global_var_node(Token *tok){
  Var *gvar = find_gvar(tok);
  if (! gvar){
    return NULL;
  }
  return new_var_node(gvar);
}
