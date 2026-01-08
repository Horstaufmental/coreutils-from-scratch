use basenc::{decode, encode, Base, Options};

/* ENCODER TESTS */

#[test]
fn base64_encoder() {
    let opts = Options {
        base: Some(Base::Base64),
        ..Default::default()
    };
    let s = encode(b"hello world", &opts).unwrap();
    assert_eq!("aGVsbG8gd29ybGQ=".as_bytes().to_vec(), s);
}

#[test]
fn base64url_encoder() {
    let opts = Options {
        base: Some(Base::Base64Url),
        ..Default::default()
    };
    let s = encode(b"release the list!", &opts).unwrap();
    assert_eq!("cmVsZWFzZSB0aGUgbGlzdCE=".as_bytes().to_vec(), s);
}

#[test]
fn base58_encoder() {
    let opts = Options {
        base: Some(Base::Base58),
        ..Default::default()
    };
    let s = encode(b"im gonna pop your cherry", &opts).unwrap();
    assert_eq!("AcSgPcDjP5pVyLUnyNGPpNNQCYe1xe8Vn".as_bytes().to_vec(), s);
}

#[test]
fn base32_encoder() {
    let opts = Options {
        base: Some(Base::Base32),
        ..Default::default()
    };
    let s = encode(b"counting or not counting gun violence", &opts).unwrap();
    assert_eq!("MNXXK3TUNFXGOIDPOIQG433UEBRW65LOORUW4ZZAM52W4IDWNFXWYZLOMNSQ====".as_bytes().to_vec(), s);
}

/* DECODER TESTS */

#[test]
fn base64_decoder() {
    let opts = Options {
        base: Some(Base::Base64),
        decode: true,
        ..Default::default()
    };
    let s = decode(b"aGVsbG8gd29ybGQ=", &opts).unwrap();
    assert_eq!("hello world".as_bytes().to_vec(), s);
}

#[test]
fn base64url_decoder() {
    let opts = Options {
        base: Some(Base::Base64Url),
        decode: true,
        ..Default::default()
    };
    let s = decode(b"cmVsZWFzZSB0aGUgbGlzdCE=", &opts).unwrap();
    assert_eq!("release the list!".as_bytes().to_vec(), s);
}

#[test]
fn base58_decoder() {
    let opts = Options {
        base: Some(Base::Base58),
        decode: true,
        ..Default::default()
    };
    let s = decode(b"AcSgPcDjP5pVyLUnyNGPpNNQCYe1xe8Vn", &opts).unwrap();
    assert_eq!("im gonna pop your cherry".as_bytes().to_vec(), s);
}

#[test]
fn base32_decoder() {
    let opts = Options {
        base: Some(Base::Base32),
        decode: true,
        ..Default::default()
    };
    let s = decode(b"MNXXK3TUNFXGOIDPOIQG433UEBRW65LOORUW4ZZAM52W4IDWNFXWYZLOMNSQ====", &opts).unwrap();
    assert_eq!("counting or not counting gun violence".as_bytes().to_vec(), s);
}