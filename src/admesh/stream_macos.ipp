#pragma once
#ifdef __APPLE__

#include <assert.h>
#include <stdio.h>

// https://www.gsp.com/cgi-bin/man.cgi?section=3&topic=funopen

#include <assert.h>
#include <stdio.h>

#include <istream>
#include <ostream>

/*********************因环境原因未编译，未测试*********************/

struct IStreamFILE {

public:
  static FILE *new_from_stream(std::istream &stream, const char *mode) {
    auto tmpCookie = new (IStreamFILE); 
    tmpCookie->m_stream = &stream;
    return funopen(tmpCookie, IStreamFILE::read, IStreamFILE::write, IStreamFILE::seek,
                   IStreamFILE::close);
  }

private:
  static int read(void *__cookie, char *__buf, int __nbytes) {
    auto tmp = reinterpret_cast<IStreamFILE *>(__cookie);
    assert(tmp != nullptr);
    return tmp->m_stream->readsome(__buf, __nbytes);
  }

  static int write(void *__cookie, const char *__buf, int __nbytes) {
    assert(false);
    return 0;
  }

  static fpos_t seek(void *__cookie, fpos_t offset, int whence) {
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
    if (tmp->m_stream->seekg(offset, _w).good()) {
      return 0;
    } else {
      return -1;
    }
  }

  static int close(void *__cookie) {
    auto tmp = reinterpret_cast<IStreamFILE *>(__cookie);
    assert(tmp != nullptr);
    delete tmp;
    return 0;
  }

private:
  std::istream *m_stream;
};

struct OStreamFILE {
public:
  static FILE *new_from_stream(std::ostream &stream, const char *mode) {
    auto tmpCookie = new (OStreamFILE);
    tmpCookie->m_stream = &stream;
    return funopen(tmpCookie, OStreamFILE::read, OStreamFILE::write, OStreamFILE::seek,
                   OStreamFILE::close);
  }

private:
  static int read(void *__cookie, char *__buf, int __nbytes) { assert(false); return 0; }
  static int write(void *__cookie, const char *__buf, int __nbytes) {
    auto tmp = reinterpret_cast<OStreamFILE *>(__cookie);
    assert(tmp != nullptr);
    tmp->m_stream->write(__buf, __nbytes);
    if (tmp->m_stream->good()) {
      return __nbytes;
    }
    return 0;
  }
  static fpos_t seek(void *__cookie, fpos_t offset, int whence) {
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
    tmp->m_stream->seekp(offset, _w);
    if (tmp->m_stream->good()) {
      return 0;
    } else {
      return -1;
    }
  }

  static int close(void *__cookie) {
    auto tmp = reinterpret_cast<OStreamFILE *>(__cookie);
    assert(tmp != nullptr);
    delete tmp;
    return 0;
  }

private:
  std::ostream *m_stream;
};

#endif // __APPLE__