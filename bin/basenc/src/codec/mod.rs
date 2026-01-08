mod base64;
mod error;

use crate::options::{Base, Options};
use error::CodecError;

pub fn encode(data: &[u8], opts: &Options) -> Result<Vec<u8>, CodecError> {
    match opts.base.unwrap() {
        Base::Base64 => base64::encode(data, base64::Base64Variant::Standard),
        Base::Base64Url => base64::encode(data, base64::Base64Variant::UrlSafe),
        _ => Ok(Vec::new()),
    }
}

pub fn decode(data: &[u8], opts: &Options) -> Result<Vec<u8>, CodecError> {
    match opts.base.unwrap() {
        Base::Base64 => base64::decode(data, base64::Base64Variant::Standard),
        Base::Base64Url => base64::decode(data, base64::Base64Variant::UrlSafe),
        _ => Ok(Vec::new()),
    }
}
