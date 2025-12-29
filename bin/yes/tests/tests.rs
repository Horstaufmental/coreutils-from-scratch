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
use yes::{parse_args, yes, ParseOutcome};

#[test]
fn default_string_is_y() {
    let mut out = Vec::new();
    yes(&Vec::new(), &mut out, true).unwrap();

    let output = String::from_utf8(out).unwrap();

    assert_eq!(output, "y\n");
}

#[test]
fn help_short_circuits() {
    let args = vec!["--help".into()];
    assert!(matches!(parse_args(&args).unwrap(), ParseOutcome::Help));
}
