// Test 1: Template literals with expressions
let name = "World";
let greeting = `Hello, ${name}!`;
console.log(greeting); // Hello, World!

let a = 10, b = 20;
let sum = `The sum of ${a} and ${b} is ${a + b}`;
console.log(sum); // The sum of 10 and 20 is 30

// Nested expressions
console.log(`2 * 3 = ${2 * 3}`); // 2 * 3 = 6

let innerVal = a + 5;
console.log(`computed: ${innerVal}`); // computed: 15

// Test 2: Arrow functions
let double = x => x * 2;
console.log(double(5)); // 10

let add = (x, y) => x + y;
console.log(add(3, 4)); // 7

let greet = (name) => {
    let msg = `Hello, ${name}!`;
    return msg;
};
console.log(greet("Alice")); // Hello, Alice!

// No params
let getPi = () => 3.14159;
console.log(getPi()); // 3.14159

// Test 3: Rest parameters
function sumAll(...nums) {
    let total = 0;
    for (let i = 0; i < nums.length; i++) {
        total = total + nums[i];
    }
    return total;
}
console.log(sumAll(1, 2, 3, 4)); // 10

// With regular params + rest
function showFirst(first, ...rest) {
    console.log(`First: ${first}`);
    console.log(`Rest count: ${rest.length}`);
}
showFirst("a", "b", "c"); // First: a, Rest count: 2

// Test 4: Spread operator in calls
let numbers = [1, 2, 3];
console.log(Math.max(...numbers)); // 3

let moreNums = [4, 5, 6];
console.log(sumAll(...numbers, ...moreNums)); // 21

// Test 5: Spread in arrays
let arr1 = [1, 2, 3];
let arr2 = [0, ...arr1, 4];
console.log(arr2.join(",")); // 0,1,2,3,4

// Test 6: Spread in objects
let obj1 = { x: 1, y: 2 };
let obj2 = { ...obj1, z: 3 };
console.log(`x=${obj2.x}, y=${obj2.y}, z=${obj2.z}`); // x=1, y=2, z=3

// Test 7: Default parameters
function greetUser(name = "Guest", greeting = "Hello") {
    return `${greeting}, ${name}!`;
}
console.log(greetUser()); // Hello, Guest!
console.log(greetUser("Bob")); // Hello, Bob!
console.log(greetUser("Bob", "Hi")); // Hi, Bob!

// Test 8: Postfix increment/decrement
let counter = 0;
console.log(counter++); // 0 (postfix returns old value)
console.log(counter); // 1
console.log(++counter); // 2 (prefix returns new value)
console.log(counter--); // 2
console.log(counter); // 1

// Test 9: in operator
let person = { name: "John", age: 30 };
console.log("name" in person); // true
console.log("email" in person); // false

let arr = [10, 20, 30];
console.log(0 in arr); // true
console.log(5 in arr); // false

// Test 10: Combined features
let people = [
    { name: "Alice", age: 25 },
    { name: "Bob", age: 30 },
    { name: "Charlie", age: 35 }
];

let names = people.map(p => p.name);
console.log(names.join(", ")); // Alice, Bob, Charlie

let adults = people.filter(p => p.age >= 30);
console.log(`Adults: ${adults.length}`); // Adults: 2

let totalAge = people.reduce((sum, p) => sum + p.age, 0);
console.log(`Total age: ${totalAge}`); // Total age: 90

console.log("ALL TESTS PASSED!");
