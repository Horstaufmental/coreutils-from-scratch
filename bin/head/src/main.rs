use head::*;
use std::env;
use std::process;
use util::help::print_help;
use util::meta::print_version;

fn main() {
    let args: Vec<String> = env::args().skip(1).collect();

    match parse_args(&args) {
        Ok(ParseOutcome::Help) => {
            print_help(
                "Usage: head [OPTION]... [FILE]...",
                "Print the first 10 lines of each FILE to standard output.\n\
            With more than one FILE, precede each with a header giving the name.",
                &HELP_ENTRIES,
            );
            println!(
                "\nNUM may have a multiplier suffix:\n\
                    b 512, kB 1000, K 1024, MB 1000*1000,\n\
                    GB 1000*1000*1000, G 1024*1024*1024, and so on for T, P, E, Z, Y, R, Q.\n\
                    Binary prefixes can be used, too: KiB=K, MiB=M, and so on."
            )
        }
        Ok(ParseOutcome::Version) => print_version(&META),
        Ok(ParseOutcome::Ok(opts, files)) => {
            if let Err(e) = run(&opts, &files) {
                eprintln!("head: {}", e);
                process::exit(e.exit_code());
            }
        }
        Err(e) => {
            eprintln!("head: {}", e);
            eprintln!("Try 'head --help' for more information.");
            process::exit(1);
        }
    }
}
