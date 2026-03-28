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

pub static BASE58_ALPHABET: &[u8; 58] =
    b"123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

const INVALID: u8 = 0x80;

pub static BASE58_DECODE_TABLE: [u8; 256] = {
    let mut table = [INVALID; 256];
    let mut i = 0;
    while i < 58 {
        table[BASE58_ALPHABET[i] as usize] = i as u8;
        i += 1;
    }
    table
};

pub fn encode(data: &[u8]) -> Result<Vec<u8>, CodecError> {
    let mut zeros = 0;
    while zeros < data.len() && data[zeros] == 0 {
        zeros += 1;
    }

    let size = data.len() * 138 / 100 + 1;
    let mut buf = vec![0u8; size];

    let mut high = 0usize;

    for &byte in &data[zeros..] {
        let mut carry = byte as u32;
        let mut j = 0usize;

        for k in (0..size).rev() {
            if carry == 0 && k >= high {
                break;
            }

            carry += 256 * buf[k] as u32;
            buf[k] = (carry % 58) as u8;
            carry /= 58;
            j += 1;
        }

        high = j;
    }

    let mut p = 0;
    while p < size && buf[p] == 0 {
        p += 1;
    }

    let mut out = Vec::with_capacity(zeros + (size - p));
    out.extend(std::iter::repeat(b'1').take(zeros));

    for &digit in &buf[p..] {
        out.push(BASE58_ALPHABET[digit as usize]);
    }

    Ok(out)
}

pub fn decode(data: &[u8]) -> Result<Vec<u8>, CodecError> {
    let mut zeros = 0;
    while zeros < data.len() && data[zeros] == b'1' {
        zeros += 1;
    }

    let size = data.len() * 733 / 1000 + 1;
    let mut buf = vec![0u8; size];

    let mut high = 0usize;
    for &byte in &data[zeros..] {
        let val = base58_value(byte).ok_or(CodecError::InvalidInput)? as u32;

        let mut carry = val;
        let mut j = 0usize;

        for k in (0..size).rev() {
            if carry == 0 && j >= high {
                break;
            }

            carry += 58 * buf[k] as u32;
            buf[k] = (carry % 256) as u8;
            carry /= 256;
            j += 1;
        }

        high = j;
    }

    let mut p = 0;
    while p < size && buf[p] == 0 {
        p += 1;
    }

    let mut out = Vec::with_capacity(zeros + (size - p));
    out.extend(std::iter::repeat(0u8).take(zeros));

    out.extend_from_slice(&buf[p..]);

    Ok(out)
}

#[inline]
fn base58_value(b: u8) -> Option<u8> {
    let v = BASE58_DECODE_TABLE[b as usize];
    if v & INVALID != 0 {
        None
    } else {
        Some(v)
    }
}
