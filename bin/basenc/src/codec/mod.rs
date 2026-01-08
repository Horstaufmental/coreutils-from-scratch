mod base64;
mod base58;
mod base32;
mod error;

use crate::options::{Base, Options};
use error::CodecError;

pub fn encode(data: &[u8], opts: &Options) -> Result<Vec<u8>, CodecError> {
    match opts.base.unwrap() {
        Base::Base64 => base64::encode(data, base64::Base64Variant::Standard),
        Base::Base64Url => base64::encode(data, base64::Base64Variant::UrlSafe),
        Base::Base58 => base58::encode(data),
        Base::Base32 => base32::encode(data, base32::Base32Variant::Standard),
        Base::Base32Hex => base32::encode(data, base32::Base32Variant::Hex),
        _ => Ok(Vec::new()),
    }
}

pub fn decode(data: &[u8], opts: &Options) -> Result<Vec<u8>, CodecError> {
    match opts.base.unwrap() {
        Base::Base64 => base64::decode(data, base64::Base64Variant::Standard),
        Base::Base64Url => base64::decode(data, base64::Base64Variant::UrlSafe),
        Base::Base58 => base58::decode(data),
        Base::Base32 => base32::decode(data, base32::Base32Variant::Standard),
        Base::Base32Hex => base32::decode(data, base32::Base32Variant::Hex),
        _ => Ok(Vec::new()),
    }
}
