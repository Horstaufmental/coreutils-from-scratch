/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of coreutils-rs from scratch.
 * Copyright (c) 2025 Horstaufmental
 *
 * coreutils-rs from scratch is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * coreutils-rs from scratch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */
use std::env;
use std::fs::File;
use std::io::{self, Read, Write};
use std::process;

static PROGRAM_NAME: &'static str = "cat";
static PROJECT_NAME: &'static str = "coreutils-rs from scratch";
static AUTHORS: &'static str = "Horstaufmental";
static VERSION: &'static str = "1.0";

struct HelpEntry {
    opt: &'static str,
    desc: &'static str,
}

static HELP_ENTRIES: [HelpEntry; 12] = [
    HelpEntry {
        opt: "-A, --show-all",
        desc: "equivalent to -vET",
    },
    HelpEntry {
        opt: "-b, --number-nonblank",
        desc: "number nonempty output lines, overrides -n",
    },
    HelpEntry {
        opt: "-e",
        desc: "equivalent to -vE",
    },
    HelpEntry {
        opt: "-E, --show-ends",
        desc: "display $ at end of each line",
    },
    HelpEntry {
        opt: "-n, --number",
        desc: "number all output lines",
    },
    HelpEntry {
        opt: "-s, --squeeze-blank",
        desc: "suppress repeated empty output lines",
    },
    HelpEntry {
        opt: "-t",
        desc: "equivalent to -vT",
    },
    HelpEntry {
        opt: "-T, --show-tabs",
        desc: "display TAB characters as ^I",
    },
    HelpEntry {
        opt: "-u",
        desc: "(ignored)",
    },
    HelpEntry {
        opt: "-v, --show-nonprinting",
        desc: "use ^ and M- notation, except for LFD and TAB",
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
                _ => (rest, None),
            };

            match parse_long_opt(name)? {
                LongOptions::NumberNonBlank => options.number_nonblank = true,
                LongOptions::ShowEnds => options.show_ends = true,
                LongOptions::Number => options.number = true,
                LongOptions::SqueezeBlank => options.squeeze_blank = true,
                LongOptions::ShowTabs => options.show_tabs = true,
                LongOptions::ShowNonPrinting => options.show_nonprinting = true,
                LongOptions::Help => {
                    print_help(PROGRAM_NAME.to_string());
                    process::exit(0);
                }
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

fn cat_files(opts: Options) -> Result<(), UtilError> {
    let mut out = io::stdout().lock();

    if opts.files.len() < 1 {
        read_stdin(&mut out).map_err(|e| UtilError::Io {
            path: "<stdout>".into(),
            err: e,
        })?;
        return Ok(());
    }

    for path in &opts.files {
        if path == "-" {
            read_stdin(&mut out).map_err(|e| UtilError::Io {
                path: "<stdout>".into(),
                err: e,
            })?;
            continue;
        }
        let mut file = File::open(&path).map_err(|e| UtilError::Io {
            path: path.clone(),
            err: e,
        })?;

        let mut buffer = [0u8; 32768]; // gnu cat buffer size
        let mut at_line_start = true;
        let mut prev_blank = false;

        let mut line_num: u64 = 0;

        loop {
            let n = file.read(&mut buffer).map_err(|e| UtilError::Io {
                path: path.clone(),
                err: e,
            })?;
            if n == 0 {
                break;
            }

            for i in 0..n {
                let c = buffer[i];

                if c == b'\n' {
                    if prev_blank && opts.squeeze_blank {
                        continue;
                    }

                    if at_line_start {
                        print_line_num(true, &opts, &mut line_num, &mut out).map_err(|e| {
                            UtilError::Io {
                                path: path.clone(),
                                err: e,
                            }
                        })?;
                    }

                    if opts.show_ends {
                        write!(out, "$").map_err(|e| UtilError::Io {
                            path: path.clone(),
                            err: e,
                        })?;
                    }

                    write!(out, "\n").map_err(|e| UtilError::Io {
                        path: path.clone(),
                        err: e,
                    })?;
                    prev_blank = true;
                    at_line_start = true;
                } else {
                    if at_line_start {
                        print_line_num(false, &opts, &mut line_num, &mut out).map_err(|e| {
                            UtilError::Io {
                                path: path.clone(),
                                err: e,
                            }
                        })?;
                        at_line_start = false;
                    }
                    prev_blank = false;

                    if c == b'\t' && opts.show_tabs {
                        write!(out, "^I").map_err(|e| UtilError::Io {
                            path: path.clone(),
                            err: e,
                        })?;
                    } else {
                        print_vis(c, opts.show_nonprinting, &mut out).map_err(|e| {
                            UtilError::Io {
                                path: path.clone(),
                                err: e,
                            }
                        })?;
                    }
                }
            }
        }
    }

    Ok(())
}

fn read_stdin(out: &mut impl Write) -> io::Result<()> {
    let mut buffer = [0u8; 32768]; // gnu cat buffer size
    let mut sdin = io::stdin().lock();

    loop {
        let n = sdin.read(&mut buffer)?;
        if n == 0 {
            break;
        }

        for i in 0..n {
            let c = buffer[i];
            write!(out, "{}", c as char)?;
        }
    }

    Ok(())
}

fn print_line_num(
    is_blank: bool,
    opts: &Options,
    line_num: &mut u64,
    out: &mut impl Write,
) -> io::Result<()> {
    if opts.number_nonblank && is_blank {
        return Ok(());
    }
    if opts.number || (opts.number_nonblank && !is_blank) {
        *line_num += 1;
        write!(out, "{:>6}", *line_num)?;
    }

    Ok(())
}

fn print_vis(c: u8, show_nonprint: bool, out: &mut impl Write) -> io::Result<()> {
    if !show_nonprint {
        write!(out, "{}", c as char)?;
        return Ok(());
    }

    match c {
        b'\n' | b'\t' => write!(out, "{}", c as char)?,
        0..=31 => write!(out, "^{}", (c + 64) as char)?,
        127 => write!(out, "^?")?,
        128..=255 => {
            write!(out, "M-")?;
            print_vis(c - 128, true, out)?;
        }
        _ => write!(out, "{}", c as char)?,
    }

    Ok(())
}

fn print_help(name: String) {
    println!("Usage: {} [OPTION]... [FILE]...", name);
    println!("Concatenate FILE(s) to standard output.\n");

    println!("With no FILE, or when FILE is -, read standard input.\n");

    let mut maxlen = 0;
    for entry in HELP_ENTRIES.iter() {
        let len = entry.opt.len();
        if len > maxlen {
            maxlen = len;
        }
    }

    for entry in HELP_ENTRIES.iter() {
        println!("  {:<width$}  {}", entry.opt, entry.desc, width = maxlen);
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
