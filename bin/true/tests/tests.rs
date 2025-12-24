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
use r#true::{parse_args, ParseOutcome};
use std::process::Command;

#[test]
fn exit_status_is_success() {
    let output = Command::new("cargo")
        .args(["run", "-p", "true", "--quiet"])
        .output()
        .expect("Failed to execute command");

    assert!(output.status.success());
    assert_eq!(Some(0), output.status.code());
}

#[test]
fn help_short_circuits() {
    let args = vec!["--help".into()];
    assert!(matches!(parse_args(&args).unwrap(), ParseOutcome::Help));
}
