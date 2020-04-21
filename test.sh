#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./tcc "$input" > tmp.s
  cc -o tmp test/func_noargs.o test/func_withargs.o tmp.s
  chmod 755 tmp
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "main(){ return 0; }"
assert 42 "main(){ return 42; }"
assert 21 "main(){ return 5+20-4; }"
assert 41 "main(){ return  12 + 34 - 5 ; }"
assert 47 "main(){ return 5+6*7; }"
assert 15 "main(){ return 5*(9-6); }"
assert 4 "main(){ return (3+5)/2; }"
assert 10 "main(){ return -10+20; }"
assert 10 "main(){ return - -10; }"
assert 10 "main(){ return - - +10; }"
assert 1 "main(){ return 1==1; }"
assert 0 "main(){ return 2==1; }"
assert 1 "main(){ return 1!=2; }"
assert 0 "main(){ return 1!=1; }"
assert 1 "main(){ return 1<=2; }"
assert 1 "main(){ return 1<=1; }"
assert 0 "main(){ return 2<=1; }"
assert 1 "main(){ return 1<2; }"
assert 0 "main(){ return 2<1; }"
assert 1 "main(){ return 2>1; }"
assert 0 "main(){ return 1>2; }"
assert 2 "main(){ return (1 > 2) + (2 > 1) + (1 == 1); }"
assert 3 "main(){ return a = 3; }"
assert 3 "main(){ return ab = 3; }"
assert 255 "main(){ return ab = 255; }"
assert 3 "main(){ return a = b = 3; }"
assert 6 "main(){ a = b = 3; return a + b; }"
assert 3 "main(){ a = b = 3; return 3; }"
assert 6 "main(){ a = b = 3; return a + b; }"
assert 6 "main(){ a = b = 3; return a + b; 42; }"
assert 1 "main(){ if (0) return 2; return 1; }"
assert 0 "main(){ if (0) return 1; else return 0; }"
assert 1 "main(){ if (1) return 1; else return 0; }"
assert 1 'main(){ a = 1;if (a) return 1; else return 0; }'
assert 10 "main(){ a = 1; while (a < 10) a = a + 1; return a; }"
assert 10 "main(){ for (i = 0; i < 10; i = i + 1) return 10; }"
assert 3 "main(){ for (i = 0; i < 10; i = i + 1){ a = 3; return a; } }"
assert 10 "main(){ return test(); } test(){ return 10; }"
assert 10 "test(){ return 10; } main(){ return test(); }"

echo OK
