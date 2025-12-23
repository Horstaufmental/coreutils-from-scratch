/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of coreutils-rs from scratch.
 * Copyright (c) 2025 Horstaufmental
 *
 * coreutils-rs from scratch is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * coreutils-rs from scratch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */
pub struct HelpEntry {
    pub opt: &'static str,
    pub desc: &'static str,
}

pub fn print_help(usage: &str, description: &str, entries: &[HelpEntry]) {
    println!("{usage}");
    println!("{description}");

    let maxlen = entries.iter().map(|e| e.opt.len()).max().unwrap_or(0);

    for e in entries {
        println!("  {:<width$}  {}", e.opt, e.desc, width = maxlen);
    }
}
