// test_float.js — IEEE 754 floating-point behavior (matches Node.js)

// Classic 0.1 + 0.2
console.log(0.1 + 0.2);
console.log(0.1 + 0.2 === 0.3);

// Other floating-point surprises
console.log(0.1 + 0.7);
console.log(0.2 + 0.4);
console.log(0.3 - 0.1);

// Precision preservation
console.log(3.14);
console.log(1 / 3);
console.log(2 / 3);
console.log(Math.sqrt(2));

// NaN behavior
console.log(NaN === NaN);
console.log(NaN !== NaN);
console.log(NaN + 1);
console.log(NaN * 0);

// Infinity
console.log(1 / 0);
console.log(-1 / 0);
console.log(Infinity + 1);
console.log(Infinity - Infinity);
console.log(Infinity * 0);
console.log(0 / 0);

// Large numbers
console.log(999999999999999);

// Small numbers
console.log(0.0000001);

// Integer boundary
console.log(9007199254740991);
