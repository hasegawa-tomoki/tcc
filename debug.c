#include "tcc.h"

char *token_name(int kind){
  static char *token_kinds[] = {
      "TK_RESERVED",
      "TK_RETURN",
      "TK_IDENT", 
      "TK_NUM",
      "TK_SIZEOF", 
      "TK_EOF",
  };
  return token_kinds[kind];
}

char *node_name(int kind){
    static char *node_kinds[] = {
      "ND_ADD", 
      "ND_PTR_ADD", 
      "ND_SUB", 
      "ND_PTR_SUB",
      "ND_MUL", 
      "ND_DIV", 
      "ND_EQ", 
      "ND_NE", 
      "ND_LT", 
      "ND_LE", 
      "ND_ASSIGN", 
      "ND_VAR", 
      "ND_NUM", 
      "ND_RETURN", 
      "ND_IF", 
      "ND_WHILE", 
      "ND_FOR", 
      "ND_BLOCK", 
      "ND_FUNCCALL", 
      "ND_ADDR",
      "ND_DEREF",
      "ND_NULL", 
    };
    return node_kinds[kind];
}

char *type_name(int kind){
  static char *type_kinds[] = {
    "TY_INT", 
    "TY_PTR", 
  };
  return type_kinds[kind];
}

void show_node(Node *node, char *name, int indent){
    for (int i = 0; i < (indent + 1) * 2; i++){
      fprintf(stderr, " ");
    }

    fprintf(stderr, "-- %s  kind: %-20s  ", name, node_name(node->kind));
    if (node->kind == ND_VAR){
      fprintf(stderr, "  name: %s", node->var->name);
      fprintf(stderr, "  offset: %d", node->var->offset);
    }
    if (node->kind == ND_NUM){
      fprintf(stderr, "  val: %d", node->val);
    }
    if (node->kind == ND_FUNCCALL){
      fprintf(stderr, "  funcname: %s", node->funcname);
    }
    if (node->type){
      fprintf(stderr, "  type: %s", type_name(node->type->kind));
    }
    fprintf(stderr, "\n");
    if (node->lhs){
      show_node(node->lhs, "lhs ", indent + 1);
    }
    if (node->rhs){
      show_node(node->rhs, "rhs ", indent + 1);
    }
    if (node->cond){
      show_node(node->cond, "cond", indent + 1);
    }
    if (node->then){
      show_node(node->then, "then", indent + 1);
    }
    if (node->els){
      show_node(node->els, "else", indent + 1);
    }
    if (node->init){
      show_node(node->init, "init", indent + 1);
    }
    if (node->iterate){
      show_node(node->iterate, "iter", indent + 1);
    }
    if (node->body){
      show_node(node->body, "body", indent + 1);
    }
    if (node->args){
      for (Node *n = node->args; n; n = n->next){
        show_node(n, "args", indent + 1);
      }
    }
}

void show_nodes(Function *prog){
  fprintf(stderr, "** nodes\n");
  for (Node *node = prog->node; node; node = node->next){
      show_node(node, "node", 0);
  }
  fprintf(stderr, "** nodes end\n");
}

void show_token(Token *tok){
    char t[64];
    strncpy(t, tok->str, tok->len);
    t[tok->len] = '\0';

    fprintf(stderr, "  -- token  kind: %-20s  str: %s", token_name(tok->kind), t);
    if (tok->kind == TK_NUM){
      fprintf(stderr, "  val: %d", tok->val);
    }
    fprintf(stderr, "\n");
}

void show_tokens(Token *token){
  fprintf(stderr, "** tokens\n");
  for (Token *tok = token; tok->kind != TK_EOF; tok = tok->next){
    show_token(tok);
  }
  fprintf(stderr, "** tokens end\n");
}

void show_variable(VarList *var_list){
  Type *type = var_list->var->type;
  fprintf(stderr, "  -- variable  name: %s  offset: %d  type: %s", var_list->var->name, var_list->var->offset, type_name(type->kind));
  while (type->kind == TY_PTR){
    type = type->ptr_to;
    fprintf(stderr, " -> %s", type_name(type->kind));
  }
  fprintf(stderr, "\n");
}

void show_variables(VarList *var_list){
  fprintf(stderr, "** variables\n");
  for (VarList *vl = var_list; vl; vl = vl->next){
    show_variable(vl);
  }
 fprintf(stderr, "** variables end\n");
}