#include "tcc.h"

char *token_name(int kind){
  static char *token_kinds[] = {
      "TK_RESERVED",
      "TK_STR",
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
      "ND_MEMBER", 
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
    "TY_CHAR", 
    "TY_INT", 
    "TY_PTR", 
    "TY_ARRAY", 
    "TY_STRUCT", 
  };
  return type_kinds[kind];
}

char *node2str(Node *node){
  char *str = calloc(512, sizeof(char));

  sprintf(str, "%s (", node_name(node->kind));
  if (node->kind == ND_VAR){
    if (node->var->is_global){
      sprintf(str, "%s name: %s (global),", str, node->var->name);
    } else {
      sprintf(str, "%s name: %s,", str, node->var->name);
    }
    sprintf(str, "%s offset: %d,", str, node->var->offset);
    sprintf(str, "%s aligh: %d,", str, node->var->type->align);
  }
  if (node->kind == ND_NUM){
    sprintf(str, "%s val: %d,", str, node->val);
  }
  if (node->kind == ND_FUNCCALL){
    sprintf(str, "%s funcname: %s,", str, node->funcname);
  }
  if (node->type){
    Type *type = node->type;
    sprintf(str, "%s type: %s", str, type_name(type->kind));
    while (type->kind == TY_PTR || type->kind == TY_ARRAY){
      type = type->ptr_to;
      sprintf(str, "%s -> %s", str, type_name(type->kind));
    }
    sprintf(str, "%s,", str);
  }
  sprintf(str, "%s )", str);
  
  return str;
}

void show_node(Node *node, char *name, int indent){
    for (int i = 0; i < (indent + 1) * 2; i++){
      fprintf(stderr, " ");
    }

    fprintf(stderr, "-- %s  %s\n", name, node2str(node));

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

void show_type(Type *type){
  fprintf(stderr, "type: %s", type_name(type->kind));
  while (type->kind == TY_PTR || type->kind == TY_ARRAY){
    type = type->ptr_to;
    fprintf(stderr, " -> %s", type_name(type->kind));
  }
}

void show_variable(VarList *var_list){
  Type *type = var_list->var->type;
  fprintf(stderr, "  -- variable  name: %s  offset: %d  align: %d  ", var_list->var->name, var_list->var->offset, var_list->var->type->align);
  fprintf(stderr, "size: %d  ", type->size);
  show_type(type);
  fprintf(stderr, "\n");
  if (type->kind == TY_STRUCT){
    for (Member *mem = type->members; mem; mem = mem->next){
      fprintf(stderr, "    -- member: name: %s  offset: %d  ", mem->name, mem->offset);
      show_type(mem->type);
      fprintf(stderr, "\n");
    }
  }
}

void show_variables(VarList *var_list){
  fprintf(stderr, "** variables\n");
  for (VarList *vl = var_list; vl; vl = vl->next){
    show_variable(vl);
  }
 fprintf(stderr, "** variables end\n");
}

void show_member(Member *member){
  fprintf(stderr, "  -- member  name: %s  offset: %d  size: %d  ", member->name, member->offset, member->type->size);
  show_type(member->type);
  fprintf(stderr, "\n");
}

void show_members(Member *members){
  fprintf(stderr, "** members\n");
  for (Member *mem = members; mem; mem = mem->next){
    show_member(mem);
  }
  fprintf(stderr, "** members end\n");
}