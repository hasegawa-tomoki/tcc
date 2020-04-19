#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// util.c

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

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

typedef struct LVar LVar;
struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

extern Token *token;
extern char *user_input;
extern LVar *locals;

bool consume(char *op);
Token *consume_ident();
bool expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
bool is_alpha(char c);
bool is_alnum(char c);
void show_token(Token *tok);
void show_tokens(Token *tok);
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
  ND_LVAR, // local variable
  ND_NUM, // integer
  ND_RETURN, // return
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
  int offset;
};

extern Node *code[100];

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void show_node(Node *node, int indent);
void show_nodes(Node *code[]);

// lvar.c

int count_lvars();
LVar *find_lvar(Token *tok);

// codegen.c

void gen(Node *node);
