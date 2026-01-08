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
    let s = decode(b"mVsZWFzZSB0aGUgbGlzdCE=", &opts).unwrap();
    assert_eq!("release the list!".as_bytes().to_vec(), s);
}
