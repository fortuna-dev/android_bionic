/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int getpt(void) {
  return posix_openpt(O_RDWR|O_NOCTTY);
}

int grantpt(int) {
  return 0;
}

int posix_openpt(int flags) {
  return open("/dev/ptmx", flags);
}

char* ptsname(int fd) {
  static char buf[64];
  return ptsname_r(fd, buf, sizeof(buf)) == 0 ? buf : NULL;
}

int ptsname_r(int fd, char* buf, size_t len) {
  if (buf == NULL) {
    errno = EINVAL;
    return errno;
  }

  unsigned int pty_num;
  if (ioctl(fd, TIOCGPTN, &pty_num) != 0) {
    errno = ENOTTY;
    return errno;
  }

  if (snprintf(buf, len, "/dev/pts/%u", pty_num) >= static_cast<int>(len)) {
    errno = ERANGE;
    return errno;
  }

  return 0;
}

char* ttyname(int fd) {
  static char buf[64];
  return ttyname_r(fd, buf, sizeof(buf)) == 0 ? buf : NULL;
}

int ttyname_r(int fd, char* buf, size_t len) {
  if (buf == NULL) {
    errno = EINVAL;
    return errno;
  }

  if (!isatty(fd)) {
    return errno;
  }

  char path[64];
  snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);

  ssize_t count = readlink(path, buf, len);
  if (count == -1) {
    return errno;
  }
  if (static_cast<size_t>(count) == len) {
    errno = ERANGE;
    return errno;
  }
  buf[count] = '\0';
  return 0;
}

int unlockpt(int fd) {
  int unlock = 0;
  return ioctl(fd, TIOCSPTLCK, &unlock);
}
