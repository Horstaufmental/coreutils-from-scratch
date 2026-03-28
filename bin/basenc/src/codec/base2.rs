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
use crate::codec::error::CodecError;

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum Base2Variant {
    MSBFirst,
    LSBFirst,
}

pub fn encode(data: &[u8], variant: Base2Variant) -> Result<Vec<u8>, CodecError> {
    let mut out = Vec::with_capacity(data.len() * 8);

    let mut i = 0;
    while i < data.len() {
        let b = data[i] as usize;
        if let Base2Variant::MSBFirst = variant {
            for bit in (0..8).rev() {
                let bit_val = (b >> bit) & 1;
                out.push(if bit_val == 1 { b'1' } else { b'0' });
            }
        } else {
            for bit in 0..8 {
                let bit_val = (b >> bit) & 1;
                out.push(if bit_val == 1 { b'1' } else { b'0' });
            }
        }

        i += 1;
    }

    Ok(out)
}

pub fn decode(data: &[u8], variant: Base2Variant) -> Result<Vec<u8>, CodecError> {
    if data.len() % 8 != 0 {
        return Err(CodecError::InvalidInput);
    }

    let mut out = Vec::with_capacity(data.len() / 8);

    let mut i = 0;
    while i < data.len() {
        let mut b = 0u8;
        for bit in 0..8 {
            let c = data[i + bit];
            if c != b'0' && c != b'1' {
                return Err(CodecError::InvalidInput);
            }
            match variant {
                Base2Variant::MSBFirst => b = (b << 1) | (c - b'0'),
                Base2Variant::LSBFirst => b |= (c - b'0') << bit,
            }
        }
        out.push(b);

        i += 8;
    }

    Ok(out)
}
