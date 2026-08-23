#ifndef META_H
#define META_H

enum entry_placement { Outline, Inline };

struct help_entry {
  const char *opt;
  const char *desc;
  enum entry_placement placement;
};

void print_help(const char *usage, const char *desc,
                const struct help_entry *entries, const char *footer);
void print_version(const char *prog_name, const char *proj_name,
                   const char *ver, const char *authors);

#endif /* META_H */
