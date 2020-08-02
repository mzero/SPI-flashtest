#ifndef _INCLUDE_HELODATA_H_
#define _INCLUDE_HELODATA_H_

#include <cstddef>
#include <cstdint>


class Print;  // forward def


// A "Helo" block is a block of data that contains a verifiable sequence of
// pseudo-random data. It can generated and written by this code, or you
// can write a file using a pyhon program and write that to a flash chip
// formatted with an SDFat file system.

union Helo {
public:
  static const size_t BLOCKSIZE = 512;    // matches file block size in SDFat

  static const uint32_t MAGIC1 = 0x6f6c6568;
  static const uint32_t MAGIC2 = 0x61746164;

  static const size_t WORDCOUNT = BLOCKSIZE/sizeof(uint32_t) - 3;


  struct {
    uint32_t magic1;
    uint32_t magic2;
    uint32_t blockNumber;
    uint32_t words[WORDCOUNT];
  } data;

  uint8_t bytes[BLOCKSIZE];


  void fill(uint32_t n);

  enum CheckResult {
    nothelo,
    good,
    bad
  };

  CheckResult check(Print* report) const;
  CheckResult check() const { return check(nullptr); }
};



#endif // _INCLUDE_HELODATA_H_
