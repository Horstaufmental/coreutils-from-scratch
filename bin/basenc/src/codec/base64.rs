use crate::codec::error::CodecError;
use std::sync::OnceLock;

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum Base64Variant {
    Standard,
    UrlSafe,
}

pub static BASE64_ALPHABET: &[u8; 64] =
    b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
pub static BASE64URL_ALPHABET: &[u8; 64] =
    b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

const INVALID: u8 = 0x80;
fn build_decoding_table(alphabet: &[u8; 64]) -> [u8; 256] {
    let mut table = [INVALID; 256];
    for (i, &b) in alphabet.iter().enumerate() {
        table[b as usize] = i as u8;
    }
    table
}

static BASE64_DECODE_TABLE: OnceLock<[u8; 256]> = OnceLock::new();
static BASE64URL_DECODE_TABLE: OnceLock<[u8; 256]> = OnceLock::new();

fn decoding_table(variant: Base64Variant) -> &'static [u8; 256] {
    match variant {
        Base64Variant::Standard => {
            BASE64_DECODE_TABLE.get_or_init(|| build_decoding_table(BASE64_ALPHABET))
        }
        Base64Variant::UrlSafe => {
            BASE64URL_DECODE_TABLE.get_or_init(|| build_decoding_table(BASE64URL_ALPHABET))
        }
    }
}

pub fn encode(data: &[u8], variant: Base64Variant) -> Result<Vec<u8>, CodecError> {
    let in_len = data.len();
    let out_len = 4 * ((in_len + 2) / 3);

    let mut out = Vec::with_capacity(out_len);

    let mut i = 0;
    while i < in_len {
        let octet_a = data.get(i).copied().unwrap_or(0);
        i += 1;
        let octet_b = data.get(i).copied().unwrap_or(0);
        i += 1;
        let octet_c = data.get(i).copied().unwrap_or(0);
        i += 1;

        let triple = ((octet_a as u32) << 16) | ((octet_b as u32) << 8) | (octet_c as u32);

        if variant == Base64Variant::UrlSafe {
            out.push(BASE64URL_ALPHABET[((triple >> 18) & 0x3F) as usize]);
            out.push(BASE64URL_ALPHABET[((triple >> 12) & 0x3F) as usize]);
            out.push(BASE64URL_ALPHABET[((triple >> 6) & 0x3F) as usize]);
            out.push(BASE64URL_ALPHABET[(triple & 0x3F) as usize]);
        } else {
            out.push(BASE64_ALPHABET[((triple >> 18) & 0x3F) as usize]);
            out.push(BASE64_ALPHABET[((triple >> 12) & 0x3F) as usize]);
            out.push(BASE64_ALPHABET[((triple >> 6) & 0x3F) as usize]);
            out.push(BASE64_ALPHABET[(triple & 0x3F) as usize]);
        }
    }

    match in_len % 3 {
        1 => {
            out[out_len - 1] = b'=';
            out[out_len - 2] = b'=';
        }
        2 => {
            out[out_len - 1] = b'=';
        }
        _ => {}
    }

    Ok(out)
}

pub fn decode(data: &[u8], variant: Base64Variant) -> Result<Vec<u8>, CodecError> {
    let table = decoding_table(variant);

    let mut out = Vec::with_capacity(data.len() / 4 * 3);

    let mut i = 0;
    while i < data.len() {
        let a = table[data[i] as usize] as u32;
        i += 1;
        let b = table[data[i] as usize] as u32;
        i += 1;
        let c = table[data[i] as usize] as u32;
        i += 1;
        let d = table[data[i] as usize] as u32;
        i += 1;

        let triple = (a << 18) | (b << 12) | (c << 6) | d;

        out.push(((triple >> 16) & 0xFF) as u8);
        if data[i - 2] != b'=' {
            out.push(((triple >> 8) & 0xFF) as u8);
        }
        if data[i - 1] != b'=' {
            out.push((triple & 0xFF) as u8);
        }
    }

    Ok(out)
}
