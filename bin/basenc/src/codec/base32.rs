use crate::codec::error::CodecError;
use std::sync::OnceLock;

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum Base32Variant {
    Standard,
    Hex,
}

pub static BASE32_ALPHABET: &[u8; 32] = b"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
pub static BASE32HEX_ALPHABET: &[u8; 32] = b"0123456789ABCDEFGHIJKLMNOPQRSTUV";

const INVALID: u8 = 0x80;
fn build_decoding_table(alphabet: &[u8; 32]) -> [u8; 256] {
    let mut table = [INVALID; 256];
    for (i, &b) in alphabet.iter().enumerate() {
        table[b as usize] = i as u8;
    }
    table
}

static BASE32_DECODE_TABLE: OnceLock<[u8; 256]> = OnceLock::new();
static BASE32HEX_DECODE_TABLE: OnceLock<[u8; 256]> = OnceLock::new();

fn decoding_table(variant: Base32Variant) -> &'static [u8; 256] {
    match variant {
        Base32Variant::Standard => {
            BASE32_DECODE_TABLE.get_or_init(|| build_decoding_table(BASE32_ALPHABET))
        }
        Base32Variant::Hex => {
            BASE32HEX_DECODE_TABLE.get_or_init(|| build_decoding_table(BASE32HEX_ALPHABET))
        }
    }
}

pub fn encode(data: &[u8], variant: Base32Variant) -> Result<Vec<u8>, CodecError> {
    let out_len = ((data.len() + 4) / 5) * 8;
    let alphabet = match variant {
        Base32Variant::Standard => BASE32_ALPHABET,
        Base32Variant::Hex => BASE32HEX_ALPHABET,
    };

    let mut out = Vec::with_capacity(out_len);

    let mut i = 0;
    while i < data.len() {
        let mut buffer = 0u64;
        let bytes = std::cmp::min(5, data.len() - i);

        for _ in 0..bytes {
            buffer <<= 8;
            buffer |= data[i] as u64;
            i += 1;
        }
        buffer <<= (5 - bytes) * 8;
        let out_count = match bytes {
            1 => 2,
            2 => 4,
            3 => 5,
            4 => 7,
            5 => 8,
            _ => 0,
        };

        for j in 0..8 {
            if j < out_count {
                let shift = 35 - 5 * j;
                out.push(alphabet[((buffer >> shift) & 0x1F) as usize]);
            } else {
                out.push(b'=');
            }
        }
    }

    Ok(out)
}

pub fn decode(data: &[u8], variant: Base32Variant) -> Result<Vec<u8>, CodecError> {
    let in_len = data.len();
    if in_len % 8 != 0 {
        return Err(CodecError::InvalidInput);
    }

    let mut final_len = in_len / 8 * 5;
    if in_len > 0 && data[in_len - 1] == b'=' {
        final_len -= 1;
    }
    if in_len > 0 && data[in_len - 2] == b'=' {
        final_len -= 1;
    }
    if in_len > 0 && data[in_len - 3] == b'=' {
        final_len -= 1;
    }
    if in_len > 0 && data[in_len - 4] == b'=' {
        final_len -= 1;
    }

    let table = decoding_table(variant);

    let mut out = Vec::with_capacity(final_len);

    let mut i = 0;
    while i < in_len {
        let mut buffer = 0u64;
        let mut valid_bits = 40;

        for _ in 0..8 {
            let c = data[i];
            i += 1;

            if c == b'=' {
                buffer <<= 5;
                valid_bits -= 5;
            } else {
                let v = table[c as usize];
                if v == INVALID {
                    return Err(CodecError::InvalidInput);
                }
                buffer = (buffer << 5) | v as u64;
            }
        }

        let bytes = valid_bits / 8;
        for k in 0..bytes {
            let shift = 32 - 8 * k;
            out.push(((buffer >> shift) & 0xFF) as u8);
        }
    }

    Ok(out)
}
