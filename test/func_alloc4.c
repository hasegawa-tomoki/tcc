#include<stdio.h>
#include<stdlib.h>
int *func_alloc4(int a, int b, int c, int d){
  int *p = calloc(4, sizeof(int));
  p[0] = a;
  p[1] = b;
  p[2] = c;
  p[3] = d;

  return p;
}
/*
void main(){
  int *p;
  p = func_alloc4(1, 2, 4, 8);
  int *q;
  q = p + 2;
  printf("ret: %d\n", *q);
  q = p + 3;
  printf("ret: %d\n", *q);
}
*/
/*
void main(){
  int *p = func_alloc4(1, 2, 3, 4);
  p++;
  printf("ret: %d\n", *p);
}
*/

