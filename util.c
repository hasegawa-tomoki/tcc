#include "tcc.h"

void error(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_tok(Token *tok, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  error_at(tok->str, fmt, ap);
}

char *substr(char *src, int len){
  char *name = calloc(len, sizeof(char));
  strncpy(name, src, len);
  return name;
}

void debug(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

void asmcomment(char *fmt, ...){
  bool debug = false;

  va_list ap;
  va_start(ap, fmt);
  if (debug){
    vfprintf(stderr, fmt, ap);
  } else {
    vfprintf(stdout, fmt, ap);
  }
}