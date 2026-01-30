// RUN: %clang_cc1 -verify -std=c2y %s
// RUN: %clang_cc1 -verify=pre-c2y  %s
// expected-no-diagnostics

/* WG14 N38X3: Yes
 * Expression Evaluation and Access in _Generic
 *
 */

static int association_with_id(void) {
  return _Generic(5,
      int a: a,   // pre-c2y-error {{expected ':'}}
      double: 0xBAD,
      default: 0xBAD);
}

static int default_with_id(void) {
  return _Generic(5,
      int: 5, 
      double: 0xBAD,
      default d: 0xBAD); // pre-c2y-error {{expected ':'}}
}