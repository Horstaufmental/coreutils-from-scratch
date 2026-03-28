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

pub static BASE16_ALPHABET: &[u8; 16] = b"0123456789ABCDEF";

const INVALID: u8 = 0x80;
pub fn build_decoding_table(alphabet: &[u8; 16]) -> [u8; 256] {
    let mut table = [INVALID; 256];
    for (i, &b) in alphabet.iter().enumerate() {
        table[b as usize] = i as u8;
    }
    table
}

pub fn encode(data: &[u8]) -> Result<Vec<u8>, CodecError> {
    let mut out = Vec::with_capacity(data.len() * 2);

    let mut i = 0;
    while i < data.len() {
        out.push(BASE16_ALPHABET[((data[i] >> 4) & 0xF) as usize]);
        out.push(BASE16_ALPHABET[(data[i] & 0xF) as usize]);
        i += 1;
    }

    Ok(out)
}

pub fn decode(data: &[u8]) -> Result<Vec<u8>, CodecError> {
    if data.len() % 2 != 0 {
        return Err(CodecError::InvalidInput);
    }

    let mut out = Vec::with_capacity(data.len() / 2);

    let table = build_decoding_table(BASE16_ALPHABET);

    let mut i = 0;
    while i < data.len() {
        let hi = table[data[i] as usize];
        let lo = table[data[i + 1] as usize];

        if ((hi | lo) & INVALID) != 0 {
            return Err(CodecError::InvalidInput);
        }

        out.push((hi << 4) | lo);
        i += 2;
    }

    Ok(out)
}
