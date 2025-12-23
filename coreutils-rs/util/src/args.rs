#[derive(Debug)]
pub enum Arg<'a> {
    Short(char),
    Long(&'a str),
    LongWithValue(&'a str, &'a str),
    Value(&'a str),
    EndOfOptions,
}

pub struct ArgIter<'a> {
    args: std::slice::Iter<'a, String>,
    short_buf: Option<std::str::Chars<'a>>,
}

impl<'a> ArgIter<'a> {
    pub fn new(args: &'a [String]) -> Self {
        Self {
            args: args.iter(),
            short_buf: None,
        }
    }
}

impl<'a> Iterator for ArgIter<'a> {
    type Item = Arg<'a>;

    fn next(&mut self) -> Option<Self::Item> {
        if let Some(ref mut chars) = self.short_buf {
            if let Some(c) = chars.next() {
                return Some(Arg::Short(c));
            }
            self.short_buf = None;
        }

        let arg = self.args.next()?;

        if arg == "--" {
            return Some(Arg::EndOfOptions);
        }

        if let Some(rest) = arg.strip_prefix("--") {
            if let Some((k, v)) = rest.split_once('=') {
                return Some(Arg::LongWithValue(k, v));
            }
            return Some(Arg::Long(rest));
        }

        if let Some(rest) = arg.strip_prefix('-') {
            self.short_buf = Some(rest.chars());
            return self.next();
        }

        Some(Arg::Value(arg))
    }
}
