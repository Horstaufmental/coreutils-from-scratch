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
use util::error::UtilError;

#[derive(Debug)]
pub enum CodecError {
    InvalidInput,
    InvalidInputMsg(String),
    AllocFailed(String, bool),
}

impl std::fmt::Display for CodecError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            CodecError::InvalidInput => write!(f, "invalid input"),
            CodecError::InvalidInputMsg(e) => write!(f, "invalid input {}", e),
            CodecError::AllocFailed(e, d) => write!(
                f,
                "failed to {} data: {}",
                if *d {
                    String::from("decode")
                } else {
                    String::from("encode")
                },
                e
            ),
        }
    }
}

impl From<CodecError> for UtilError {
    fn from(err: CodecError) -> Self {
        UtilError::Parse(err.to_string())
    }
}
