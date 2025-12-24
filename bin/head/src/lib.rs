use std::fs::File;
use std::io::{self, Read, Write};

use util::args::{Arg, ArgIter};
use util::error::UtilError;
use util::help::HelpEntry;
use util::meta::ProgramMeta;

pub static META: ProgramMeta = ProgramMeta {
    name: "head",
    project: "coreutils-rs from scratch",
    version: "1.0.0",
    authors: "Horstaufmental",
};

pub static HELP_ENTRIES: [HelpEntry; 7] = [
    HelpEntry {
        opt: "-c, --bytes=[-]NUM",
        desc: "print the first NUM bytes of each file;\n\
                with the leading '-', print all but the last\n\
                NUM bytes of each file",
    },
    HelpEntry {
        opt: "-n, --lines=[-]NUM",
        desc: "print the first NUM lines instead of the first 10;\n\
                with the leading '-', print all but the last\n\
                NUM lines of each file",
    },
    HelpEntry {
        opt: "-q, --quiet, --silent",
        desc: "never print headers giving file names",
    },
    HelpEntry {
        opt: "-v, --verbose",
        desc: "always print headers giving file names",
    },
    HelpEntry {
        opt: "-z, --zero-terminated",
        desc: "line delimiter is NUL, not newline",
    },
    HelpEntry {
        opt: "    --help",
        desc: "display this help and exit",
    },
    HelpEntry {
        opt: "    --version",
        desc: "output version information and exit",
    },
];

#[derive(Debug)]
pub enum ParseError {
    UnknownOption(char),
    UnknownLongOption(String),
    MissingOperand(&'static str),
    BadValue(String, &'static str, &'static str),
    NoInput,
}

#[derive(Debug)]
pub enum ParseOutcome {
    Ok(Options, Vec<String>),
    Help,
    Version,
}

impl std::fmt::Display for ParseError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            ParseError::UnknownOption(c) => write!(f, "unknown option -- '{}'", c),
            ParseError::UnknownLongOption(opt) => write!(f, "unrecognized option '--{}'", opt),
            ParseError::MissingOperand(opt) => write!(f, "option '{}' requires an argument", opt),
            ParseError::BadValue(v, opt, valid) => {
                write!(f, "ambiguous argument '{}' for '{}'\n{}", v, opt, valid)
            }
            ParseError::NoInput => write!(f, "missing operand"),
        }
    }
}

impl From<ParseError> for UtilError {
    fn from(err: ParseError) -> Self {
        UtilError::Parse(err.to_string())
    }
}

#[derive(Default, Debug)]
pub struct Options {
    pub bytes: Option<u64>, // -c, --bytes=[-]NUM
    pub lines: Option<u64>, // -n, --lines=[-]NUM
    // bytes and lines are mutex (gnu principle: last one wins)
    pub quiet: bool,       // -q, --quiet, --silent
    pub verbose: bool,     // -v, --verbose
    pub null_termed: bool, // -z, --zero-terminated
}

fn expect_value<'a>(it: &mut ArgIter<'a>, opt: &'static str) -> Result<&'a str, ParseError> {
    match it.next() {
        Some(Arg::Value(v)) => Ok(v),
        _ => Err(ParseError::MissingOperand(opt)),
    }
}

fn parse_positive(v: &str, opt: &'static str) -> Result<u64, ParseError> {
    match v.parse::<u64>() {
        Ok(0) => Err(ParseError::BadValue(
            v.to_string(),
            opt,
            "value must be greater than zero",
        )),
        Ok(n) => Ok(n),
        Err(_) => Err(ParseError::BadValue(
            v.to_string(),
            opt,
            "invalid numeric value",
        )),
    }
}

fn split_count(s: &str) -> (&str, &str) {
    let idx = s.find(|c: char| !c.is_ascii_digit()).unwrap_or(s.len());
    (&s[..idx], &s[idx..])
}

fn parse_count(s: &str, opt: &'static str) -> Result<u64, ParseError> {
    let (num, suf) = split_count(s);

    let base: u64 = parse_positive(num, opt)
        .map_err(|_| ParseError::BadValue(s.to_string(), opt, "invalid number"))?;

    let mult = match suf {
        "" => 1,
        "b" => 512,
        "K" => 1_024,
        "M" => 1_024 * 1_024,
        "G" => 1_024 * 1_024 * 1_024,
        "T" => 1_024 * 1_024 * 1_024 * 1_024,
        "P" => 1_024 * 1_024 * 1_024 * 1_024 * 1_024,
        "E" => 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024,
        "Z" => 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024,
        "Y" => 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024,
        "R" => 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024,
        "Q" => 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024 * 1_024,
        // bytes
        "kB" | "KB" => 1_000,
        "MB" => 1_000 * 1_000,
        "GB" => 1_000 * 1_000 * 1_000,
        "TB" => 1_000 * 1_000 * 1_000 * 1_000,
        "PB" => 1_000 * 1_000 * 1_000 * 1_000 * 1_000,
        "EB" => 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000,
        "ZB" => 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000,
        "YB" => 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000,
        "RB" => 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000,
        "QB" => 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000 * 1_000,
        _ => {
            return Err(ParseError::BadValue(s.to_string(), opt, "invalid suffix"));
        }
    };

    base.checked_mul(mult)
        .ok_or_else(|| ParseError::BadValue(s.to_string(), opt, "value too large"))
}

