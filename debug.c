#include "tcc.h"

char *node_name(int kind){
    static char *node_kinds[] = {
      "ND_ADD", 
      "ND_SUB", 
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
    };
    return node_kinds[kind];
}

void show_node(Node *node, char *name, int indent){
    for (int i = 0; i < (indent + 1) * 2; i++){
      fprintf(stderr, " ");
    }

    fprintf(stderr, "-- %s  kind: %-20s  ", name, node_name(node->kind));
    if (node->kind == ND_VAR){
      fprintf(stderr, "  offset: %d", node->var->offset);
    }
    if (node->kind == ND_NUM){
      fprintf(stderr, "  val: %d", node->val);
    }
    fprintf(stderr, "\n");
    if (node->lhs){
      show_node(node->lhs, "lhs", indent + 1);
    }
    if (node->rhs){
      show_node(node->rhs, "rhs", indent + 1);
    }
    if (node->cond){
      show_node(node->cond, "cond", indent + 1);
    }
    if (node->then){
      show_node(node->then, "then", indent + 1);
    }
    if (node->els){
      show_node(node->els, "els", indent + 1);
    }
    if (node->init){
      show_node(node->init, "init", indent + 1);
    }
    if (node->iterate){
      show_node(node->iterate, "iterate", indent + 1);
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

    static char *token_kinds[] = {
      "TK_RESERVED",
      "TK_RETURN",
      "TK_IDENT," 
      "TK_NUM",
      "TK_EOF",
    };

    fprintf(stderr, "  -- token  kind: %-20s  str: %s", token_kinds[tok->kind], t);
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
