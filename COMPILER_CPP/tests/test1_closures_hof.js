// Complex Test 1: Closures + Higher-Order Functions + Recursion + Method Chaining

function makeCounter(start) {
  let count = start;
  return {
    increment: function (step = 1) {
      count += step;
      return count;
    },
    decrement: function (step = 1) {
      count -= step;
      return count;
    },
    value: function () {
      return count;
    },
  };
}

const counter = makeCounter(10);
console.log(counter.increment());      // 11
console.log(counter.increment(5));     // 16
console.log(counter.decrement(2));     // 14
console.log(counter.value());          // 14

// Recursive factorial via named function expression
const factorial = function fact(n) {
  return n <= 1 ? 1 : n * fact(n - 1);
};
console.log(factorial(6)); // 720

// Memoized fibonacci using closure cache
function makeMemoFib() {
  const cache = {};
  function fib(n) {
    if (n in cache) return cache[n];
    const result = n <= 1 ? n : fib(n - 1) + fib(n - 2);
    cache[n] = result;
    return result;
  }
  return fib;
}
const fib = makeMemoFib();
console.log(fib(10)); // 55
console.log(fib(15)); // 610

// Chained array method pipeline
const nums = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
const result = nums
  .filter((n) => n % 2 === 0)
  .map((n) => n * n)
  .reduce((acc, n) => acc + n, 0);
console.log(result); // 220 (4+16+36+64+100)

// Array of functions (callbacks) applied in sequence
const pipeline = [
  (x) => x + 1,
  (x) => x * 2,
  (x) => x - 3,
];
const piped = pipeline.reduce((val, fn) => fn(val), 5);
console.log(piped); // ((5+1)*2)-3 = 9

// Function returning function returning function (currying)
const add = (a) => (b) => (c) => a + b + c;
console.log(add(1)(2)(3)); // 6
