use std::collections::VecDeque;
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
        desc: "print the first NUM bytes of each file;
                            with the leading '-', print all but the last
                            NUM bytes of each file",
    },
    HelpEntry {
        opt: "-n, --lines=[-]NUM",
        desc: "print the first NUM lines instead of the first 10;
                            with the leading '-', print all but the last
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

#[derive(Debug, Clone)]
pub enum Count {
    First(u128),
    AllButLast(u128),
}

#[derive(Default, Debug)]
pub struct Options {
    pub bytes: Option<Count>, // -c, --bytes=[-]NUM
    pub lines: Option<Count>, // -n, --lines=[-]NUM
    // bytes and lines are mutex (gnu principle: last one wins)
    pub quiet: bool,       // -q, --quiet, --silent
    pub verbose: bool,     // -v, --verbose
    pub null_termed: bool, // -z, --zero-terminated
}

pub fn run(opts: &Options, files: &[String]) -> Result<(), UtilError> {
    let mut out = io::stdout().lock();
    let mut sdin = io::stdin().lock();
    head_files(opts, files, &mut sdin, &mut out)?;
    Ok(())
}

fn expect_value<'a>(it: &mut ArgIter<'a>, opt: &'static str) -> Result<&'a str, ParseError> {
    match it.next() {
        Some(Arg::Value(v)) => Ok(v),
        _ => Err(ParseError::MissingOperand(opt)),
    }
}

fn normalize_suffix(s: &str) -> (&str, bool) {
    let s = s.trim();
    if let Some(prefix) = s.strip_suffix("iB") {
        (prefix, false)
    } else if let Some(prefix) = s.strip_suffix("B") {
        (prefix, true)
    } else {
        (s, false)
    }
}

fn suffix_mul(suf: &str, bytes: bool) -> Option<u128> {
    if bytes {
        match suf {
            "" => Some(1),
            "b" => Some(512),
            "k" => Some(1000u128),
            "m" => Some(1000u128.pow(2)),
            "g" => Some(1000u128.pow(3)),
            "t" => Some(1000u128.pow(4)),
            "p" => Some(1000u128.pow(5)),
            "e" => Some(1000u128.pow(6)),
            "z" => Some(1000u128.pow(7)),
            "y" => Some(1000u128.pow(8)),
            "r" => Some(1000u128.pow(9)),
            "q" => Some(1000u128.pow(10)),
            _ => None,
        };
    }
    match suf {
        "" => Some(1),
        "b" => Some(512),
        "k" => Some(1024u128),
        "m" => Some(1024u128.pow(2)),
        "g" => Some(1024u128.pow(3)),
        "t" => Some(1024u128.pow(4)),
        "p" => Some(1024u128.pow(5)),
        "e" => Some(1024u128.pow(6)),
        "z" => Some(1024u128.pow(7)),
        "y" => Some(1024u128.pow(8)),
        "r" => Some(1024u128.pow(9)),
        "q" => Some(1024u128.pow(10)),
        _ => None,
    }
}

fn parse_human_bytes(s: &str) -> Option<u128> {
    let idx = s.find(|c: char| !c.is_ascii_digit()).unwrap_or(s.len());
    let (num, suf) = s.split_at(idx);

    let n: u128 = num.parse().ok()?;

    let (suf, by_bytes) = normalize_suffix(suf);
    let mult = suffix_mul(&suf.to_ascii_lowercase(), by_bytes)?;

    n.checked_mul(mult)
}

fn parse_count(s: &str, opt: &'static str) -> Result<Count, ParseError> {
    let (neg, v) = if let Some(rest) = s.strip_prefix('-') {
        (true, rest)
    } else {
        (false, s)
    };

    let n = parse_human_bytes(v).ok_or(ParseError::BadValue(
        s.to_string(),
        opt,
        "invalid numeric suffix",
    ))?;

    if neg {
        Ok(Count::AllButLast(n))
    } else {
        Ok(Count::First(n))
    }
}

pub fn parse_args(args: &[String]) -> Result<ParseOutcome, ParseError> {
    let mut opts: Options = Default::default();
    let mut files = Vec::new();

    let mut it = ArgIter::new(args);

    opts.lines = Some(Count::First(10)); // default

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
                opts.lines = Some(parse_count(v, "-n")?);
                opts.bytes = None;
            }
            Arg::LongWithValue("lines", v) => {
                opts.lines = Some(parse_count(v, "--lines")?);
                opts.bytes = None;
            }
            Arg::Short('q') | Arg::Long("quiet") | Arg::Long("silent") => {
                opts.quiet = true;
                opts.verbose = false;
            }
            Arg::Short('v') | Arg::Long("verbose") => {
                opts.verbose = true;
                opts.quiet = false;
            }
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

pub fn head_files(
    opts: &Options,
    files: &[String],
    sdin: &mut impl Read,
    out: &mut impl Write,
) -> Result<(), UtilError> {
    let mut first = true;
    let delim = if opts.null_termed { b'\0' } else { b'\n' };

    if files.is_empty() {
        if opts.verbose {
            write!(out, "==> standard input <==").map_err(|e| UtilError::Io {
                path: "<stdin>".into(),
                err: e,
            })?;
        }
        match &opts.bytes {
            Some(n) => stdin_bytes(sdin, n, out, opts.null_termed).map_err(|e| UtilError::Io {
                path: "<stdin>".into(),
                err: e,
            })?,
            _ => {
                let n = match opts.lines {
                    Some(ref n) => n,
                    _ => &Count::First(10),
                };
                stdin_lines(sdin, n, out, delim).map_err(|e| UtilError::Io {
                    path: "<stdin>".into(),
                    err: e,
                })?;
            }
        }
    } else {
        for path in files {
            if files.len() > 1 {
                if !first {
                    writeln!(out).map_err(|e| UtilError::Io {
                        path: path.clone(),
                        err: e,
                    })?;
                }
                if !opts.quiet {
                    if path == "-" {
                        write!(out, "==> standard input <==").map_err(|e| UtilError::Io {
                            path: path.clone(),
                            err: e,
                        })?;
                    } else {
                        write!(out, "==> {} <==", path).map_err(|e| UtilError::Io {
                            path: path.clone(),
                            err: e,
                        })?;
                    }
                }
            } else if opts.verbose {
                if path == "-" {
                    write!(out, "==> standard input <==").map_err(|e| UtilError::Io {
                        path: path.clone(),
                        err: e,
                    })?;
                } else {
                    write!(out, "==> {} <==", path).map_err(|e| UtilError::Io {
                        path: path.clone(),
                        err: e,
                    })?;
                }
            }
            first = false;

            if path == "-" {
                match &opts.bytes {
                    Some(n) => {
                        stdin_bytes(sdin, n, out, opts.null_termed).map_err(|e| UtilError::Io {
                            path: "<stdin>".into(),
                            err: e,
                        })?
                    }
                    _ => {
                        let n = match opts.lines {
                            Some(ref n) => n,
                            _ => &Count::First(10),
                        };
                        stdin_lines(sdin, n, out, delim).map_err(|e| UtilError::Io {
                            path: "<stdin>".into(),
                            err: e,
                        })?;
                    }
                }
            } else {
                let mut file = File::open(path).map_err(|e| UtilError::Io {
                    path: path.clone(),
                    err: e,
                })?;
                match &opts.bytes {
                    Some(n) => head_bytes(&mut file, n, out, opts.null_termed).map_err(|e| {
                        UtilError::Io {
                            path: path.clone(),
                            err: e,
                        }
                    })?,
                    _ => {
                        let n = match opts.lines {
                            Some(ref n) => n,
                            _ => &Count::First(10),
                        };
                        head_lines(&mut file, n, out, delim).map_err(|e| UtilError::Io {
                            path: path.clone(),
                            err: e,
                        })?;
                    }
                }
            }
        }
    }

    Ok(())
}

fn head_lines(
    file: &mut File,
    bytes_count: &Count,
    out: &mut impl Write,
    delim: u8,
) -> io::Result<()> {
    match bytes_count {
        Count::First(n) => head_lines_bytes_first(file, *n, out, delim),
        Count::AllButLast(n) => head_lines_all_but_last(file, *n, out, delim),
    }
}

fn stdin_lines(
    sdin: &mut impl Read,
    bytes_count: &Count,
    out: &mut impl Write,
    delim: u8,
) -> io::Result<()> {
    match bytes_count {
        Count::First(n) => stdin_lines_bytes_first(sdin, *n, out, delim),
        Count::AllButLast(n) => stdin_lines_all_but_last(sdin, *n, out, delim),
    }
}

fn head_bytes(
    file: &mut File,
    bytes_count: &Count,
    out: &mut impl Write,
    term: bool,
) -> io::Result<()> {
    match bytes_count {
        Count::First(n) => head_bytes_first(file, *n, out, term),
        Count::AllButLast(n) => head_all_but_last_bytes(file, *n, out),
    }
}

fn stdin_bytes(
    sdin: &mut impl Read,
    bytes_count: &Count,
    out: &mut impl Write,
    term: bool,
) -> io::Result<()> {
    match bytes_count {
        Count::First(n) => read_stdin_bytes_first(sdin, *n, out, term),
        Count::AllButLast(n) => read_stdin_all_but_last(sdin, *n, out),
    }
}

fn head_lines_bytes_first(
    file: &mut File,
    max_lines: u128,
    out: &mut impl Write,
    delim: u8,
) -> io::Result<()> {
    let mut buf = [0u8; 8192];
    let mut lines = 0;

    if max_lines == 0 {
        io::copy(file, out)?;
        return Ok(());
    }

    while lines < max_lines {
        let n = file.read(&mut buf)?;
        if n == 0 {
            break;
        }

        for &b in &buf[..n] {
            out.write_all(&[b])?;

            if b == delim {
                lines += 1;
                if lines == max_lines {
                    return Ok(());
                }
            }
        }
    }
    Ok(())
}

fn stdin_lines_bytes_first(
    sdin: &mut impl Read,
    max_lines: u128,
    out: &mut impl Write,
    delim: u8,
) -> io::Result<()> {
    let mut buf = [0u8; 8192];
    let mut lines = 0;

    while lines < max_lines {
        let n = sdin.read(&mut buf)?;
        if n == 0 {
            break;
        }

        for &b in &buf[..n] {
            out.write_all(&[b])?;

            if b == delim {
                lines += 1;
                if lines == max_lines {
                    return Ok(());
                }
            }
        }
    }
    Ok(())
}

fn head_bytes_first(
    file: &mut File,
    max_bytes: u128,
    out: &mut impl Write,
    term: bool,
) -> io::Result<()> {
    let mut buf = [0u8; 8192];
    let mut written = 0u128;

    while written < max_bytes {
        let n = file.read(&mut buf)?;
        if n == 0 {
            break;
        }

        let remaining = (max_bytes - written) as usize;
        let to_write = n.min(remaining);

        for &b in &buf[..to_write] {
            out.write_all(&[b])?;
            written += to_write as u128;

            if term && b == b'\0' {
                return Ok(());
            }
        }

        if written == max_bytes {
            break;
        }
    }
    Ok(())
}

fn head_all_but_last_bytes(
    file: &mut File,
    tail_len: u128,
    out: &mut impl Write,
) -> io::Result<()> {
    let tail_len = tail_len as usize;
    let mut buf = [0u8; 8192];

    if tail_len == 0 {
        io::copy(file, out)?;
        return Ok(());
    }

    let mut ring: VecDeque<u8> = VecDeque::with_capacity(tail_len);

    loop {
        let n = file.read(&mut buf)?;
        if n == 0 {
            break;
        }

        for &byte in &buf[..n] {
            if ring.len() == tail_len {
                let b = ring.pop_front().unwrap();
                out.write_all(&[b])?;
            }
            ring.push_back(byte);
        }
    }

    Ok(())
}

fn head_lines_all_but_last(
    file: &mut File,
    tail_len: u128,
    out: &mut impl Write,
    delim: u8,
) -> io::Result<()> {
    let tail_len = tail_len as usize;
    let mut buf = [0u8; 8192];
    let mut lines = 0_usize;

    if tail_len == 0 {
        io::copy(file, out)?;
        return Ok(());
    }

    let mut ring: VecDeque<u8> = VecDeque::with_capacity(tail_len);

    loop {
        let n = file.read(&mut buf)?;
        if n == 0 {
            break;
        }

        for &byte in &buf[..n] {
            if ring.len() == tail_len {
                let b = ring.pop_front().unwrap();
                out.write_all(&[b])?;

                if b == delim {
                    lines += 1;
                    if lines == tail_len {
                        break;
                    }
                }
            }
            ring.push_back(byte);
        }
    }

    Ok(())
}

fn read_stdin_bytes_first(
    sdin: &mut impl Read,
    max_bytes: u128,
    out: &mut impl Write,
    term: bool,
) -> io::Result<()> {
    let mut buf = [0u8; 8192];
    let mut written = 0u128;

    while written < max_bytes {
        let n = sdin.read(&mut buf)?;
        if n == 0 {
            break;
        }

        let remaining = (max_bytes - written) as usize;
        let to_write = n.min(remaining);

        for &b in &buf[..to_write] {
            out.write_all(&[b])?;
            written += to_write as u128;

            if term && b == b'\0' {
                return Ok(());
            }
        }

        if written == max_bytes {
            break;
        }
    }
    Ok(())
}

fn read_stdin_all_but_last(
    sdin: &mut impl Read,
    tail_len: u128,
    out: &mut impl Write,
) -> io::Result<()> {
    let tail_len = tail_len as usize;
    let mut buf = [0u8; 8192];

    if tail_len == 0 {
        io::copy(sdin, out)?;
        return Ok(());
    }

    let mut ring: VecDeque<u8> = VecDeque::with_capacity(tail_len);

    loop {
        let n = sdin.read(&mut buf)?;
        if n == 0 {
            break;
        }

        for &byte in &buf[..n] {
            if ring.len() == tail_len {
                let b = ring.pop_front().unwrap();
                out.write_all(&[b])?;
            }
            ring.push_back(byte);
        }
    }
    Ok(())
}

fn stdin_lines_all_but_last(
    sdin: &mut impl Read,
    tail_len: u128,
    out: &mut impl Write,
    delim: u8,
) -> io::Result<()> {
    let tail_len = tail_len as usize;
    let mut buf = [0u8; 8192];
    let mut lines = 0_usize;

    if tail_len == 0 {
        io::copy(sdin, out)?;
        return Ok(());
    }

    let mut ring: VecDeque<u8> = VecDeque::with_capacity(tail_len);

    loop {
        let n = sdin.read(&mut buf)?;
        if n == 0 {
            break;
        }

        for &byte in &buf[..n] {
            if ring.len() == tail_len {
                let b = ring.pop_front().unwrap();
                out.write_all(&[b])?;

                if b == delim {
                    lines += 1;
                    if lines == tail_len {
                        break;
                    }
                }
            }
            ring.push_back(byte);
        }
    }

    Ok(())
}
