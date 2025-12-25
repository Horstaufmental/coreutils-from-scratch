use head::{head_files, parse_args, Count, Options, ParseError, ParseOutcome};
use std::io::{self, Cursor, Write};
use tempfile::NamedTempFile;

// PARSER TESTS

#[test]
fn default_is_10_line() {
    let args = vec![];
    let ParseOutcome::Ok(opts, files) = parse_args(&args).unwrap() else {
        panic!();
    };

    assert_eq!(
        match opts.lines {
            Some(c) => match c {
                Count::First(n) | Count::AllButLast(n) => n,
            },
            _ => 0,
        },
        10
    );
    assert!(opts.bytes.is_none());
    assert!(files.is_empty());
}

#[test]
fn parse_n_lines() {
    let args = vec!["-n".into(), "5".into()];
    let ParseOutcome::Ok(opts, _) = parse_args(&args).unwrap() else {
        panic!();
    };

    assert_eq!(
        match opts.lines {
            Some(c) => match c {
                Count::First(n) | Count::AllButLast(n) => n,
            },
            _ => 0,
        },
        5
    );
}

#[test]
fn parse_long_lines_equals() {
    let args = vec!["--lines=7".into()];
    let ParseOutcome::Ok(opts, _) = parse_args(&args).unwrap() else {
        panic!();
    };

    assert_eq!(
        match opts.lines {
            Some(c) => match c {
                Count::First(n) | Count::AllButLast(n) => n,
            },
            _ => 0,
        },
        7
    );
}

#[test]
fn parse_bytes_overrides_lines() {
    let args = vec!["-n".into(), "5".into(), "-c".into(), "3".into()];
    let ParseOutcome::Ok(opts, _) = parse_args(&args).unwrap() else {
        panic!();
    };

    assert!(opts.lines.is_none());
    assert!(match opts.bytes {
        Some(t) => match t {
            Count::First(3) => true,
            _ => false,
        },
        _ => false,
    });
}

#[test]
fn missing_value_is_error() {
    let args = vec!["-n".into()];
    let err = parse_args(&args).unwrap_err();

    matches!(err, ParseError::MissingOperand("-n"));
}

#[test]
fn unknown_option_errors() {
    let args = vec!["-Z".into()];
    let err = parse_args(&args).unwrap_err();

    matches!(err, ParseError::UnknownOption('Z'));
}

#[test]
fn help_short_circuits() {
    let args = vec!["--help".into()];
    assert!(matches!(parse_args(&args).unwrap(), ParseOutcome::Help));
}

// RUNTIME TESTS

#[test]
fn head_first_3_lines() {
    let mut file = NamedTempFile::new().unwrap();
    writeln!(file, "a\nb\nc\nd").unwrap();

    let opts = Options {
        lines: Some(Count::First(3)),
        ..Default::default()
    };

    let mut out = Vec::new();
    let mut sdin = io::stdin().lock();
    head_files(
        &opts,
        &[file.path().to_str().unwrap().into()],
        &mut sdin,
        &mut out,
    )
    .unwrap();

    let s = String::from_utf8(out).unwrap();
    assert_eq!(s, "a\nb\nc\n");
}

#[test]
fn head_first_4_bytes() {
    let mut file = NamedTempFile::new().unwrap();
    writeln!(file, "abcdef").unwrap();

    let opts = Options {
        bytes: Some(Count::First(4)),
        lines: None,
        ..Default::default()
    };

    let mut out = Vec::new();
    let mut sdin = io::stdin().lock();
    head_files(
        &opts,
        &[file.path().to_str().unwrap().into()],
        &mut sdin,
        &mut out,
    )
    .unwrap();

    let s = String::from_utf8(out).unwrap();
    assert_eq!(s, "abcd");
}

