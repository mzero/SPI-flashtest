#include <initializer_list>
#include <Adafruit_SPIFlash.h>

#include "helodata.h"


namespace {

  // ===
  // === Utilities
  // ===


  void fatal(const char* msg) {
    Serial.print("\n[FATAL] ");
    Serial.println(msg);
    while (true) ;
  }

  // ===
  // === Flash device
  // ===


  // On-board external flash (QSPI or SPI) macros should already
  // defined in your board variant if supported
  // - EXTERNAL_FLASH_USE_QSPI
  // - EXTERNAL_FLASH_USE_CS/EXTERNAL_FLASH_USE_SPI
  #if defined(EXTERNAL_FLASH_USE_QSPI)
  Adafruit_FlashTransport_QSPI flashTransport;
  #elif defined(EXTERNAL_FLASH_USE_SPI)
  Adafruit_FlashTransport_SPI flashTransport(
    EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);
  #else
  #error No QSPI/SPI flash are defined on your board variant.h !
  #endif

  Adafruit_SPIFlash flash(&flashTransport);


  const std::initializer_list<SPIFlash_Device_t> additional_devices = {
    S25FL064L,
  };

  const size_t MAX_TEST_SIZE = 1024 * 1024; // just test first 1MB
  size_t testSize;


  bool flashBegin() {
    if (!flash.begin(additional_devices.begin(), additional_devices.size()))
      return false;

    testSize = min(flash.size(), MAX_TEST_SIZE);

    Serial.print("JEDEC ID:   "); Serial.println(flash.getJEDECID(), HEX);
    Serial.print("Flash size: "); Serial.println(flash.size());
    Serial.print("Test size:  "); Serial.println(testSize);
    return true;
  }



  bool chipErased = false;

  // ===
  // === Flash routines
  // ===


  void eraseFlash() {
    if (!flash.eraseChip())
      fatal("flash erase failed");

    flash.waitUntilReady();
    Serial.println("flash erased");

    chipErased = true;
  }


  void writeTestData() {
    Helo helo;

    int count = 0;


    for (uint32_t addr = 0; addr < testSize; addr += sizeof(helo)) {
      helo.fill(count);

      if (!chipErased && addr % SFLASH_SECTOR_SIZE == 0)
        flash.eraseSector(addr / SFLASH_SECTOR_SIZE);
      flash.writeBuffer(addr, helo.bytes, sizeof(helo));
      count += 1;

      if (count % 200 == 0) Serial.println();
      if (count % 20 == 0) Serial.printf("%d..", count);
      yield();
    }

    chipErased = false;
    Serial.printf("\ntest data written to %d blocks\n", count);
  }

  void verifyTestData() {
    Helo helo;

    int count = 0;
    int heloCount = 0;
    int heloBad = 0;

    for (uint32_t addr = 0; addr < testSize; addr += sizeof(helo)) {
      flash.readBuffer(addr, helo.bytes, sizeof(helo));
      switch (helo.check(&Serial)) {
        case Helo::bad:   heloBad += 1;  // fall through
        case Helo::good:  heloCount += 1;  // fall through
        default:          count += 1;
      }

      if (count % 200 == 0) Serial.println();
      if (count % 20 == 0) Serial.printf("%d..", count);
      yield();
    }

    Serial.printf("\ntested %d blocks, %d were Helo data blocks, %d failed\n",
      count, heloCount, heloBad);
  }


  #define USE_CLOCK_CONTROLS
    // this requires a fork'd version of Adafruit_SPIFlash to supply the
    // Adafruit_FlashTransport_SPI::getClockSpeed function
  #ifdef USE_CLOCK_CONTROLS

    uint32_t original_clock_hz;

    void noteOriginalClockSpeed() {
      original_clock_hz = flashTransport.getClockSpeed();
      Serial.printf("original SPI clock %d hz\n", original_clock_hz);
    }

    void setClockRate(uint32_t divisor) {
      auto clock_hz = divisor == 0 ? 1000000 : original_clock_hz / divisor;
      flashTransport.setClockSpeed(clock_hz);
      Serial.printf("now using SPI clock %d hz\n", clock_hz);
    }

  #endif
}





void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);


  if (!flashBegin()) {
    fatal("Failed to initialize flash chip.");
  }
  #ifdef USE_CLOCK_CONTROLS
    noteOriginalClockSpeed();
  #endif

  Serial.print(
    "\n"
    "Enter a command:\n"
    "   e(rase)  - erase the whole flash chip\n"
    "   w(rite)  - write test data in every block\n"
    "   v(erify) - read all blocks, and verify those with Helo data\n"
  #ifdef USE_CLOCK_CONTROLS
    "   1, 2, 4  - reduce SPI clock by this factor\n"
    "   s(low)   - set SPI clock to 1 MHz\n"
  #endif
    );
}

void loop() {
  Serial.print("\n> ");

  char cmd[32];

  while (Serial.readBytesUntil('\n', cmd, sizeof(cmd)) < 1)
    delay(100);

  Serial.println(cmd);

  switch (cmd[0]) {
    case 'e':  eraseFlash();     break;
    case 'w':  writeTestData();  break;
    case 'v':  verifyTestData(); break;
  #ifdef USE_CLOCK_CONTROLS
    case '1':  setClockRate(1);  break;
    case '2':  setClockRate(2);  break;
    case '4':  setClockRate(4);  break;
    case 's':  setClockRate(0);  break;
  #endif
    default:
      Serial.println("what?");
  }
}
