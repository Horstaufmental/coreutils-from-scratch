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
use std::fmt;

#[derive(Debug)]
pub enum UtilError {
    Parse(String),
    Io { path: String, err: std::io::Error },
}

impl UtilError {
    pub fn exit_code(&self) -> i32 {
        1
    }
}

impl fmt::Display for UtilError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            UtilError::Parse(err) => write!(f, "{err}"),
            UtilError::Io { path, err } => {
                write!(f, "{}: {}", path, err)
            }
        }
    }
}