#[test]
fn multiple_files_print_headers() {
    let mut f1 = NamedTempFile::new().unwrap();
    let mut f2 = NamedTempFile::new().unwrap();

    writeln!(f1, "a").unwrap();
    writeln!(f2, "b").unwrap();

    let opts = Options {
        lines: Some(Count::First(1)),
        ..Default::default()
    };

    let mut out = Vec::new();
    let mut sdin = io::stdin().lock();
    head_files(
        &opts,
        &[
            f1.path().to_str().unwrap().into(),
            f2.path().to_str().unwrap().into(),
        ],
        &mut sdin,
        &mut out,
    )
    .unwrap();

    let s = String::from_utf8(out).unwrap();

    assert!(s.contains("==>"));
    assert!(s.contains("a"));
    assert!(s.contains("b"));
}

#[test]
fn dont_print_when_quiet_enabled() {
    let mut f1 = NamedTempFile::new().unwrap();
    let mut f2 = NamedTempFile::new().unwrap();

    writeln!(f1, "a").unwrap();
    writeln!(f2, "b").unwrap();

    let opts = Options {
        lines: Some(Count::First(1)),
        quiet: true,
        ..Default::default()
    };

    let mut out = Vec::new();
    let mut sdin = io::stdin().lock();
    head_files(
        &opts,
        &[
            f1.path().to_str().unwrap().into(),
            f2.path().to_str().unwrap().into(),
        ],
        &mut sdin,
        &mut out,
    )
    .unwrap();
    let s = String::from_utf8(out).unwrap();

    assert!(!s.contains("==>"));
    assert!(s.contains("a"));
    assert!(s.contains("b"));
}

#[test]
fn always_print_when_verbose_enabled() {
    let mut file = NamedTempFile::new().unwrap();
    writeln!(file, "a").unwrap();

    let opts = Options {
        lines: Some(Count::First(1)),
        verbose: true,
        ..Default::default()
    };

    let mut out = Vec::new();
    let mut sdin = io::stdin().lock();
    head_files(
        &opts,
        &[file.path().to_str().unwrap().into()],
        &mut sdin,
        &mut out,
    )
    .unwrap();
    let s = String::from_utf8(out).unwrap();

    assert!(s.contains("==>"));
    assert!(s.contains("a"));
}

#[test]
fn print_stdin_header_empty_verbose() {
    let opts = Options {
        verbose: true,
        ..Default::default()
    };

    let input = b"a\nb\nc";
    let mut out = Vec::new();
    let mut sdin = Cursor::new(input);
    head_files(&opts, &[], &mut sdin, &mut out).unwrap();
    let s = String::from_utf8(out).unwrap();

    assert!(s.contains("==> standard input <=="));
}

#[test]
fn print_file_und_stdin_header_non_verbose() {
    let mut file = NamedTempFile::new().unwrap();
    writeln!(file, "a").unwrap();

    let opts = Options {
        ..Default::default()
    };

    let input = b"b";
    let mut out = Vec::new();
    let mut sdin = Cursor::new(input);
    head_files(
        &opts,
        &[file.path().to_str().unwrap().into(), String::from("-")],
        &mut sdin,
        &mut out,
    )
    .unwrap();
    let s = String::from_utf8(out).unwrap();

    assert!(s.contains("==> standard input <=="));
}

#[test]
fn head_file_till_nul() {
    let opts = Options {
        null_termed: true,
        ..Default::default()
    };

    let input = b"a\nb\0\nc\nd";
    let mut out = Vec::new();
    let mut sdin = Cursor::new(input);
    head_files(&opts, &[], &mut sdin, &mut out).unwrap();
    let s = String::from_utf8(out).unwrap();

    assert!(s.contains("a\nb"))
}

#[test]
fn zero_terminated_lines() {
    let input = b"a\0b\0c\0";
    let mut out = Vec::new();
    let mut sdin = std::io::Cursor::new(input);

    let opts = Options {
        null_termed: true,
        lines: Some(Count::First(2)),
        ..Default::default()
    };

    head_files(&opts, &[], &mut sdin, &mut out).unwrap();

    assert_eq!(&out, b"a\0b\0");
}

#[test]
fn opt_verbose_overrides_quiet() {
    let args = vec!["--quiet".into(), "--verbose".into()];

    let ParseOutcome::Ok(opts, _) = parse_args(&args).unwrap() else {
        panic!();
    };

    assert_eq!(opts.quiet, false);
    assert_eq!(opts.verbose, true);
}
