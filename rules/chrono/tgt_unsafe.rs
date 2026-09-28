// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.
//
// steady_clock time_point -> Instant (monotonic); system_clock -> SystemTime
// (wall-clock).  Two keys on purpose: they are not interchangeable.

use std::time::Instant;
use std::time::SystemTime;

fn t1() -> Instant {
    Instant::now()
}

fn t2() -> SystemTime {
    SystemTime::now()
}

fn t3() -> SystemTime {
    SystemTime::now()
}
