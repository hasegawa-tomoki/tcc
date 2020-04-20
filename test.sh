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

assert 0 "0"
assert 42 42
assert 21 "5+20-4"
assert 41 " 12 + 34 - 5 "
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 4 '(3+5)/2'
assert 10 '-10+20'
assert 10 '- -10'
assert 10 '- - +10'
assert 1 '1==1'
assert 0 '2==1'
assert 1 '1!=2'
assert 0 '1!=1'
assert 1 '1<=2'
assert 1 '1<=1'
assert 0 '2<=1'
assert 1 '1<2'
assert 0 '2<1'
assert 1 '2>1'
assert 0 '1>2'
assert 2 '(1 > 2) + (2 > 1) + (1 == 1)'
assert 3 'a = 3'
assert 3 'ab = 3'
assert 255 'ab = 255'
assert 3 'a = b = 3'
assert 6 'a = b = 3;a + b;'
assert 3 'a = b = 3;return 3;'
assert 6 'a = b = 3;return a + b;'
assert 6 'a = b = 3;return a + b; 42'
assert 1 'if (0) return 1; 1'
assert 0 'if (0) return 1; else return 0;'
assert 1 'if (1) return 1; else return 0;'
assert 1 'a = 1;if (a) return 1; else return 0;'
assert 10 'a = 1;while (a < 10) a = a + 1;'
assert 10 'for (i = 0; i < 10; i = i + 1) return 10;'
assert 3 'for (i = 0; i < 10; i = i + 1){ a = 3; return a; }'

echo OK
