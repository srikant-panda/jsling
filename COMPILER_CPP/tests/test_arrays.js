// test_arrays.js — all array methods

// Basic operations
let a = [1, 2, 3];
console.log(a.length);
console.log(a[0]);
console.log(a[2]);

// push / pop
a.push(4);
console.log(a);
console.log(a.pop());
console.log(a);

// shift / unshift
console.log(a.shift());
console.log(a);
a.unshift(0);
console.log(a);

// join
console.log([1, 2, 3].join(", "));
console.log(["a", "b", "c"].join("-"));

// includes / indexOf
console.log([1, 2, 3].includes(2));
console.log([1, 2, 3].includes(99));
console.log([10, 20, 30].indexOf(20));
console.log([10, 20, 30].indexOf(99));

// slice
console.log([1, 2, 3, 4, 5].slice(1, 3));
console.log([1, 2, 3, 4, 5].slice(2));

// reverse
let r = [1, 2, 3];
r.reverse();
console.log(r);

// sort
let s = [3, 1, 4, 1, 5];
s.sort();
console.log(s);

// concat
console.log([1, 2].concat([3, 4]));
console.log([1, 2].concat([3], [4, 5]));

// splice
let sp = [1, 2, 3, 4, 5];
console.log(sp.splice(1, 2));
console.log(sp);

// Higher-order methods
let nums = [1, 2, 3, 4, 5];

// map
console.log(nums.map(function(n) { return n * 2; }));

// filter
console.log(nums.filter(function(n) { return n % 2 === 0; }));

// reduce
console.log(nums.reduce(function(acc, n) { return acc + n; }, 0));

// find
console.log(nums.find(function(n) { return n > 3; }));

// some / every
console.log(nums.some(function(n) { return n > 4; }));
console.log(nums.every(function(n) { return n > 0; }));
console.log(nums.every(function(n) { return n > 3; }));

// forEach (side effects)
let total = 0;
nums.forEach(function(n) { total = total + n; });
console.log(total);

// Nested arrays
let grid = [[1, 2], [3, 4], [5, 6]];
console.log(grid[0]);
console.log(grid[1][1]);

// Array as arguments
function sum3(arr) {
    let s = 0;
    for (let i = 0; i < arr.length; i++) {
        s = s + arr[i];
    }
    return s;
}
console.log(sum3([10, 20, 30]));

// Array literal with mixed types
let mixed = [1, "hello", true, null, undefined];
console.log(mixed.length);
console.log(mixed[1]);

// Empty array
let empty = [];
console.log(empty.length);
empty.push(1);
console.log(empty.length);
