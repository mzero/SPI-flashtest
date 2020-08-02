#include "helodata.h"

#include <Print.h>


namespace {

  uint32_t sequenceStep(uint32_t x) {
    uint64_t x_ = static_cast<uint64_t>(x);

    x_ = (x_ * 1815976680ULL) % 4294967291ULL;
      // from "Tables of Linear Congruential Generators of Different Sizes and
      // Good Lattice Structure" - Peter L'Ecuyer, Mathematics of Computation,
      // Volume 68, Number 225, January 1999


    return static_cast<uint32_t>(x_);
  }

}


void Helo::fill(uint32_t n) {
  data.magic1 = MAGIC1;
  data.magic2 = MAGIC2;
  data.blockNumber = n;

  uint32_t x = data.blockNumber;
  for (size_t i = 0; i < WORDCOUNT; ++i) {
    x = sequenceStep(x);
    data.words[i] = x;
  }
}

Helo::CheckResult Helo::check(Print* report) const {
  if (data.magic1 != MAGIC1 || data.magic2 != MAGIC2)
    return nothelo;

  int errorCount = 0;

  uint32_t x = data.blockNumber;
  for (size_t i = 0; i < WORDCOUNT; ++i) {
    x = sequenceStep(x);
    if (data.words[i] != x) {
      errorCount += 1;
      if (report && errorCount == 1)
        report->printf(
          "helo error: block %5d, word %3d, expected %08x, actual %08x\n",
          data.blockNumber, i, x, data.words[i]);
      // break;
    }
  }

  if (report && errorCount > 1)
    report->printf(
      "                       and %d errors\n", errorCount);
  return errorCount == 0 ? good : bad;
}
