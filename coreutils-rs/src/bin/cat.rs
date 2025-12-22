use std::env;
use std::fs::File;
use std::io::{self, BufRead, Write};
use std::process;

static PROGRAM_NAME: &str = "cat";
static PROJECT_NAME: &str = "coreutils-rs from scratch";
static AUTHORS: &str = "Horstaufmental";
static VERSION: &str = "1.0";

#[derive(Debug)]
pub enum ParseError {
    UnknownOption(char),
    UnknownLongOption(String),
    MissingOperand(&'static str),
    NoInput,
}

impl std::fmt::Display for ParseError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            ParseError::UnknownOption(c) => write!(f, "unknown option -- '{}'", c),
            ParseError::UnknownLongOption(opt) => write!(f, "unrecognized option '--{}'", opt),
            ParseError::MissingOperand(opt) => write!(f, "option '{}' requires an argument", opt),
            ParseError::NoInput => write!(f, "missing operand"),
        }
    }
}

#[derive(Debug)]
pub enum UtilError {
    Parse(ParseError),
    Io { path: String, err: std::io::Error },
}

impl UtilError {
    fn exit_code(&self) -> i32 {
        match self {
            UtilError::Parse(_) => 1,
            UtilError::Io { path: _, err: _ } => 1,
        }
    }
}

impl std::fmt::Display for UtilError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            UtilError::Parse(err) => err.fmt(f),
            UtilError::Io { path, err } => {
                write!(f, "{}: {}", path, err)
            }
        }
    }
}

impl From<ParseError> for UtilError {
    fn from(err: ParseError) -> Self {
        UtilError::Parse(err)
    }
}

#[derive(Default, Debug)]
struct Options {
    number_nonblank: bool,  // -b, --number_nonblank
    show_ends: bool,        // -E, --show_ends
    number: bool,           // -n, --number
    squeeze_blank: bool,    // -s, --squeeze-blank
    show_tabs: bool,        // -T, --show-tabs
    show_nonprinting: bool, // -v, --show-nonprinting
    files: Vec<String>,
}

enum LongOptions {
    NumberNonBlank,
    ShowEnds,
    Number,
    SqueezeBlank,
    ShowTabs,
    ShowNonPrinting,
    Help,
    Version,
}

fn main() {
    if let Err(e) = run() {
        eprintln!("cat: {}", e);

        if matches!(e, UtilError::Parse(_)) {
            eprintln!("Try 'cat --help' for more information.");
        }

        process::exit(e.exit_code());
    }
}

fn parse_args() -> Result<Options, ParseError> {
    let mut args = env::args();
    let mut options: Options = Default::default();
    args.next(); // skip program name
    while let Some(arg) = args.next() {
        if arg == "--" {
            options.files.extend(args);
            break;
        }

        if let Some(rest) = arg.strip_prefix("--") {
            let (name, _value) = match rest.split_once('=') {
                Some((n, v)) => (n, Some(v)),
                None => (rest, None),
            };

            match parse_long_opt(name)? {
                LongOptions::NumberNonBlank => options.number_nonblank = true,
                LongOptions::ShowEnds => options.show_ends = true,
                LongOptions::Number => options.number = true,
                LongOptions::SqueezeBlank => options.squeeze_blank = true,
                LongOptions::ShowTabs => options.show_tabs = true,
                LongOptions::ShowNonPrinting => options.show_nonprinting = true,
                LongOptions::Help => (),
                LongOptions::Version => {
                    print_version();
                    process::exit(0);
                }
            }
        }

        if arg.starts_with('-') && arg.len() > 1 {
            for ch in arg[1..].chars() {
                match ch {
                    'A' => {
                        options.show_nonprinting = true;
                        options.show_ends = true;
                        options.show_tabs = true;
                    }
                    'b' => options.number_nonblank = true,
                    'e' => {
                        options.show_nonprinting = true;
                        options.show_ends = true;
                    }
                    'E' => options.show_ends = true,
                    'n' => options.number = true,
                    's' => options.squeeze_blank = true,
                    't' => {
                        options.show_nonprinting = true;
                        options.show_tabs = true;
                    }
                    'T' => options.show_tabs = true,
                    'u' => (), // ignored
                    'v' => options.show_nonprinting = true,
                    _ => return Err(ParseError::UnknownOption(ch)),
                }
            }
        }
        options.files.push(arg);
    }
    Ok(options)
}

fn parse_long_opt(name: &str) -> Result<LongOptions, ParseError> {
    match name {
        "number-nonblank" => Ok(LongOptions::NumberNonBlank),
        "show-ends" => Ok(LongOptions::ShowEnds),
        "number" => Ok(LongOptions::Number),
        "squeeze-blank" => Ok(LongOptions::SqueezeBlank),
        "show-tabs" => Ok(LongOptions::ShowTabs),
        "show-nonprinting" => Ok(LongOptions::ShowNonPrinting),
        "help" => Ok(LongOptions::Help),
        "version" => Ok(LongOptions::Version),
        _ => Err(ParseError::UnknownLongOption(name.to_string())),
    }
}

fn run() -> Result<(), UtilError> {
    let opts = parse_args()?;
    cat_files(opts)?;
    Ok(())
}

// our C's cat reads buffer by char, in this case we're reading by line
// so we'll have to adapt our code to bufread
fn cat_files(opts: Options) -> Result<(), UtilError> {
    if opts.files.len() < 1 {
        return Err(UtilError::Parse(ParseError::NoInput));
    }

    for path in &opts.files {
        let file = File::open(&path).map_err(|e| UtilError::Io {
            path: path.clone(),
            err: e,
        })?;
        let reader = io::BufReader::new(file);
        let mut out = io::stdout().lock();

        for line in reader.lines() {
            let line = line.map_err(|e| UtilError::Io {
                path: path.clone(),
                err: e,
            })?;
            writeln!(out, "{}", line).map_err(|e| UtilError::Io {
                path: path.clone(),
                err: e,
            })?;
        }
    }

    Ok(())
}

fn print_line_num(opts: Options, is_blank: bool, mut line_num: u32) {
    if opts.number_nonblank && is_blank {
        return;
    }
    if opts.number || (opts.number_nonblank && !is_blank) {
        line_num += 1;
        print!("{:>6}", line_num);
    }
}

fn print_version() {
    println!("{} ({}) {}", PROGRAM_NAME, PROJECT_NAME, VERSION);
    println!("Copyright (C) 2025 {}", AUTHORS);
    println!(
        "License GPLv3+: GNU GPL version 3 or later \
             <https://gnu.org/licenses/gpl.html>.\n\
             This is free software: you are free to change and redistribute it.\n\
             There is NO WARRANTY, to the extent permitted by law.\n"
    );
    println!("Written by {}", AUTHORS);
}
