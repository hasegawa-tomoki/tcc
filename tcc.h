#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// util.c

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
char *substr(char *src, int len);

// tokenize.c

typedef enum {
  TK_RESERVED,
  TK_RETURN,
  TK_IDENT, 
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

typedef enum {
  TY_INT, 
  TY_PTR, 
} TypeKind;

typedef struct Type Type;
struct Type {
  TypeKind kind;
  Type *ptr_to;
};

typedef struct Var Var;
struct Var {
  char *name;
  Type* type;
  int offset;
};

typedef struct VarList VarList;
struct VarList {
  VarList *next;
  Var *var;
};

extern Token *token;
extern char *user_input;
extern VarList *locals;

bool consume(char *op);
Token *consume_ident();
bool expect(char *op);
int expect_number();
char *expect_ident();
bool peek(char *op);
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
bool is_alpha(char c);
bool is_alnum(char c);
Token *tokenize();

// parse.c

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // == 
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_ASSIGN, // =
  ND_VAR, // local variable
  ND_NUM, // integer
  ND_RETURN, // return
  ND_IF, 
  ND_WHILE, 
  ND_FOR, 
  ND_BLOCK,
  ND_FUNCCALL, 
  ND_ADDR, // address &
  ND_DEREF, // deference *
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *next;

  // Left/right hand side
  Node *lhs;
  Node *rhs;
  // Used if kind == ND_NUM
  int val;
  // Used if kind == ND_VAR
  Var *var;
  //int offset;
  // "if" "(" cond ")" then "else" els
  Node *cond;
  Node *then;
  Node *els;
  // "for" "(" init ";" cond ";" iterate ")" then
  Node *init;
  Node *iterate;
  // Block
  Node *body;
  // Function Call
  char *funcname;
  Node *args;
};

typedef struct Function Function;
struct Function {
  Function *next;
  Type *type;
  char *name;
  VarList *params;
  Node *node;
  VarList *locals;
  int stack_size;
};

extern Node *code[100];

Node *new_node(NodeKind kind);
Node *new_node_with_lrs(NodeKind kind, Node *lhs, Node *rhs);
Node *new_num_node(int val);

Function *program();
Function *function();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
char *node_name(int kind);

// lvar.c

Var *find_lvar(Token *tok);

// codegen.c

void gen(Node *node);
void codegen(Function *prog);

// debug.c

void show_node(Node *node, char *name, int indent);
void show_nodes(Function *prog);
void show_token(Token *tok);
void show_tokens(Token *tok);
void show_variable(VarList *var_list);
void show_variables(VarList *var_list);
