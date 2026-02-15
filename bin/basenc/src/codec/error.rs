use util::error::UtilError;

#[derive(Debug)]
pub enum CodecError {
    InvalidInput,
    InvalidInputMsg(String),
    AllocFailed(String, bool),
}

impl std::fmt::Display for CodecError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            CodecError::InvalidInput => write!(f, "invalid input"),
            CodecError::InvalidInputMsg(e) => write!(f, "invalid input {}", e),
            CodecError::AllocFailed(e, d) => write!(
                f,
                "failed to {} data: {}",
                if *d {
                    String::from("decode")
                } else {
                    String::from("encode")
                },
                e
            ),
        }
    }
}

impl From<CodecError> for UtilError {
    fn from(err: CodecError) -> Self {
        UtilError::Parse(err.to_string())
    }
}
