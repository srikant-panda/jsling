// Complex Test 3: Control Flow + Type Coercion + Math/Number Edge Cases

// Nested loops with labeled-like break via flag, switch inside loop
function classify(n) {
  switch (true) {
    case n % 15 === 0:
      return "FizzBuzz";
    case n % 3 === 0:
      return "Fizz";
    case n % 5 === 0:
      return "Buzz";
    default:
      return String(n);
  }
}
let output = [];
for (let i = 1; i <= 20; i++) {
  output.push(classify(i));
}
console.log(output.join(" "));

// do-while with break/continue
let i = 0;
let sum = 0;
do {
  i++;
  if (i % 2 === 0) continue;
  if (i > 15) break;
  sum += i;
} while (i < 100);
console.log(`sum of odd numbers up to 15 = ${sum}`); // 1+3+5+7+9+11+13+15=64

// Type coercion gauntlet
console.log("5" + 3);        // "53"
console.log("5" - 3);        // 2
console.log("5" * "2");      // 10
console.log(1 + true);       // 2
console.log("3" == 3);       // true
console.log("3" === 3);      // false
console.log(null == undefined); // true
console.log(null === undefined); // false
console.log([] + []);        // ""
console.log([] + {});        // "[object Object]"
console.log(0 == "0");       // true
console.log(0 == "");        // true
console.log("" == "0");      // false
console.log(!!"");           // false
console.log(!!"0");          // true
console.log(!!0);            // false
console.log(!!NaN);          // false

// Math + Number edge cases
console.log(Math.max(1, 2, 3, -5)); // 3
console.log(Math.min(1, 2, 3, -5)); // -5
console.log(Math.floor(-4.5));      // -5
console.log(Math.ceil(-4.5));       // -4
console.log(Math.round(2.5));       // 3
console.log(Math.round(-2.5));      // -2
console.log(Math.abs(-7));          // 7
console.log((10 / 3).toFixed(2));   // "3.33"
console.log((255).toString(16));    // "ff"
console.log((8).toString(2));       // "1000"
console.log(Number("42"));          // 42
console.log(Number("  42  "));      // 42
console.log(Number("abc"));         // NaN
console.log(parseInt("42px"));      // 42
console.log(parseFloat("3.14abc")); // 3.14
console.log(typeof NaN);            // "number"
console.log(Number.isNaN(NaN));     // true
console.log(Number.isInteger(5));   // true
console.log(Number.isInteger(5.5)); // false

// Exponent operator and operator precedence
console.log(2 ** 3 ** 2);   // 512 (right-assoc: 2**(3**2))
console.log((2 + 3) * 4 - 6 / 2); // 17
console.log(10 % 3 + 1);    // 2
