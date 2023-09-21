#pragma once

#ifdef __linux__
#include <assert.h>
#include <stdio.h>

#include <istream>
#include <ostream>

struct IStreamFILE {

public:
  static FILE *new_from_stream(std::istream &stream, const char *mode) {
    auto tmpCookie = new (IStreamFILE);
    tmpCookie->m_stream = &stream;
    return fopencookie(tmpCookie, mode, new_cookie());
  }

private:
  static __ssize_t read(void *__cookie, char *__buf, size_t __nbytes) {
    auto tmp = reinterpret_cast<IStreamFILE *>(__cookie);
    assert(tmp != nullptr);
    return tmp->m_stream->readsome(__buf, __nbytes);
  }

  static __ssize_t write(void *__cookie, const char *__buf, size_t __nbytes) {
    assert(false);
    return 0;
  }

  static int seek(void *__cookie, off64_t *offset, int whence) {
    auto tmp = reinterpret_cast<IStreamFILE *>(__cookie);
    assert(tmp != nullptr);

    std::ios::seekdir _w;
    if (whence == SEEK_SET) {
      _w = std::ios::beg;
    } else if (whence == SEEK_END) {
      _w = std::ios::end;
    } else if (whence == SEEK_CUR) {
      _w = std::ios::cur;
    } else {
      return -1;
    }
    tmp->m_stream->seekg(*offset, _w);
    return 0;
  }

  static int close(void *__cookie) {
    auto tmp = reinterpret_cast<IStreamFILE *>(__cookie);
    assert(tmp != nullptr);
    delete tmp;
    return 0;
  }

  static cookie_io_functions_t new_cookie(void) {
    return cookie_io_functions_t{.read = IStreamFILE::read,
                                 .write = IStreamFILE::write,
                                 .seek = IStreamFILE::seek,
                                 .close = IStreamFILE::close};
  }

private:
  std::istream *m_stream;
};

struct OStreamFILE {
public:
  static FILE *new_from_stream(std::ostream &stream, const char *mode) {
    auto tmpCookie = new (OStreamFILE);
    tmpCookie->m_stream = &stream;
    return fopencookie(tmpCookie, mode, new_cookie());
  }

private:
  static __ssize_t read(void *__cookie, char *__buf, size_t __nbytes) {
    assert(false);
    return 0;
  }
  static __ssize_t write(void *__cookie, const char *__buf, size_t __nbytes) {
    auto tmp = reinterpret_cast<OStreamFILE *>(__cookie);
    assert(tmp != nullptr);
    tmp->m_stream->write(__buf, __nbytes);
    if (tmp->m_stream->good()) {
      return __nbytes;
    }
    return 0;
  }
  static int seek(void *__cookie, off64_t *offset, int whence) {
    auto tmp = reinterpret_cast<OStreamFILE *>(__cookie);
    assert(tmp != nullptr);

    std::ios::seekdir _w;
    if (whence == SEEK_SET) {
      _w = std::ios::beg;
    } else if (whence == SEEK_END) {
      _w = std::ios::end;
    } else if (whence == SEEK_CUR) {
      _w = std::ios::cur;
    } else {
      return -1;
    }
    tmp->m_stream->seekp(*offset, _w);
    return 0;
  }

  static int close(void *__cookie) {
    auto tmp = reinterpret_cast<OStreamFILE *>(__cookie);
    assert(tmp != nullptr);
    delete tmp;
    return 0;
  }

  static cookie_io_functions_t new_cookie(void) {
    return cookie_io_functions_t{.read = OStreamFILE::read,
                                 .write = OStreamFILE::write,
                                 .seek = OStreamFILE::seek,
                                 .close = OStreamFILE::close};
  }

private:
  std::ostream *m_stream;
};

#endif // __linux__
