#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include "tty.h"

#include "../config.h"

void TTYWrapper::reset() {
  tcsetattr(this->fdin, TCSANOW, &this->original_termios);
}

void TTYWrapper::close() {
  this->reset();
  fclose(this->fout);
  ::close(this->fdin);
}

void TTYWrapper::init(const char *tty_filename) {
  this->fdin = open(tty_filename, O_RDONLY);
  if (this->fdin < 0) {
    perror("Failed to open tty");
    exit(EXIT_FAILURE);
  }

  this->fout = fopen(tty_filename, "w");
  if (!this->fout) {
    perror("Failed to open tty");
    exit(EXIT_FAILURE);
  }

  if (setvbuf(this->fout, NULL, _IOFBF, 4096)) {
    perror("setvbuf");
    exit(EXIT_FAILURE);
  }

  if (tcgetattr(this->fdin, &this->original_termios)) {
    perror("tcgetattr");
    exit(EXIT_FAILURE);
  }

  struct termios new_termios = this->original_termios;

  /*
   * Disable all of
   * ICANON  Canonical input (erase and kill processing).
   * ECHO    Echo.
   * ISIG    Signals from control characters
   * ICRNL   Conversion of CR characters into NL
   */
  new_termios.c_iflag &= ~(ICRNL);
  new_termios.c_lflag &= ~(ICANON | ECHO | ISIG);

  if (tcsetattr(this->fdin, TCSANOW, &new_termios))
    perror("tcsetattr");

  this->getwinsz();

  this->setnormal();
}

void TTYWrapper::getwinsz() {
  struct winsize ws;
  if (ioctl(fileno(this->fout), TIOCGWINSZ, &ws) == -1) {
    this->maxwidth = 80;
    this->maxheight = 25;
  } else {
    this->maxwidth = ws.ws_col;
    this->maxheight = ws.ws_row;
  }
}

char TTYWrapper::getchar() {
  char ch;
  int size = read(this->fdin, &ch, 1);
  if (size < 0) {
    perror("error reading from tty");
    exit(EXIT_FAILURE);
  } else if (size == 0) {
    /* EOF */
    exit(EXIT_FAILURE);
  } else {
    return ch;
  }
}

int TTYWrapper::input_ready(int pending) {
  fd_set readfs;
  struct timeval tv = {0, pending ? (KEYTIMEOUT * 1000) : 0};
  FD_ZERO(&readfs);
  FD_SET(this->fdin, &readfs);
  select(this->fdin + 1, &readfs, NULL, NULL, &tv);
  return FD_ISSET(this->fdin, &readfs);
}

static void TTYWrapper_sgr(TTYWrapper *tty, int code) {
  tty->printf("%c%c%im", 0x1b, '[', code);
}

void TTYWrapper::setfg(int fg) {
  if (this->fgcolor != fg) {
    TTYWrapper_sgr(this, 30 + fg);
    this->fgcolor = fg;
  }
}

void TTYWrapper::setinvert() { TTYWrapper_sgr(this, 7); }

void TTYWrapper::setunderline() { TTYWrapper_sgr(this, 4); }

void TTYWrapper::setnormal() {
  TTYWrapper_sgr(this, 0);
  this->fgcolor = 9;
}

void TTYWrapper::setnowrap() { this->printf("%c%c?7l", 0x1b, '['); }

void TTYWrapper::setwrap() { this->printf("%c%c?7h", 0x1b, '['); }

void TTYWrapper::newline() { this->printf("%c%cK\n", 0x1b, '['); }

void TTYWrapper::clearline() { this->printf("%c%cK", 0x1b, '['); }

void TTYWrapper::setcol(int col) {
  this->printf("%c%c%iG", 0x1b, '[', col + 1);
}

void TTYWrapper::moveup(int i) { this->printf("%c%c%iA", 0x1b, '[', i); }

void TTYWrapper::printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(this->fout, fmt, args);
  va_end(args);
}

void TTYWrapper::flush() { fflush(this->fout); }

size_t TTYWrapper::getwidth() { return this->maxwidth; }

size_t TTYWrapper::getheight() { return this->maxheight; }
