// test_functions.js — declarations, closures, recursion, higher-order functions

// Function declaration
function add(a, b) {
    return a + b;
}
console.log(add(3, 4));

// Function with default behavior
function greet(name) {
    return "Hello, " + name + "!";
}
console.log(greet("World"));

// Recursive fibonacci
function fib(n) {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}
console.log(fib(10));

// Recursive factorial
function factorial(n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
console.log(factorial(5));

// Closure: counter
function makeCounter() {
    let count = 0;
    return function() {
        count = count + 1;
        return count;
    };
}
let counter = makeCounter();
console.log(counter());
console.log(counter());
console.log(counter());

// Closure: adder
function makeAdder(x) {
    return function(y) {
        return x + y;
    };
}
let add5 = makeAdder(5);
let add10 = makeAdder(10);
console.log(add5(3));
console.log(add10(3));

// Nested closures
function outer() {
    let a = 1;
    function middle() {
        let b = 2;
        function inner() {
            let c = 3;
            return a + b + c;
        }
        return inner();
    }
    return middle();
}
console.log(outer());

// Function expression
let square = function(n) { return n * n; };
console.log(square(7));

// Immediately invoked function expression (IIFE)
let result = (function() { return 42; })();
console.log(result);

// Higher-order: passing functions
function apply(fn, x) {
    return fn(x);
}
console.log(apply(function(n) { return n * 2; }, 5));

// Function returning function
function compose(f, g) {
    return function(x) {
        return f(g(x));
    };
}
let doubleSquare = compose(function(n) { return n * 2; }, function(n) { return n * n; });
console.log(doubleSquare(3));

// Multiple return paths
function abs(n) {
    if (n < 0) return -n;
    return n;
}
console.log(abs(-5));
console.log(abs(5));

// Early return
function findFirst(arr, target) {
    for (let i = 0; i < arr.length; i++) {
        if (arr[i] === target) return i;
    }
    return -1;
}
console.log(findFirst([10, 20, 30, 40], 30));
console.log(findFirst([10, 20, 30, 40], 99));

// Recursive: sum array
function sumArr(arr, idx) {
    if (idx >= arr.length) return 0;
    return arr[idx] + sumArr(arr, idx + 1);
}
console.log(sumArr([1, 2, 3, 4, 5], 0));
