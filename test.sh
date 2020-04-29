#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./tcc <(echo "$input") > tmp.s
  gcc -static -o /tmp/tcctest test/func_noargs.o test/func_withargs.o test/func_alloc4.o tmp.s
  chmod 755 /tmp/tcctest
  /tmp/tcctest
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "int main(){ return 0; }"
assert 42 "int main(){ return 42; }"
assert 25 "int main(){ return 5+20; }"
assert 15 "int main(){ return 20-5; }"
assert 29 "int main(){ return 5+20+4; }"
assert 21 "int main(){ return 5+20-4; }"
assert 41 "int main(){ return  12 + 34 - 5 ; }"
assert 47 "int main(){ return 5+6*7; }"
assert 15 "int main(){ return 5*(9-6); }"
assert 4 "int main(){ return (3+5)/2; }"
assert 10 "int main(){ return -10+20; }"
assert 10 "int main(){ return - -10; }"
assert 10 "int main(){ return - - +10; }"
assert 1 "int main(){ return 1==1; }"
assert 0 "int main(){ return 2==1; }"
assert 1 "int main(){ return 1!=2; }"
assert 0 "int main(){ return 1!=1; }"
assert 1 "int main(){ return 1<=2; }"
assert 1 "int main(){ return 1<=1; }"
assert 0 "int main(){ return 2<=1; }"
assert 1 "int main(){ return 1<2; }"
assert 0 "int main(){ return 2<1; }"
assert 1 "int main(){ return 2>1; }"
assert 0 "int main(){ return 1>2; }"
assert 2 "int main(){ return (1 > 2) + (2 > 1) + (1 == 1); }"
assert 3 "int main(){ int a; a = 3; return a;}"
assert 3 "int main(){ int a; return a = 3; }"
assert 3 "int main(){ int ab; return ab = 3; }"
assert 255 "int main(){ int ab; return ab = 255; }"
assert 3 "int main(){ int a; int b; return a = b = 3; }"
assert 3 "int main(){ int a; int b; a = b = 3; return 3; }"
assert 6 "int main(){ int a; int b; a = b = 3; return a + b; }"
assert 6 "int main(){ int a; int b; a = b = 3; return a + b; 42; }"
assert 1 "int main(){ if (0) return 2; return 1; }"
assert 0 "int main(){ if (0) return 1; else return 0; }"
assert 1 "int main(){ if (1) return 1; else return 0; }"
assert 1 "int main(){ int a; a = 1;if (a) return 1; else return 0; }"
assert 10 "int main(){ int a; a = 1; while (a < 10) a = a + 1; return a; }"
assert 10 "int main(){ int i; for (i = 0; i < 10; i = i + 1) return 10; }"
assert 3 "int main(){ int i; int a; for (i = 0; i < 10; i = i + 1){ a = 3; return a; } }"
assert 10 "int main(){ return test(); } int test(){ return 10; }"
assert 10 "int test(){ return 10; } int main(){ return test(); }"
assert 8 "int main(){ int a; int b; a = 10; b = 20; return &b - &a; }"
assert 2 "int main(){ return test(2); } int test(int a){ return a; }"
assert 5 "int main(){ return test(2, 3); } int test(int a, int b){ return a + b; }"
assert 1 "int main(){ int *a; return 1; }"
assert 1 "int main(){ int **a; return 1; }"
assert 3 "int main(){ int x; int *y; y = &x; *y = 3; return x; }"
# assert 1 "int main(){ int *p; p = func_alloc4(1, 2, 3, 4); return *p; }"
# assert 4 "int main(){ int *p; p = func_alloc4(1, 2, 3, 4); int *q; q = p + 2; return *q; }"
assert 8 "int main(){ int p; return sizeof(p); }"
assert 8 "int main(){ int p; return sizeof p; }"
assert 8 "int main(){ int *p; return sizeof(p); }"
assert 8 "int main(){ int *p; return sizeof p; }"
assert 0 "int main(){ int a[20]; return 0; }"
assert 1 "int main(){ int a[2]; *a = 1; return *a; }"
assert 2 "int main(){ int a[2]; *a = 1; *(a + 1) = 2; return *(a + 1); }"
assert 1 "int main(){ int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p; }"
#assert 1 "int main(){ int a[2]; *a = 1; a = a + 1; a = a - 1; return *a; }"
assert 3 "int main(){ int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }"
assert 3 "int main() { int x[2]; int *y; y=&x; *y=3; return *x; }"
assert 3 "int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *x; }"
assert 4 "int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+1); }"
assert 5 "int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+2); }"
assert 5 "int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return x[2]; }"
assert 9 "int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return x[1] + x[2]; }"
assert 12 "int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return x[0] + x[1] + x[2]; }"
assert 7 "int main() { int x[3][3]; x[0][0]=3; x[1][1]=4; return x[0][0] + x[1][1]; }"
assert 1 'int main() { int x[2][3]; int *y; y=x; *(y+1)=1; return *(*x+1); }'
assert 3 'int x; int main() { x=3; return x; }'
assert 128 'int ga; int main(){ ga = 100; test(); return ga; } int test(){ ga = ga + 28; }'
assert 97 "int main(){ char a; a = 97; return a; }"
assert 97 "int main(){ char a; a = 'a'; return a; }"
assert 65 'int main(){ char *a; a = "ABC"; return a[0]; }'
assert 65 'int main(){ char *a; a = "Answer to the Ultimate Question of Life, the Universe, and Everything"; return a[0]; }'
assert 110 'int main(){ char *a; a = "Answer to the Ultimate Question of Life, the Universe, and Everything"; return a[1]; }'
assert 110 'int main(){ /* block comment */ char *a; a = "Answer to the Ultimate Question of Life, the Universe, and Everything"; return a[1]; }'
assert 6 'int main(){ return sizeof("abcde"); }'
assert 10 'int main(){ int a; a = 10; { int a; a = 20; } return a; }'

echo OK
