#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *filename;

// tokenize.c

typedef enum {
  TK_RESERVED,
  TK_STR,
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

// Edit here to add type
typedef enum {
  TY_CHAR,
  TY_INT, 
  TY_PTR, 
  TY_ARRAY, 
  TY_STRUCT, 
} TypeKind;

typedef struct Type Type;
typedef struct Member Member;
struct Type {
  TypeKind kind;
  Type *ptr_to;
  int size;
  int array_len;
  int align;
  Member *members;
};

struct Member {
  Member *next;
  Type *type;
  char *name;
  int offset;
};

typedef struct Var Var;
struct Var {
  char *name;
  Type* type;
  int offset;
  bool is_global;

  char *contents;
  int contents_len;
};

typedef struct VarList VarList;
struct VarList {
  VarList *next;
  Var *var;
};

typedef struct VarScope VarScope;
struct VarScope {
  VarScope *next;
  Var *var;
  char *name;
  Type *type_def;
};

typedef struct TagScope TagScope;
struct TagScope {
  TagScope *next;
  char *name;
  Type *type;
};

extern Token *token;

extern char *user_input;
extern VarList *locals;
extern VarList *globals;
extern VarScope *var_scope;
extern TagScope *tag_scope;

typedef struct Scope Scope;
struct Scope {
  VarScope *var_scope;
  TagScope *tag_scope;
};

extern char *keywords[];

Type *consume_pointer(Type *type);
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
char *starts_with_reserved(char *p);
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
  ND_MEMBER ,
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
  // Used if kind == ND_MEMBER
  Member *member;
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
Member *find_member(Type *type, char *name);
Node *new_struct_ref_node(Node *lhs);
Node *new_lr_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_add_node(Node *lhs, Node *rhs);
Node *new_sub_node(Node *lhs, Node *rhs);
Node *new_num_node(int val);
Node *new_var_node(Var *var);
Node *new_local_var_node(Token *tok);
Node *new_global_var_node(Token *tok);

// type.c

// Edit here to add type
extern char *typenames[2];

int align_to(int n, int align);

Type *new_type(TypeKind kind, int size, int align);
Type *new_array_type(Type *ptr_to, int length);
Type *new_struct_type(Member *members);
Type *new_int_type();
Type *new_char_type();
Type *pointer_to(Type *type);

TagScope *find_tag(Token *tok);
Member *struct_member();
Type *expect_type();
bool peek_type();

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

void add_var2locals(Var *var);
void add_var2globals(Var *var);

VarList *read_func_param();
VarList *read_func_params();

bool is_function();

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

VarScope *find_var(Token *tok);
Var *find_scope_var(Token *tok);
Var *find_global_var(Token *tok);
void push_var(Var *var);
void push_typedef(char *name, Type *type_def);
Type *read_type_suffix(Type *type);
Var *new_var();
Var *declare_gvar();
Var *declare_lvar();

// codegen.c

void asmc(char *fmt, ...);
void gen_lval(Node *node);
void gen_addr(Node *node);
void gen_stack_addr2value();
int new_label_no();
char *new_text_literal_label();
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
void show_type(Type *type);
void show_variable(VarList *var_list);
void show_variables(VarList *var_list);
void show_member(Member *member);
void show_members(Member *members);
void show_var_scopes(VarScope *var_scopes);

// util.c

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_token(Token *tok, char *fmt, ...);
char *substr(char *src, int len);
void debug(char *fmt, ...);
char *read_file(char *path);
