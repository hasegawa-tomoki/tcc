#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenize.c

extern char *keywords[];
extern char *typenames[];

typedef enum {
  TK_RESERVED,
  TK_RETURN,
  TK_IDENT, 
  TK_NUM,
  TK_SIZEOF, 
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
  TY_ARRAY, 
} TypeKind;


typedef struct Type Type;
struct Type {
  TypeKind kind;
  Type *ptr_to;
  int size;
  int array_len;
};

typedef struct Var Var;
struct Var {
  char *name;
  Type* type;
  int offset;
  bool is_global;
};

typedef struct VarList VarList;
struct VarList {
  VarList *next;
  Var *var;
};

extern Token *token;
extern char *user_input;
extern VarList *locals;
extern VarList *globals;

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

// node.c

typedef enum {
  ND_ADD, // +
  ND_PTR_ADD, 
  ND_SUB, // -
  ND_PTR_SUB, 
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
  ND_NULL, 
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *next;
  Type *type;
  Token *token;

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

void set_lhs(Node *node, Node *lhs);
void set_rhs(Node *node, Node *rhs);
Node *new_node(NodeKind kind);
Node *new_deref_node(Node *lhs);
Node *new_lr_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_add_node(Node *lhs, Node *rhs);
Node *new_sub_node(Node *lhs, Node *rhs);
Node *new_num_node(int val);
Node *new_var_node(Var *var);
Node *new_local_var_node(Token *tok);
Node *new_global_var_node(Token *tok);

// parse.c

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

void add_var2vl(VarList *var_list, Var *var);
void add_var2locals(Var *var);
void add_var2globals(Var *var);

// Var *new_var(char *name, Type *type);
// Var *new_array(char *name, Type *type);

Type *new_type(TypeKind kind);
Type *pointer_to(Type *type);
Type *expect_type();

VarList *read_func_param();
VarList *read_func_params();

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
Node *postfix();
Node *primary();

// var.c

Var *find_var(VarList *var_list, Token *tok);
Var *find_lvar(Token *tok);
Var *find_gvar(Token *tok);
Var *new_var();
void declare_gvar();
void declare_lvar();

// codegen.c

void asmc(char *fmt, ...);
void gen_lval(Node *node);
void gen_addr(Node *node);
void gen_stack_addr2value();
int local_label_no();
void gen(Node *node);
void codegen(Function *prog);

// debug.c

char *token_name(int kind);
char *node_name(int kind);
char *type_name(int kind);
char *node2str(Node *node);
void show_node(Node *node, char *name, int indent);
void show_nodes(Function *prog);
void show_token(Token *tok);
void show_tokens(Token *tok);
void show_variable(VarList *var_list);
void show_variables(VarList *var_list);

// util.c

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
char *substr(char *src, int len);
void debug(char *fmt, ...);
