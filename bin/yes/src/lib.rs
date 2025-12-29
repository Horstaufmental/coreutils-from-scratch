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
use std::io::{self, Write};

use util::args::{Arg, ArgIter};
use util::error::UtilError;
use util::help::HelpEntry;
use util::meta::ProgramMeta;

pub static META: ProgramMeta = ProgramMeta {
    name: "yes",
    project: "coreutils-rs from scratch",
    version: "1.0.0",
    authors: "Horstaufmental",
};

pub static HELP_ENTRIES: [HelpEntry; 2] = [
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
    Ok(Vec<String>),
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

pub fn run(str: &Vec<String>) -> Result<(), UtilError> {
    let mut out = io::stdout().lock();
    yes(str, &mut out, false)?;
    Ok(())
}

pub fn parse_args(args: &[String]) -> Result<ParseOutcome, ParseError> {
    let mut str = Vec::new();

    let mut it = ArgIter::new(args);

    while let Some(arg) = it.next() {
        match arg {
            Arg::Long("help") => {
                return Ok(ParseOutcome::Help);
            }
            Arg::Long("version") => {
                return Ok(ParseOutcome::Version);
            }

            Arg::Value(v) => str.push(v.to_string()),

            Arg::EndOfOptions => {
                str.extend(it.map(|a| match a {
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

    Ok(ParseOutcome::Ok(str))
}

pub fn yes(str: &Vec<String>, out: &mut impl Write, test: bool) -> Result<(), UtilError> {
    let output_str: String;
    if str.is_empty() {
        output_str = String::from("y");
    } else {
        output_str = str.join(" ");
    }
    loop {
        writeln!(out, "{}", output_str).map_err(|e| UtilError::IoNoPath { err: e })?;
        if test {
            break;
        }
    }
    Ok(())
}
