use std::io::Write;

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
    pub bytes: Option<u64>,  // -c, --bytes=[-]NUM
    pub lines: Option<u64>,  // -n, --lines=[-]NUM
    // bytes and lines are mutex (gnu principle: last one wins)
    pub quiet: bool,         // -q, --quiet, --silent
    pub verbose: bool,       // -v, --verbose
    pub null_termed: bool,   // -z, --zero-terminated
}

fn expect_value<'a>(
    it: &mut ArgIter<'a>,
    opt: &'static str,
) -> Result<&'a str, ParseError> {
    match it.next() {
        Some(Arg::Value(v)) => Ok(v),
        _ => Err(ParseError::MissingOperand(opt)),
    }
}

fn parse_positive(
    v: &str,
    opt: &'static str,
) -> Result<u64, ParseError> {
    match v.parse::<u64>() {
        Ok(0) => Err(ParseError::BadValue(v.to_string(), opt, "value must be greater than zero")),
        Ok(n) => Ok(n),
        Err(_) => Err(ParseError::BadValue(v.to_string(), opt, "invalid numeric value")),
    }
}

pub fn parse_args(args: &[String]) -> Result<ParseOutcome, ParseError> {
    let mut opts: Options = Default::default();
    let mut files = Vec::new();

    let mut it = ArgIter::new(args);

    while let Some(arg) = it.next() {
        match arg {
            Arg::Short('c') => {
                let v = expect_value(&mut it, "-c")?;
                opts.bytes = Some(parse_positive(v, "-c")?);
                opts.lines = None;
            }
            Arg::LongWithValue("bytes", v) => {
                opts.bytes = Some(parse_positive(v, "--bytes")?);
                opts.lines = None;
            }
            Arg::Short('n') | Arg::Long("lines") => {
                let v = expect_value(&mut it, "-n")?;
                opts.lines = Some(parse_positive(v, "-n")?);
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
    Ok(())
}