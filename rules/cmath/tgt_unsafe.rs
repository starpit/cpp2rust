// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

unsafe fn f1(a0: i32) -> i32 {
    a0.abs()
}
unsafe fn f2(a0: i32) -> i32 {
    a0.abs()
}
unsafe fn f3(a0: i32) -> i32 {
    a0.abs()
}
unsafe fn f4(a0: f64) -> f64 {
    a0.log2()
}
unsafe fn f5(a0: i32) -> f64 {
    (a0 as f64).log2()
}
unsafe fn f6(a0: i64) -> f64 {
    let __x = a0;
    (__x as f64).ceil()
}
unsafe fn f7(a0: i32) -> f64 {
    let __x = a0;
    (__x as f64).ceil()
}
unsafe fn f8(a0: i64) -> f64 {
    let __x = a0;
    (__x as f64).floor()
}
unsafe fn f9(a0: i32) -> f64 {
    let __x = a0;
    (__x as f64).floor()
}
unsafe fn f12(a0: i32) -> f64 {
    let __x = a0;
    (__x as f64).sqrt()
}
unsafe fn f10(a0: i32, a1: i32) -> f64 {
    let __x = a0;
    let __y = a1;
    (__x as f64).powf(__y as f64)
}
unsafe fn f11(a0: f64, a1: i32) -> f64 {
    let __x: f64 = a0;
    let __y = a1;
    __x.powf(__y as f64)
}
unsafe fn f13(a0: f64, a1: i32) -> f64 {
    let __x: f64 = a0;
    let __y = a1;
    __x % (__y as f64)
}
unsafe fn f14(a0: f32) -> f32 {
    let __x: f32 = a0;
    __x.ceil()
}
unsafe fn f15(a0: i64) -> f64 {
    let __x = a0;
    (__x as f64).ceil()
}
unsafe fn f16(a0: u32) -> f64 {
    let __x = a0;
    (__x as f64).ceil()
}
unsafe fn f17(a0: u64) -> f64 {
    let __x = a0;
    (__x as f64).ceil()
}
unsafe fn f18(a0: f32) -> f32 {
    let __x: f32 = a0;
    __x.floor()
}
unsafe fn f19(a0: i64) -> f64 {
    let __x = a0;
    (__x as f64).floor()
}
unsafe fn f20(a0: u64) -> f64 {
    let __x = a0;
    (__x as f64).floor()
}
unsafe fn f21(a0: i64, a1: i64) -> f64 {
    let __x = a0;
    let __y = a1;
    (__x as f64).powf(__y as f64)
}
unsafe fn f22(a0: i64) -> f64 {
    let __x = a0;
    (__x as f64).sqrt()
}
unsafe fn f23(a0: f32) -> f32 {
    let __x: f32 = a0;
    __x.sqrt()
}
unsafe fn f24(a0: i64, a1: i64) -> f64 {
    let __x = a0;
    let __y = a1;
    (__x as f64) % (__y as f64)
}
