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
use crate::codec::CodecError;

pub static Z85_ALPHABET: &[u8; 85] =
    b"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";

const INVALID: u8 = 0x80;
pub fn build_decoding_table(alphabet: &[u8; 85]) -> [u8; 256] {
    let mut table = [INVALID; 256];
    for (i, &b) in alphabet.iter().enumerate() {
        table[b as usize] = i as u8;
    }
    table
}

pub fn encode(data: &[u8]) -> Result<Vec<u8>, CodecError> {
    if data.len() % 4 != 0 {
        return Err(CodecError::InvalidInputMsg(
            "(length must be multiple of 4 characters)".to_string(),
        ));
    }

    let mut out = Vec::with_capacity((data.len() * 5) / 4);

    for chunk in data.chunks_exact(4) {
        let mut value = u32::from_be_bytes(chunk.try_into().unwrap());

        let mut block = [0u8; 5];
        for k in (0..5).rev() {
            block[k] = Z85_ALPHABET[(value % 85) as usize];
            value /= 85;
        }

        out.extend_from_slice(&block);
    }

    Ok(out)
}

pub fn decode(data: &[u8]) -> Result<Vec<u8>, CodecError> {
    if data.len() % 5 != 0 {
        // i dont get why they dont also print the message if they did it for encoder
        // and at this point im too afraid to ask
        return Err(CodecError::InvalidInput);
    }

    let mut out = Vec::with_capacity((data.len() * 4) / 5);
    let table = build_decoding_table(Z85_ALPHABET);

    for chunk in data.chunks_exact(5) {
        let mut value = 0u32;

        for &c in chunk {
            let v = table[c as usize];
            if v == 0xFF {
                return Err(CodecError::InvalidInput);
            }
            value = value * 85 + v as u32;
        }

        out.extend_from_slice(&value.to_be_bytes());
    }

    Ok(out)
}
