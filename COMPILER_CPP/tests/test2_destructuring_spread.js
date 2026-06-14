// Complex Test 2: Destructuring + Spread/Rest + Template Literals + Object Manipulation

// Nested destructuring with defaults and rename
const config = {
  name: "ServerA",
  settings: { port: 8080, debug: false },
  tags: ["prod", "east"],
};
const {
  name,
  settings: { port, debug = true, timeout = 30 },
  tags: [primaryTag, ...otherTags],
} = config;
console.log(`Server: ${name}, Port: ${port}, Debug: ${debug}, Timeout: ${timeout}`);
console.log(`Primary tag: ${primaryTag}, Other tags: ${otherTags.join(", ")}`);

// Array destructuring with swap, holes, and rest
let [a, b] = [1, 2];
[a, b] = [b, a]; // swap
console.log(`a=${a}, b=${b}`); // a=2, b=1

const [first, , third, ...remaining] = [10, 20, 30, 40, 50];
console.log(`first=${first}, third=${third}, remaining=${remaining.join(",")}`);

// Object spread merging with overrides
const defaults = { theme: "dark", fontSize: 12, showSidebar: true };
const userPrefs = { fontSize: 16 };
const merged = { ...defaults, ...userPrefs, lastModified: "today" };
console.log(JSON_like(merged));

function JSON_like(obj) {
  const parts = Object.entries(obj).map(([k, v]) => `${k}: ${v}`);
  return `{ ${parts.join(", ")} }`;
}

// Rest parameters in functions with default + spread call
function summarize(title, ...scores) {
  const total = scores.reduce((s, x) => s + x, 0);
  const avg = total / scores.length;
  return `${title} -> total=${total}, avg=${avg.toFixed(2)}`;
}
console.log(summarize("Quiz", 80, 90, 100));
const scoreList = [70, 85, 95, 60];
console.log(summarize("Exam", ...scoreList));

// Template literal with nested expressions and method calls
const items = ["apple", "banana", "cherry"];
console.log(`Items (${items.length}): ${items.map((i) => i.toUpperCase()).join(", ")}`);

// Computed property names + shorthand methods in object literal
function makeShape(type, w, h) {
  return {
    type,
    width: w,
    height: h,
    [`${type}Area`]: w * h,
    area() {
      return this.width * this.height;
    },
  };
}
const rect = makeShape("rect", 4, 5);
console.log(`${rect.type}: area=${rect.area()}, computedKey=${rect.rectArea}`);
