use util::args::{Arg, ArgIter};

#[test]
fn parses_short_clusters() {
    let v = vec!["-abc".into()];
    let args: Vec<_> = ArgIter::new(&v).collect();
    assert!(matches!(arg[0], Arg::Short('a')));
    assert!(matches!(arg[1], Arg::Short('b')));
    assert!(matches!(arg[2], Arg::Short('c')));
}
