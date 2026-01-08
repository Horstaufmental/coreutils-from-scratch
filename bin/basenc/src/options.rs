#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Base {
    Base64,
    Base64Url,
    Base58,
    Base32,
    Base32Hex,
    Base16,
    Base2MSBF,
    Base2LSBF,
    Z85,
}

#[derive(Debug)]
pub struct Options {
    pub base: Option<Base>,
    pub decode: bool,
    pub ignore_garbage: bool,
    pub wrap: Option<usize>,
}

impl Default for Options {
    fn default() -> Self {
        Self {
            base: None,
            decode: false,
            ignore_garbage: false,
            wrap: Some(76),
        }
    }
}
