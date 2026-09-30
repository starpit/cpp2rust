// Copyright (c) 2022-present INESC-ID.
// Distributed under the MIT license that can be found in the LICENSE file.

use sprintf::parser::{ConversionType, FormatElement, parse_format_string};
use sprintf::{Printf, vsprintfp};

use crate::va_args::{VaArg, VaArgGet};

enum Arg {
    Int(i32),
    UInt(u32),
    Long(i64),
    ULong(u64),
    Size(usize),
    Char(char),
    Str(String),
    Double(f64),
}

impl Arg {
    fn as_printf(&self) -> &dyn Printf {
        match self {
            Arg::Int(v) => v,
            Arg::UInt(v) => v,
            Arg::Long(v) => v,
            Arg::ULong(v) => v,
            Arg::Size(v) => v,
            Arg::Char(v) => v,
            Arg::Str(v) => v,
            Arg::Double(v) => v,
        }
    }
}

pub fn format_c(fmt: &str, va: &[VaArg]) -> String {
    let elements = match parse_format_string(fmt) {
        Ok(elements) => elements,
        Err(e) => panic!("format_c: cannot parse {fmt:?}: {e:?}"),
    };
    let mut args: Vec<Arg> = Vec::with_capacity(va.len());
    let mut pos = 0;
    for element in &elements {
        if let FormatElement::Format(spec) = element {
            if spec.conversion_type == ConversionType::PercentSign {
                continue;
            }
            let arg = &va[pos];
            args.push(match spec.conversion_type {
                ConversionType::DecInt => match arg {
                    VaArg::Int(v) => Arg::Int(*v),
                    VaArg::UInt(v) => Arg::UInt(*v),
                    VaArg::Long(v) => Arg::Long(*v),
                    VaArg::ULong(v) => Arg::ULong(*v),
                    _ => panic!("format_c: integer conversion expects an integer argument"),
                },
                ConversionType::OctInt
                | ConversionType::HexIntLower
                | ConversionType::HexIntUpper => match arg {
                    VaArg::Int(v) => Arg::UInt(*v as u32),
                    VaArg::UInt(v) => Arg::UInt(*v),
                    VaArg::Long(v) => Arg::ULong(*v as u64),
                    VaArg::ULong(v) => Arg::ULong(*v),
                    VaArg::Ptr(p) => Arg::Size(p.to_int()),
                    VaArg::RawPtr(v) => Arg::Size(*v as usize),
                    VaArg::Double(_) => {
                        panic!("format_c: integer conversion expects an integer argument")
                    }
                },
                ConversionType::Char => Arg::Char(i32::get(arg) as u8 as char),
                ConversionType::String => match arg {
                    VaArg::Ptr(v) => Arg::Str(v.reinterpret_cast::<u8>().to_rust_string()),
                    _ => panic!("format_c: %s expects a string argument"),
                },
                ConversionType::DecFloatLower
                | ConversionType::DecFloatUpper
                | ConversionType::SciFloatLower
                | ConversionType::SciFloatUpper
                | ConversionType::CompactFloatLower
                | ConversionType::CompactFloatUpper => Arg::Double(f64::get(arg)),
                ConversionType::PercentSign => panic!("format_c: %% consumes no argument"),
            });
            pos += 1;
        }
    }
    let refs: Vec<&dyn Printf> = args.iter().map(Arg::as_printf).collect();
    match vsprintfp(&elements, &refs) {
        Ok(s) => s,
        Err(e) => panic!("format_c: cannot format {fmt:?}: {e:?}"),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::Ptr;

    fn s(lit: &'static [u8]) -> VaArg {
        Ptr::<u8>::from_string_literal(lit).into()
    }

    #[test]
    fn integers() {
        let va: Vec<VaArg> = vec![(-5i32).into(), 7u32.into(), (-9i64).into(), u64::MAX.into()];
        assert_eq!(
            format_c("%d %u %ld %lu", &va),
            "-5 7 -9 18446744073709551615"
        );
        let va: Vec<VaArg> = vec![(-1i32).into(), 255u32.into(), 8i64.into(), 255u64.into()];
        assert_eq!(format_c("%x %X %lo %lx", &va), "ffffffff FF 10 ff");
        assert_eq!(
            format_c("[%5d|%-5d|%05d]", &[1i32.into(), 2i32.into(), 3i32.into()]),
            "[    1|2    |00003]"
        );
    }

    #[test]
    fn strings_chars_and_floats() {
        let va: Vec<VaArg> = vec![s(b"abc"), (b'z' as i32).into(), 2.5f64.into()];
        assert_eq!(format_c("%s %c %.2f", &va), "abc z 2.50");
        assert_eq!(
            format_c("%8s|%-8s|", &[s(b"hi"), s(b"yo")]),
            "      hi|yo      |"
        );
        assert_eq!(format_c("%e", &[1500.0f64.into()]), "1.500000e+03");
        assert_eq!(format_c("%g", &[0.5f64.into()]), "0.5");
    }

    #[test]
    fn percent_signs_consume_no_argument() {
        assert_eq!(format_c("100%% %d%%", &[3i32.into()]), "100% 3%");
        assert_eq!(format_c("no conversions", &[]), "no conversions");
    }

    #[test]
    fn raw_pointers_as_hex() {
        let va: Vec<VaArg> = vec![
            VaArg::RawPtr(std::ptr::null_mut()),
            VaArg::RawPtr(0x10 as *mut _),
        ];
        assert_eq!(format_c("%x %x", &va), "0 10");
    }

    #[test]
    #[should_panic(expected = "integer conversion expects an integer argument")]
    fn wrong_argument_type_panics() {
        format_c("%d", &[1.5f64.into()]);
    }
}

// ============================================================================
// `ostream_field` -- ONE FORMATTED OUTPUT OPERATION on a C++ `std::ostream`
// with the WIDTH / FILL / ADJUSTFIELD state applied.  Row g3098.
//
// ⛔⛔ WHY THIS IS A FUNCTION AND NOT AN `OStream` TYPE WITH STICKY STATE.  The
// row that added it was briefed to build `libcc2rs::OStream { width, fill,
// adjustfield, ... } + impl std::io::Write` and re-point `rules/iostream`
// t1/t2/t3 onto it.  MEASURED (verif/g3098/osrow.unsafe.vlog, converter
// 086c2ff4, ir/g3090A, 99 modules) THAT DESIGN CANNOT WORK: a `<<` chain on a
// `std::ostream` is NOT rule-dispatched per operand.  `Converter::
// ConvertCallToOstream` (converter.cpp) flattens the whole chain into ONE
// `write!(os, "<fmt>", <args>)`, and a manipulator operand becomes an ORDINARY
// FORMAT ARGUMENT -- the emitted text never calls a method on the stream for
// it.  The converter asks for `std::ostream & operator shl(std::ostream &,
// const char *)` (10 asks, all `None`) and NEVER ONCE for
// `operator shl(std::ostream &, std::__iom_t6)` or `(..., std::__iom_t4<char>)`.
// ⭐ So no amount of state on the stream object is reachable: the zero-hit
// accessor-grep diagnostic applies -- if the converter never asks, no rule and
// no runtime type can answer.  The fix has to be in the layer that BUILDS the
// format string, which is why the converter change is where this row landed and
// why all this runtime needs to be is a pad-to-width helper.
//
// SEMANTICS, from [ostream.formatted.reqmts] / [facet.num.put.members]:
//  * the field is padded to `width` with `fill`;
//  * `left` pads on the RIGHT, everything else (the default) pads on the LEFT;
//  * if the field is already at least `width` wide it is written unchanged --
//    C++ NEVER TRUNCATES, which is why this cannot be `{:.*}`;
//  * width is ONE-SHOT: it is reset to 0 by the operation.  The reset lives in
//    the caller (ConvertCallToOstream clears its pending width after emitting
//    one field), not here.
//
// ⚠️ LENGTH IS COUNTED IN BYTES, deliberately.  A `char` stream's width is a
// count of `char`s, and `s.chars().count()` would pad a UTF-8 multibyte field
// to FEWER bytes than clang emits.
pub fn ostream_field(args: std::fmt::Arguments<'_>, width: usize, fill: char, left: bool) -> String {
    let body = args.to_string();
    if body.len() >= width {
        return body;
    }
    let pad: String = std::iter::repeat(fill).take(width - body.len()).collect();
    if left { body + &pad } else { pad + &body }
}

#[cfg(test)]
mod ostream_field_tests {
    use super::ostream_field;

    // Each case is one of the discriminators from verif/g3098/osrow.cpp, i.e.
    // each one makes a WRONG body differ rather than merely compile.
    #[test]
    fn width_fill_and_adjustfield_match_cpp() {
        // setw(8) on a 3-char field: an identity body prints 3 chars.
        assert_eq!(ostream_field(format_args!("{}", "abc"), 8, ' ', false), "     abc");
        // left vs right on the same value: an ignored adjustfield prints the
        // other one of these two.
        assert_eq!(ostream_field(format_args!("{}", "abc"), 8, ' ', true), "abc     ");
        // setfill('0'): a fill that defaulted to space prints "   42".
        assert_eq!(ostream_field(format_args!("{}", 42), 5, '0', false), "00042");
        // C++ NEVER TRUNCATES a too-wide field.
        assert_eq!(ostream_field(format_args!("{}", "abcdefgh"), 3, ' ', false), "abcdefgh");
        // width 0 / equal width are both pass-through.
        assert_eq!(ostream_field(format_args!("{}", "abc"), 0, '0', false), "abc");
        assert_eq!(ostream_field(format_args!("{}", "abc"), 3, '0', true), "abc");
        // the basefield composes: `std::hex` reaches us as the format trait.
        assert_eq!(ostream_field(format_args!("{:x}", 255), 4, '0', false), "00ff");
    }

    // BYTES, not chars -- see the note above the function.
    #[test]
    fn width_is_counted_in_bytes_like_a_char_stream() {
        assert_eq!(ostream_field(format_args!("{}", "é"), 4, '.', false), "..é");
    }
}
