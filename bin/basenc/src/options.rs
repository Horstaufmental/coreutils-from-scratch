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

#[derive(Debug)]
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
