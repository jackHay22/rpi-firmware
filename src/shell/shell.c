/*
 * (C) Jack Hay, Apr 2021
 */

#include "shell.h"
#include "../display/console.h"

void prompt() {
  write_str(">");
}

/**
 * The main kernel mode shell
 * @param  argc arg count
 * @param  argv arg vec
 * @return      exit status
 */
int shell_main(int argc, char **argv) {
  write_strln("aarch64 kernel mode");
  write_strln("starting kernel shell");
}
