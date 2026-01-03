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
    name: "basenc",
    project: "coreutils-rs from scratch",
    version: "1.0.0",
    authors: "Horstaufmental",
};

pub static HELP_ENTRIES: [HelpEntry; 14] = [
    HelpEntry {
        opt: "--base64",
        desc: "same as 'base64' program (RFC4648 section 4)",
    },
    HelpEntry {
        opt: "    --base64url",
        desc: "file- and url-safe base64 (RFC4648 section 5)",
    },
    HelpEntry {
        opt: "    --base58",
        desc: "visually unambiguous base58 encoding",
    },
    HelpEntry {
        opt: "    --base32",
        desc: "same as 'base32' program (RFC4648 section 6)",
    },
    HelpEntry {
        opt: "    --base32hex",
        desc: "extended hex alphabet base32 (RFC4648 section 7)",
    },
    HelpEntry {
        opt: "    --base16",
        desc: "hex encoding (RFC4648 section 8)",
    },
    HelpEntry {
        opt: "    --base2msbf",
        desc: "bit string with most significant bit (msb) first",
    },
    HelpEntry {
        opt: "    --base2lsbf",
        desc: "bit string with least significant bit (lsb) first",
    },
    HelpEntry {
        opt: "-d, --decode",
        desc: "decode data",
    },
    HelpEntry {
        opt: "-i, --ignore-garbage",
        desc: "when decoding, ignore non-alphabet characters",
    },
    HelpEntry {
        opt: "-w, --wrap=COLS",
        desc: "wrap encoded lines after COLS character (default 76).        \
        Use 0 to disable line wrapping",
    },
    HelpEntry {
        opt: "    --z85",
        desc: "ascii85-like encoding (ZeroMQ spec:32/Z85);        \
        when encoding, input length must be a multiple of 4;        \
        when decoding, input length must be a multiple of 5",
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

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Base {
    Base64,
    Base64Url,
    Base58,
    Base32,
    Base32Hex,
    Base16,
    Base2MSBF,
    Base2LSBF,
    Z85,
}

#[derive(Default, Debug)]
pub struct Options {
    pub base: Option<Base>,
    pub decode: bool,
    pub ignore_garbage: bool,
    pub wrap: Option<usize>,
}

impl Default for Options {
    fn default() -> Self {
        Self {
            base: None,
            decode: false,
            ignore_garbage: false,
            wrap: Some(76),
        }
    }
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
            Arg::Long("base64") => opts.base = Some(Base::Base64),
            Arg::Long("base64url") => opts.base = Some(Base::Base64Url),
            Arg::Long("base58") => opts.base = Some(Base::Base58),
            Arg::Long("base32") => opts.base = Some(Base::Base32),
            Arg::Long("base32hex") => opts.base = Some(Base::Base32Hex),
            Arg::Long("base16") => opts.base = Some(Base::Base16),
            Arg::Long("base2msbf") => opts.base = Some(Base::Base2MSBF),
            Arg::Long("base2lsbf") => opts.base = Some(Base::Base2LSBF),
            Arg::Short('d') | Arg::Long("decode") => opts.decode = true,
            Arg::Short('i') | Arg::Long("ignore-garbage") => opts.ignore_garbage = true,
            Arg::Short('E') | Arg::Long("show-ends") => {
                opts.show_ends = true;
            }
            Arg::Short('w')
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
        opts.show_ends = true;
    }

    Ok(ParseOutcome::Ok(opts, files))
}