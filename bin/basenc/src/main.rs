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
use cat::*;
use std::env;
use std::process;
use util::help::print_help;
use util::meta::print_version;

fn main() {
    let args: Vec<String> = env::args().skip(1).collect();

    match parse_args(&args) {
        Ok(ParseOutcome::Help) => {
            print_help(
                "Usage: cat [OPTION]... [FILE]...",
                "Concatenate FILE(s) to standard output.\n\
                 With no FILE, or when FILE is -, read standard input.\n",
                &HELP_ENTRIES,
            );
        }
        Ok(ParseOutcome::Version) => {
            print_version(&META);
        }
        Ok(ParseOutcome::Ok(opts, files)) => {
            if let Err(e) = run(&opts, &files) {
                eprintln!("cat: {}", e);
                process::exit(e.exit_code());
            }
        }
        Err(e) => {
            eprintln!("cat: {}", e);
            eprintln!("Try 'cat --help' for more information.");
            process::exit(1);
        }
    }
}
