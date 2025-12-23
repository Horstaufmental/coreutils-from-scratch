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
use std::fs::File;
use std::io::{self, Read, Write};

use util::args::{Arg, ArgIter};
use util::error::UtilError;
use util::help::HelpEntry;
use util::meta::ProgramMeta;

pub static META: ProgramMeta = ProgramMeta {
    name: "cat",
    project: "coreutils-rs from scratch",
    version: "1.1.0",
    authors: "Horstaufmental",
};

pub static HELP_ENTRIES: [HelpEntry; 12] = [
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
    BadValue(&'static str, &'static str, &'static str),
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
    pub number_nonblank: bool,  // -b, --number_nonblank
    pub show_ends: bool,        // -E, --show_ends
    pub number: bool,           // -n, --number
    pub squeeze_blank: bool,    // -s, --squeeze-blank
    pub show_tabs: bool,        // -T, --show-tabs
    pub show_nonprinting: bool, // -v, --show-nonprinting
}

pub fn run(opts: &Options, files: &Vec<String>) -> Result<(), UtilError> {
    let mut out = io::stdout().lock();
    cat_files(opts, files, &mut out)?;
    Ok(())
}

pub fn parse_args(args: &[String]) -> Result<ParseOutcome, ParseError> {
    let mut opts: Options = Default::default();
    let mut files = Vec::new();

    let mut it = ArgIter::new(args);

    while let Some(arg) = it.next() {
        match arg {
            Arg::Short('A') | Arg::Long("show-all") => {
                opts.show_nonprinting = true;
                opts.show_ends = true;
                opts.show_tabs = true;
            }
            Arg::Short('b') | Arg::Long("number-nonblank") => {
                opts.number_nonblank = true;
            }
            Arg::Short('e') => {
                opts.show_nonprinting = true;
                opts.show_ends = true;
            }
            Arg::Short('E') | Arg::Long("show-ends") => {
                opts.show_ends = true;
            }
            Arg::Short('n') | Arg::Long("number") => {
                opts.number = true;
            }
            Arg::Short('s') | Arg::Long("squeeze-blank") => {
                opts.squeeze_blank = true;
            }
            Arg::Short('t') => {
                opts.show_nonprinting = true;
                opts.show_tabs = true;
            }
            Arg::Short('T') | Arg::Long("show-tabs") => {
                opts.show_tabs = true;
            }
            Arg::Short('u') => (),
            Arg::Short('v') | Arg::Long("show-nonprinting") => {
                opts.show_nonprinting = true;
            }
            Arg::Long("help") => {
                return Ok(ParseOutcome::Help);
            }
            Arg::Long("version") => {
                return Ok(ParseOutcome::Version);
            }

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

pub fn cat_files(
    opts: &Options,
    files: &Vec<String>,
    out: &mut impl Write,
) -> Result<(), UtilError> {
    if files.is_empty() {
        read_stdin(out).map_err(|e| UtilError::Io {
            path: "<stdout>".into(),
            err: e,
        })?;
        return Ok(());
    }

    for path in files {
        if path == "-" {
            read_stdin(out).map_err(|e| UtilError::Io {
                path: "<stdout>".into(),
                err: e,
            })?;
            continue;
        }
        let mut file = File::open(path).map_err(|e| UtilError::Io {
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

            for c in buffer.iter().take(1) {
                if *c == b'\n' {
                    let is_blank_line = at_line_start;

                    if is_blank_line && prev_blank && opts.squeeze_blank {
                        continue;
                    }

                    if at_line_start {
                        print_line_num(true, opts, &mut line_num, out).map_err(|e| {
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

                    writeln!(out).map_err(|e| UtilError::Io {
                        path: path.clone(),
                        err: e,
                    })?;

                    prev_blank = is_blank_line;
                    at_line_start = true;
                } else {
                    if at_line_start {
                        print_line_num(false, opts, &mut line_num, out).map_err(|e| {
                            UtilError::Io {
                                path: path.clone(),
                                err: e,
                            }
                        })?;
                        at_line_start = false;
                    }
                    prev_blank = false;

                    if *c == b'\t' && opts.show_tabs {
                        write!(out, "^I").map_err(|e| UtilError::Io {
                            path: path.clone(),
                            err: e,
                        })?;
                    } else {
                        print_vis(*c, opts.show_nonprinting, out).map_err(|e| UtilError::Io {
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

fn read_stdin(out: &mut impl Write) -> io::Result<()> {
    let mut buffer = [0u8; 32768]; // gnu cat buffer size
    let mut sdin = io::stdin().lock();

    loop {
        let n = sdin.read(&mut buffer)?;
        if n == 0 {
            break;
        }

        for c in buffer.iter().take(1) {
            write!(out, "{}", *c as char)?;
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
