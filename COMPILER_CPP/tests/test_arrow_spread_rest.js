// test_arrow_spread_rest.js - arrow functions, spread elements, and rest parameters

const double = (n) => n * 2;
console.log(double(5));

const add = (a, b = 10) => a + b;
console.log(add(5));
console.log(add(5, 7));

function sum(label, ...nums) {
    return label + ": " + nums.reduce((total, n) => total + n, 0);
}
console.log(sum("total", 1, 2, 3, 4));

const base = [2, 3];
const expanded = [1, ...base, 4];
console.log(expanded.join(","));
console.log(Math.max(...expanded));

const defaults = { theme: "dark", size: 12 };
const overrides = { size: 16 };
const merged = { ...defaults, ...overrides, enabled: true };
console.log(merged.theme, merged.size, merged.enabled);

const pipeline = [
    (x) => x + 1,
    (x) => x * 2,
];
console.log(pipeline.reduce((value, fn) => fn(value), 5));
