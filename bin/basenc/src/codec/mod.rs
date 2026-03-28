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
mod base16;
mod base2;
mod base32;
mod base58;
mod base64;
mod error;
mod z85;

use crate::options::{Base, Options};
use error::CodecError;

use once_cell::sync::Lazy;

pub fn encode(data: &[u8], opts: &Options) -> Result<Vec<u8>, CodecError> {
    match opts.base.unwrap() {
        Base::Base64 => base64::encode(data, base64::Base64Variant::Standard),
        Base::Base64Url => base64::encode(data, base64::Base64Variant::UrlSafe),
        Base::Base58 => base58::encode(data),
        Base::Base32 => base32::encode(data, base32::Base32Variant::Standard),
        Base::Base32Hex => base32::encode(data, base32::Base32Variant::Hex),
        Base::Base16 => base16::encode(data),
        Base::Base2MSBF => base2::encode(data, base2::Base2Variant::MSBFirst),
        Base::Base2LSBF => base2::encode(data, base2::Base2Variant::LSBFirst),
        Base::Z85 => z85::encode(data),
    }
}

pub fn decode(data: &[u8], opts: &Options) -> Result<Vec<u8>, CodecError> {
    match opts.base.unwrap() {
        Base::Base64 => base64::decode(data, base64::Base64Variant::Standard),
        Base::Base64Url => base64::decode(data, base64::Base64Variant::UrlSafe),
        Base::Base58 => base58::decode(data),
        Base::Base32 => base32::decode(data, base32::Base32Variant::Standard),
        Base::Base32Hex => base32::decode(data, base32::Base32Variant::Hex),
        Base::Base16 => base16::decode(data),
        Base::Base2MSBF => base2::decode(data, base2::Base2Variant::MSBFirst),
        Base::Base2LSBF => base2::decode(data, base2::Base2Variant::LSBFirst),
        Base::Z85 => z85::decode(data),
    }
}

static BASE16_TABLE: Lazy<[u8; 256]> =
    Lazy::new(|| base16::build_decoding_table(base16::BASE16_ALPHABET));
static Z85_TABLE: Lazy<[u8; 256]> = Lazy::new(|| z85::build_decoding_table(z85::Z85_ALPHABET));

pub fn decoding_table_for(opts: &Options) -> Option<&[u8; 256]> {
    match opts.base.unwrap() {
        Base::Base64 => Some(base64::decoding_table(base64::Base64Variant::Standard)),
        Base::Base64Url => Some(base64::decoding_table(base64::Base64Variant::UrlSafe)),
        Base::Base58 => Some(&base58::BASE58_DECODE_TABLE),
        Base::Base32 => Some(base32::decoding_table(base32::Base32Variant::Standard)),
        Base::Base32Hex => Some(base32::decoding_table(base32::Base32Variant::Hex)),
        Base::Base16 => Some(&*BASE16_TABLE),
        Base::Z85 => Some(&*Z85_TABLE),
        _ => None,
    }
}
