// RUN: %clang_cc1 -verify=expected,c2y -std=c2y -fexperimental-mode %s
// RUN: %clang_cc1 -verify -std=c2y -fexperimental-mode -triple x86_64 -emit-llvm -o - %s | FileCheck %s

/* WG14 N38X3: Yes
 * Expression Evaluation and Access in _Generic
 *
 */

static void association_with_id(void) {
  constexpr int rr = 5;
  constexpr int r = _Generic(rr,
      constexpr int a: a,
      double: 0xBAD,
      default: 0xBAD);
  _Static_assert(r == 5);
}

static void default_with_id(void) {
  constexpr int r = _Generic(5,
      double: 0xDAB,
      default d: 0xBAD);
  static_assert(r == 0xBAD);
}


#define decay_generic(x) _Generic(x, constexpr int a: a, int: 1, double: 2, default: 0)

static void decay(void) {
  // Should not decay and will select const int (value a).
  constexpr int value = 3;
  constexpr int result = decay_generic(value);
  static_assert(result == 3);

  // Should decay and select int (value 1).
  int value1 = 0;
  constexpr int result1 = decay_generic(value1);
  static_assert(result1 == 1);

  // Should decay back to double and select double (value 2).
  constexpr double value2 = 3;
  constexpr double result2 = decay_generic(value2);
  static_assert(result2 == 2);

  // Should decay and select the default.
  constexpr char value3 = 0;
  constexpr char result3 = decay_generic(value3);
  static_assert(result3 == 0);
}

// Use the same identifier in all cases to
// test that the scoping is correct.
#define scope_generic(X) _Generic(X, constexpr int b: b * b, constexpr double b: b, default: 27.2)

static void limited_scope() {
  // Test the int path.
  constexpr int ax = 3;
  constexpr int a = scope_generic(ax);
  static_assert(a == 9);

  // Test the double path.
  constexpr double cx = 3.1;
  constexpr double c = scope_generic(cx);
  static_assert(c == 3.1);

  // Test the default path.
  constexpr double dx = scope_generic((float)(3.1));
  static_assert(dx == 27.2);
}

static int maybe_called(void) {
  return 1;
}

// CHECK-LABEL: define {{.*}} void @operand_evaluated()
static void operand_evaluated(void) {
// CHECK: %a = alloca i32, align 4
// CHECK-NEXT: %call = call i32 @maybe_called()
  _Generic(maybe_called(), 
      int a: a, // expected-warning {{expression result unused}}
      double: 0xBAD,
      default: 0xBAD);
}

// CHECK-LABEL: define {{.*}} void @operand_evaluated_once()
static void operand_evaluated_once(void) {
// CHECK: %a = alloca i32, align 4
// CHECK-NEXT: %call = call i32 @maybe_called()
  _Generic(maybe_called(), 
      int a: (a * a) / 2, // expected-warning {{expression result unused}}
      double: 0xBAD,
      default: 0xBAD);
}

// CHECK-LABEL: define {{.*}} void @operand_evaluated_unused()
static void operand_evaluated_unused(void) {
// CHECK: %a = alloca i32, align 4
// CHECK-NEXT: %call = call i32 @maybe_called()
  _Generic(maybe_called(), 
      int a: 2, // expected-warning {{expression result unused}}
      double: 0xBAD,
      default: 0xBAD);
}

// CHECK-LABEL: define {{.*}} void @operand_not_evaluated_no_name()
static void operand_not_evaluated_no_name(void) {
// CHECK: ret void
  _Generic(maybe_called(), 
      int: 2, // expected-warning {{expression result unused}}
      double: 0xBAD,
      default: 0xBAD);
}

// CHECK-LABEL: define {{.*}} void @operand_not_evaluated_no_match()
static void operand_not_evaluated_no_match(void) {
// CHECK: ret void
  _Generic(maybe_called(), 
      double d: 0xBAD, 
      default: 0xBAD); // expected-warning {{expression result unused}}
}

int main() {
  operand_evaluated();
  operand_evaluated_once();
  operand_evaluated_unused();
  operand_not_evaluated_no_name();
  operand_not_evaluated_no_match();
  return 1;
}