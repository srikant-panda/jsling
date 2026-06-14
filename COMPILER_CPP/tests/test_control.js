// test_control.js — if/else, for, while, do-while, switch, break, continue

// if/else
let x = 10;
if (x > 5) {
    console.log("big");
} else {
    console.log("small");
}

// if/else if/else
if (x > 100) {
    console.log("huge");
} else if (x > 10) {
    console.log("medium");
} else {
    console.log("small");
}

// Nested if
if (x > 0) {
    if (x > 5) {
        console.log("positive and big");
    } else {
        console.log("positive and small");
    }
}

// for loop
let sum = 0;
for (let i = 1; i <= 10; i++) {
    sum = sum + i;
}
console.log(sum);

// for loop with break
for (let i = 0; i < 100; i++) {
    if (i === 5) break;
    console.log(i);
}

// for loop with continue
for (let i = 0; i < 10; i++) {
    if (i % 2 === 0) continue;
    console.log(i);
}

// Nested for with labels
for (let i = 0; i < 3; i++) {
    for (let j = 0; j < 3; j++) {
        if (j === 1) continue;
        if (i === 2 && j === 2) break;
    }
    console.log("row " + i);
}

// while loop
let n = 0;
while (n < 5) {
    n = n + 1;
}
console.log("while: " + n);

// while with break
let m = 0;
while (true) {
    m = m + 1;
    if (m === 3) break;
}
console.log("while break: " + m);

// do-while loop
let d = 0;
do {
    d = d + 1;
} while (d < 5);
console.log("do-while: " + d);

// do-while runs at least once
let count = 10;
do {
    console.log("do ran: " + count);
    count = count + 1;
} while (count < 10);

// switch
let day = 3;
switch (day) {
    case 1: console.log("Monday"); break;
    case 2: console.log("Tuesday"); break;
    case 3: console.log("Wednesday"); break;
    case 4: console.log("Thursday"); break;
    case 5: console.log("Friday"); break;
    default: console.log("Weekend");
}

// switch fall-through (with break)
let grade = "B";
switch (grade) {
    case "A": console.log("Excellent"); break;
    case "B": console.log("Good"); break;
    case "C": console.log("Average"); break;
    default: console.log("Fail");
}

// switch with default
let z = 99;
switch (z) {
    case 1: console.log("one"); break;
    case 2: console.log("two"); break;
    default: console.log("other");
}

// FizzBuzz (1-15)
for (let i = 1; i <= 15; i++) {
    if (i % 15 === 0) {
        console.log("FizzBuzz");
    } else if (i % 3 === 0) {
        console.log("Fizz");
    } else if (i % 5 === 0) {
        console.log("Buzz");
    } else {
        console.log(i);
    }
}
