#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'

#this stuff should work

echo hello world > hello.tmp
cat hello.tmp | grep hello && cat hello.tmp
echo hellogoodbye | grep hello && echo hello && echo hi || echo goodbye
( echo this is a subshell; echo it uses semicolons; (echo this is a nested subshell && echo it does complex commands) )
( echo one; 
  echo two && (
  echo three && 
  echo four) )

(cat hello.tmp; echo hello world) | grep hello

echo hello again

(ls .. | (grep test | ( ls .. && cat | sort) ) ) && (echo complicated things && (ls .. | sort) )

echo seven

(echo one | exec sort; echo two) |
(ls .. | exec sort | exec cat &&
exec ls .. | exec grep st > a.tmp; echo four)

EOF

chmod 777 test.sh
./test.sh >test.exp
../timetrash test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"
