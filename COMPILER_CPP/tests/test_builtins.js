// test_builtins.js — Math, Object, conversion functions, Date

// Math basics
console.log(Math.PI);
console.log(Math.E);
console.log(Math.floor(3.7));
console.log(Math.ceil(3.2));
console.log(Math.round(3.5));
console.log(Math.round(3.4));
console.log(Math.trunc(3.9));
console.log(Math.abs(-5));
console.log(Math.abs(5));

// Math min/max
console.log(Math.max(1, 5, 3));
console.log(Math.min(1, 5, 3));
console.log(Math.max(-1, -5, -3));

// Math power/root
console.log(Math.pow(2, 10));
console.log(Math.sqrt(9));
console.log(Math.sqrt(2));

// Math sign
console.log(Math.sign(-42));
console.log(Math.sign(0));
console.log(Math.sign(42));

// Math log
console.log(Math.log(Math.E));
console.log(Math.log2(8));
console.log(Math.log10(100));
console.log(Math.log10(1000));

// Math hypot
console.log(Math.hypot(3, 4));

// parseInt / parseFloat
console.log(parseInt("42"));
console.log(parseInt("3.14"));
console.log(parseInt("ff", 16));
console.log(parseInt("1010", 2));
console.log(parseFloat("3.14"));
console.log(parseFloat("hello"));

// isNaN / isFinite
console.log(isNaN(NaN));
console.log(isNaN(42));
console.log(isNaN("hello"));
console.log(isFinite(42));
console.log(isFinite(Infinity));
console.log(isFinite(NaN));

// Number / String / Boolean
console.log(Number("42"));
console.log(Number("3.14"));
console.log(Number(true));
console.log(Number(false));
console.log(String(42));
console.log(String(3.14));
console.log(String(true));
console.log(Boolean(0));
console.log(Boolean(1));
console.log(Boolean(""));
console.log(Boolean("hello"));
console.log(Boolean(null));
console.log(Boolean(undefined));

// Object.keys / values / entries
let obj = { a: 1, b: 2, c: 3 };
console.log(Object.keys(obj));
console.log(Object.values(obj));
console.log(Object.entries(obj));

// Object with string values
let person = { name: "Alice", age: 30 };
console.log(Object.keys(person));
console.log(Object.values(person));

// Number methods
let n = 3.14159;
console.log(n.toFixed(2));
console.log(n.toFixed(0));
console.log((42).toString());
console.log((255).toString(16));
console.log((10).toString(2));

// Date basic (use fixed patterns)
let now = Date.now();
console.log(typeof now);
console.log(now > 0);