pub fn parse_args(args: &[String]) -> Result<ParseOutcome, ParseError> {
    let mut opts: Options = Default::default();
    let mut files = Vec::new();

    let mut it = ArgIter::new(args);

    opts.lines = Some(10); // default

    while let Some(arg) = it.next() {
        match arg {
            Arg::Short('c') => {
                let v = expect_value(&mut it, "-c")?;
                opts.bytes = Some(parse_count(v, "-c")?);
                opts.lines = None;
            }
            Arg::LongWithValue("bytes", v) => {
                opts.bytes = Some(parse_count(v, "--bytes")?);
                opts.lines = None;
            }
            Arg::Short('n') => {
                let v = expect_value(&mut it, "-n")?;
                opts.lines = Some(parse_positive(v, "-n")?);
                opts.bytes = None;
            }
            Arg::LongWithValue("lines", v) => {
                opts.lines = Some(parse_positive(v, "--lines")?);
                opts.bytes = None;
            }
            Arg::Short('q') | Arg::Long("quiet") | Arg::Long("silent") => opts.quiet = true,
            Arg::Short('v') | Arg::Long("verbose") => opts.verbose = true,
            Arg::Short('z') | Arg::Long("zero-terminated") => opts.null_termed = true,

            Arg::Long("help") => return Ok(ParseOutcome::Help),
            Arg::Long("version") => return Ok(ParseOutcome::Version),

            Arg::Value(v) => files.push(v.to_string()),
            Arg::EndOfOptions => {
                files.extend(it.map(|a| match a {
                    Arg::Value(v) => v.to_string(),
                    _ => unreachable!(),
                }));
                break;
            }

            Arg::Short(c) => return Err(ParseError::UnknownOption(c)),
            Arg::Long(l) => return Err(ParseError::UnknownLongOption(l.to_string())),
            _ => {}
        }
    }

    Ok(ParseOutcome::Ok(opts, files))
}

pub fn head_files(opts: &Options, files: &[String], out: &mut impl Write) -> Result<(), UtilError> {
    let mut first = true;

    for path in files {
        if files.len() > 1 {
            if !first {
                writeln!(out).map_err(|e| UtilError::Io {
                    path: path.clone(),
                    err: e,
                })?;
            }
            write!(out, "==> {} <==", path).map_err(|e| UtilError::Io {
                path: path.clone(),
                err: e,
            })?;
        }
        first = false;

        let mut file = File::open(path).map_err(|e| UtilError::Io {
            path: path.clone(),
            err: e,
        })?;
        match opts.bytes {
            Some(n) => head_bytes(&mut file, n, out).map_err(|e| UtilError::Io {
                path: path.clone(),
                err: e,
            })?,
            _ => {
                let n = opts.lines.unwrap_or(10);
                head_lines(&mut file, n, out).map_err(|e| UtilError::Io {
                    path: path.clone(),
                    err: e,
                })?;
            }
        }
    }

    Ok(())
}

fn head_lines(file: &mut File, max_lines: u64, out: &mut impl Write) -> io::Result<()> {
    let mut buf = [0u8; 8192];
    let mut lines = 0;

    while lines < max_lines {
        let n = file.read(&mut buf)?;
        if n == 0 {
            break;
        }

        for &b in &buf[..n] {
            out.write_all(&[b])?;

            if b == b'\n' {
                lines += 1;
                if lines == max_lines {
                    return Ok(());
                }
            }
        }
    }
    Ok(())
}

fn head_bytes(file: &mut File, max_bytes: u64, out: &mut impl Write) -> io::Result<()> {
    let mut buf = [0u8; 8192];
    let mut written = 0u64;

    while written < max_bytes {
        let n = file.read(&mut buf)?;
        if n == 0 {
            break;
        }

        let remaining = (max_bytes - written) as usize;
        let to_write = n.min(remaining);

        out.write_all(&buf[..to_write])?;
        written += to_write as u64;

        if written == max_bytes {
            break;
        }
    }
    Ok(())
}
