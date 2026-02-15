use crate::codec::error::CodecError;

pub static BASE16_ALPHABET: &[u8; 16] = b"0123456789ABCDEF";

const INVALID: u8 = 0x80;
fn build_decoding_table(alphabet: &[u8; 16]) -> [u8; 256] {
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