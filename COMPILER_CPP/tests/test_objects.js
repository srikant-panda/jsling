// test_objects.js — object literals, property access, methods

// Basic object
let obj = { a: 1, b: 2, c: 3 };
console.log(obj.a);
console.log(obj.b);
console.log(obj.c);
console.log(obj.d);

// Bracket notation
console.log(obj["a"]);
let key = "b";
console.log(obj[key]);

// Property assignment
obj.d = 4;
console.log(obj.d);

// Nested objects
let nested = { outer: { inner: 42 } };
console.log(nested.outer.inner);

// Object with methods
let calc = {
    value: 0,
    add: function(n) { return this.value + n; },
    sub: function(n) { return this.value - n; }
};
console.log(calc.value);

// Object.keys/values/entries
let colors = { red: 1, green: 2, blue: 3 };
console.log(Object.keys(colors));
console.log(Object.values(colors));

// Object with arrays
let data = { items: [10, 20, 30] };
console.log(data.items.length);
console.log(data.items[1]);

// Object update
let user = { name: "Bob", age: 25 };
user.age = 26;
console.log(user.age);
user.email = "bob@example.com";
console.log(user.email);

// Empty object
let empty = {};
console.log(Object.keys(empty));
empty.x = 1;
console.log(Object.keys(empty));

// Object in array
let list = [{ id: 1 }, { id: 2 }, { id: 3 }];
console.log(list[0].id);
console.log(list[2].id);

// Object with boolean/null values
let flags = { active: true, deleted: false, data: null };
console.log(flags.active);
console.log(flags.deleted);
console.log(flags.data);
