mod base16;
mod base2;
mod base32;
mod base58;
mod base64;
mod error;
mod z85;

use crate::options::{Base, Options};
use error::CodecError;

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
        Base::Base16 => base16::decode(data),
        Base::Base2MSBF => base2::decode(data, base2::Base2Variant::MSBFirst),
        Base::Base2LSBF => base2::decode(data, base2::Base2Variant::LSBFirst),
        Base::Z85 => z85::decode(data),
        _ => Ok(Vec::new()),
    }
}
