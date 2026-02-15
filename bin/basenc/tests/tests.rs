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
    assert_eq!(
        "MNXXK3TUNFXGOIDPOIQG433UEBRW65LOORUW4ZZAM52W4IDWNFXWYZLOMNSQ===="
            .as_bytes()
            .to_vec(),
        s
    );
}

#[test]
fn base16_encoder() {
    let opts = Options {
        base: Some(Base::Base16),
        ..Default::default()
    };
    let s = encode(b"help me eirin!!!", &opts).unwrap();
    assert_eq!("68656C70206D6520656972696E212121".as_bytes().to_vec(), s);
}

#[test]
fn base2msbf_encoder() {
    let opts = Options {
        base: Some(Base::Base2MSBF),
        ..Default::default()
    };
    let s = encode(b"KILL EVERYONE", &opts).unwrap();
    assert_eq!(
        "01001011010010010100110001001100001000000100010101010110010001010101001001011001010011110100111001000101"
            .as_bytes()
            .to_vec(),
        s
    );
}

#[test]
fn base2lsbf_encoder() {
    let opts = Options {
        base: Some(Base::Base2LSBF),
        ..Default::default()
    };
    let s = encode(b"KILL EVERYONE", &opts).unwrap();
    assert_eq!(
        "11010010100100100011001000110010000001001010001001101010101000100100101010011010111100100111001010100010"
            .as_bytes()
            .to_vec(),
        s
    );
}

#[test]
fn z85_encoder() {
    let opts = Options {
        base: Some(Base::Z85),
        ..Default::default()
    };
    let s = encode(b"fuckass shit", &opts).unwrap();
    assert_eq!("w{4Q0vrlGjB7/of".as_bytes().to_vec(), s);
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
    let s = decode(
        b"MNXXK3TUNFXGOIDPOIQG433UEBRW65LOORUW4ZZAM52W4IDWNFXWYZLOMNSQ====",
        &opts,
    )
    .unwrap();
    assert_eq!(
        "counting or not counting gun violence".as_bytes().to_vec(),
        s
    );
}

#[test]
fn base16_decoder() {
    let opts = Options {
        base: Some(Base::Base16),
        decode: true,
        ..Default::default()
    };
    let s = decode(b"68656C70206D6520656972696E212121", &opts).unwrap();
    assert_eq!("help me eirin!!!".as_bytes().to_vec(), s);
}

#[test]
fn base2msbf_decoder() {
    let opts = Options {
        base: Some(Base::Base2MSBF),
        decode: true,
        ..Default::default()
    };
    let s = decode(
        b"01001011010010010100110001001100001000000100010101010110010001010101001001011001010011110100111001000101",
        &opts,
    )
    .unwrap();
    assert_eq!("KILL EVERYONE".as_bytes().to_vec(), s);
}

#[test]
fn base2lsbf_decoder() {
    let opts = Options {
        base: Some(Base::Base2LSBF),
        decode: true,
        ..Default::default()
    };
    let s = decode(
        b"11010010100100100011001000110010000001001010001001101010101000100100101010011010111100100111001010100010",
        &opts,
    )
    .unwrap();
    assert_eq!("KILL EVERYONE".as_bytes().to_vec(), s);
}

#[test]
fn z85_decoder() {
    let opts = Options {
        base: Some(Base::Z85),
        decode: true,
        ..Default::default()
    };
    let s = decode(b"w{4Q0vrlGjB7/of", &opts).unwrap();
    assert_eq!("fuckass shit".as_bytes().to_vec(), s);
}
