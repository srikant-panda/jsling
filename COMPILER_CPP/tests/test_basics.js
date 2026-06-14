// test_basics.js — variables, types, operators, coercion

// Variable declarations
let a = 10;
const b = 20;
var c = 30;
console.log(a);
console.log(b);
console.log(c);

// Reassignment
a = 100;
console.log(a);

// Arithmetic operators
console.log(10 + 3);
console.log(10 - 3);
console.log(10 * 3);
console.log(10 / 3);
console.log(10 % 3);
console.log(2 ** 10);

// Comparison operators
console.log(5 > 3);
console.log(5 < 3);
console.log(5 >= 5);
console.log(5 <= 4);
console.log(5 === 5);
console.log(5 !== 5);

// Logical operators
console.log(true && true);
console.log(true && false);
console.log(false || true);
console.log(false || false);
console.log(!true);
console.log(!false);

// String concatenation
console.log("hello" + " " + "world");

// String + number coercion
console.log("5" + 3);
console.log("5" - 3);
console.log("5" * 3);
console.log("5" / 3);

// Boolean coercion
console.log(true + 1);
console.log(false + 1);

// Equality
console.log(null == undefined);
console.log(null === undefined);
console.log(0 == "");
console.log(0 === "");
console.log(1 == true);
console.log(1 === true);

// typeof
console.log(typeof 42);
console.log(typeof "hello");
console.log(typeof true);
console.log(typeof null);
console.log(typeof undefined);
console.log(typeof {});
console.log(typeof []);
console.log(typeof function(){});

// Literals
console.log(null);
console.log(undefined);
console.log(NaN);
console.log(Infinity);
console.log(-Infinity);

// Ternary
console.log(true ? "yes" : "no");
console.log(false ? "yes" : "no");
console.log(5 > 3 ? "bigger" : "smaller");

// Comma and assignment
let x = 0;
x += 5;
console.log(x);
x -= 2;
console.log(x);
x *= 3;
console.log(x);
x /= 3;
console.log(x);
x %= 2;
console.log(x);
