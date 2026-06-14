// test_strings.js — all string methods

let s = "Hello, World!";
console.log(s.length);
console.log(s.toUpperCase());
console.log(s.toLowerCase());
console.log(s.indexOf("World"));
console.log(s.indexOf("xyz"));
console.log(s.includes("World"));
console.log(s.includes("xyz"));
console.log(s.startsWith("Hello"));
console.log(s.startsWith("World"));
console.log(s.endsWith("!"));
console.log(s.endsWith("Hello"));

// slice
console.log(s.slice(7));
console.log(s.slice(0, 5));
console.log(s.slice(-6));

// trim
let padded = "  hello  ";
console.log(padded.trim());

// replace / replaceAll
console.log(s.replace("World", "jsling"));
console.log("aaa".replaceAll("a", "b"));

// split
console.log("one,two,three".split(","));
console.log("hello world".split(" "));

// charAt
console.log(s.charAt(0));
console.log(s.charAt(7));

// repeat
console.log("abc".repeat(3));

// padStart / padEnd
console.log("5".padStart(3, "0"));
console.log("5".padEnd(3, "0"));

// substring
console.log(s.substring(7, 12));

// String concatenation
console.log("foo" + "bar");
console.log("num: " + 42);

// String indexing
console.log("hello"[0]);
console.log("hello"[4]);

// Empty string
console.log("".length);
console.log("".indexOf("a"));

// concat method
console.log("hello".concat(" ", "world"));
