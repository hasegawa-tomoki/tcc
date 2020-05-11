#include "tcc.h"

void error(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void verror_at(char *loc, char *fmt, va_list ap){
  // Calc start & end position including loc
  char *line = loc;
  while (user_input < line && line[-1] != '\n'){
    line--;
  }
  char *end = loc;
  while (*end != '\n'){
    end++;
  }
  // Calc number of the line
  int line_num = 1;
  for (char *p = user_input; p < line; p++){
    if (*p == '\n'){
      line_num++;
    }
  }

  int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  int pos = loc - line + indent;
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
  exit(1);
}

void error_token(Token *tok, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->str, fmt, ap);
  exit(1);
}

void warning_token(Token *tok, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->str, fmt, ap);
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

char *read_file(char *path) {
  // Open and read the file.
  FILE *fp = fopen(path, "r");
  if (!fp){
    error("cannot open %s: %s", path, strerror(errno));
  }

  int filemax = 10 * 1024 * 1024;
  char *buf = malloc(filemax);
  int size = fread(buf, 1, filemax - 2, fp);
  if (!feof(fp)){
    error("%s: file too large");
  }

  // Make sure that the string ends with "\n\0".
  if (size == 0 || buf[size - 1] != '\n'){
    buf[size++] = '\n';
  }
  buf[size] = '\0';

  return buf;
}
