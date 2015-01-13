#!/usr/bin/env node

var suite;
var assert;

if(typeof process !== 'undefined')
{
    console.log('running from nodejs');
    console.log('path ', process.env.EINA_SUITE_PATH);
    console.log("teste1");

    suite = require(process.env.EINA_SUITE_PATH);
    assert = require('assert');
}
else
{
    assert = function(test, message) { if (test !== true) throw message; };
    console.log('running from libv8')
}

// container tests

console.log("teste");
var l1 = suite.raw_list;
console.log ("l1 ", l1.toString());
assert (l1.length == 3);
var l2 = suite.raw_list;
console.log ("l2 ", l2.toString());
assert (l2.length == 3);
var c = l1.concat(l2);
console.log ("c ", c.toString());
assert (c.length == (l1.length + l2.length));
assert (c[0] == l1[0]);
assert (c[1] == l1[1]);
assert (c[2] == l1[2]);
assert (c[3] == l2[0]);
assert (c[4] == l2[1]);
assert (c[5] == l2[2]);
assert (c.indexOf(c[0]) == 0);
assert (c.indexOf(c[1]) == 1);
assert (c.indexOf(c[2]) == 2);
assert (c.indexOf(c[3]) == 0);
assert (c.indexOf(c[4]) == 1);
assert (c.indexOf(c[5]) == 2);
assert (c.lastIndexOf(c[0]) == 3);
assert (c.lastIndexOf(c[1]) == 4);
assert (c.lastIndexOf(c[2]) == 5);
assert (c.lastIndexOf(c[3]) == 3);
assert (c.lastIndexOf(c[4]) == 4);
assert (c.lastIndexOf(c[5]) == 5);
var s1 = l1.slice(1, 3);
console.log ("s1 ", s1.toString());
assert (s1.length == 2);
assert (s1[0] == l1[1]);
assert (s1[1] == l1[2]);
var s2 = c.slice(1, 3);
console.log ("s2 ", s2.toString());
assert (s2.length == 2);
assert (s2[0] == l1[1]);
assert (s2[1] == l1[2]);

// error tests

var captured = false;
try {
  suite.clear_eina_error();
} catch(e) {
  console.log("Exception ", e.toString());
  captured = true;
}
assert(captured === false, 'error #1');

captured = false;
try {
  suite.set_eina_error();
} catch(e) {
  console.log("Exception ", e.toString());
  assert(e.code === 'Eina_Error', 'error #2');
  assert(e.value === 'foobar', 'error #3');
  captured = true;
}
assert(captured === true, 'error #4');

// accessor tests

assert(suite.acc.get(0) === 42, 'accessor #1');
assert(suite.acc.get(1) === 24, 'accessor #2');

// finished tests

console.log ("Test execution with success");

