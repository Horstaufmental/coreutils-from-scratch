use head::{head_files, parse_args, Options, ParseError, ParseOutcome};
use std::io::Write;
use tempfile::NamedTempFile;

// PARSER TESTS

#[test]
fn default_is_10_line() {
    let args = vec![];
    let ParseOutcome::Ok(opts, files) = parse_args(&args).unwrap() else {
        panic!();
    };

    assert_eq!(opts.lines, Some(10));
    assert_eq!(opts.bytes, None);
    assert!(files.is_empty());
}

#[test]
fn parse_n_lines() {
    let args = vec!["-n".into(), "5".into()];
    let ParseOutcome::Ok(opts, _) = parse_args(&args).unwrap() else {
        panic!();
    };

    assert_eq!(opts.lines, Some(5));
}

#[test]
fn parse_long_lines_equals() {
    let args = vec!["--lines=7".into()];
    let ParseOutcome::Ok(opts, _) = parse_args(&args).unwrap() else {
        panic!();
    };

    assert_eq!(opts.lines, Some(7));
}

#[test]
fn parse_bytes_overrides_lines() {
    let args = vec!["-n".into(), "5".into(), "-c".into(), "3".into()];
    let ParseOutcome::Ok(opts, _) = parse_args(&args).unwrap() else {
        panic!();
    };

    assert_eq!(opts.lines, None);
    assert_eq!(opts.bytes, Some(3));
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
        lines: Some(3),
        ..Default::default()
    };

    let mut out = Vec::new();
    head_files(&opts, &[file.path().to_str().unwrap().into()], &mut out).unwrap();

    let s = String::from_utf8(out).unwrap();
    assert_eq!(s, "a\nb\nc\n");
}

#[test]
fn head_first_4_bytes() {
    let mut file = NamedTempFile::new().unwrap();
    writeln!(file, "abcdef").unwrap();

    let opts = Options {
        bytes: Some(4),
        lines: None,
        ..Default::default()
    };

    let mut out = Vec::new();
    head_files(&opts, &[file.path().to_str().unwrap().into()], &mut out).unwrap();

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
        lines: Some(1),
        ..Default::default()
    };

    let mut out = Vec::new();
    head_files(
        &opts,
        &[
            f1.path().to_str().unwrap().into(),
            f2.path().to_str().unwrap().into(),
        ],
        &mut out,
    )
    .unwrap();

    let s = String::from_utf8(out).unwrap();

    assert!(s.contains("==>"));
    assert!(s.contains("a"));
    assert!(s.contains("b"));
}
