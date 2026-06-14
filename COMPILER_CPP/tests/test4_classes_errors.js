// Complex Test 4: Classes + Inheritance + `this` + Error Handling + Getters/Setters

class Shape {
  constructor(name) {
    this.name = name;
  }
  describe() {
    return `${this.name} has area ${this.area()}`;
  }
  area() {
    return 0;
  }
}

class Rectangle extends Shape {
  constructor(width, height) {
    super("Rectangle");
    this.width = width;
    this.height = height;
  }
  area() {
    return this.width * this.height;
  }
}

class Circle extends Shape {
  constructor(radius) {
    super("Circle");
    this.radius = radius;
  }
  area() {
    return Math.PI * this.radius * this.radius;
  }
  get diameter() {
    return this.radius * 2;
  }
  set diameter(d) {
    this.radius = d / 2;
  }
}

const shapes = [new Rectangle(4, 5), new Circle(3)];
for (const shape of shapes) {
  console.log(shape.describe());
}

const c = new Circle(5);
console.log(`diameter before: ${c.diameter}`);
c.diameter = 20;
console.log(`radius after setting diameter to 20: ${c.radius}`);

// instanceof checks
console.log(shapes[0] instanceof Rectangle); // true
console.log(shapes[0] instanceof Shape);     // true
console.log(shapes[1] instanceof Rectangle); // false

// Static methods
class MathUtils {
  static square(x) {
    return x * x;
  }
  static cube(x) {
    return x * x * x;
  }
}
console.log(MathUtils.square(5), MathUtils.cube(3)); // 25 27

// Error handling with custom error classes
class ValidationError extends Error {
  constructor(message, field) {
    super(message);
    this.name = "ValidationError";
    this.field = field;
  }
}

function validateAge(age) {
  if (typeof age !== "number") {
    throw new ValidationError("Age must be a number", "age");
  }
  if (age < 0 || age > 150) {
    throw new ValidationError("Age out of range", "age");
  }
  return age;
}

const testInputs = [25, -5, "abc", 200];
for (const input of testInputs) {
  try {
    const result = validateAge(input);
    console.log(`Valid age: ${result}`);
  } catch (e) {
    console.log(`Error (${e.name}): ${e.message} [field: ${e.field}]`);
  } finally {
    console.log("-- checked one input --");
  }
}

// Nested try/catch with rethrow
function outer() {
  try {
    try {
      throw new Error("inner failure");
    } catch (innerErr) {
      throw new Error(`wrapped: ${innerErr.message}`);
    }
  } catch (outerErr) {
    return outerErr.message;
  }
}
console.log(outer());
