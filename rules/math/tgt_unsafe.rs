// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

unsafe fn f1(a0: f64) -> f64 {
    a0.cos()
}
unsafe fn f2(a0: f64) -> f64 {
    a0.round()
}
unsafe fn f3(a0: f64) -> f64 {
    a0.sin()
}
// A placeholder is substituted as raw text, so an argument has to be bound
// before a method call or a cast: `a / b.ceil()` and `a / b as f64` both
// regroup around the wrong operand.
unsafe fn f4(a0: f64) -> f64 {
    let __x: f64 = a0;
    __x.ceil()
}
unsafe fn f5(a0: f64) -> f64 {
    let __x: f64 = a0;
    __x.floor()
}
unsafe fn f6(a0: f64, a1: f64) -> f64 {
    let __x: f64 = a0;
    let __y: f64 = a1;
    __x.powf(__y)
}
unsafe fn f7(a0: f64) -> f64 {
    let __x: f64 = a0;
    __x.sqrt()
}
unsafe fn f8(a0: f64, a1: f64) -> f64 {
    let __x: f64 = a0;
    let __y: f64 = a1;
    __x % __y
}
