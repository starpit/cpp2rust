// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

// mlir::registerPass -> evaluate the allocator argument, discard it.  src.cpp
// records why the registration itself is unobservable in the program the port
// emits, and why DROPPING `a0` would be a real defect rather than a smaller one.

// `let _ = a0;` and not a bare `a0;`: the body is inlined textually, so `a0`
// becomes the caller's expression -- a std::function construction whose value
// must be produced and then dropped.  `let _ =` evaluates and drops at the end of
// the statement; it binds nothing, so it holds no borrow (E0716) across it.
fn f1(a0: ::std::rc::Rc<dyn Fn() -> Option<Box<u64>>>) {
    let _ = a0;
}
