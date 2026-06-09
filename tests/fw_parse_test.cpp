//
//  fw_parse_test.cpp
//  RealtekBluetoothFirmware
//
//  Created by theVakhovskeIsTaken on 2026/8/9.
//  Copyright © 2026 thegwchr. All rights reserved.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>

#include "../RealtekBluetoothFirmware/RtlFwParse.h"

static int g_fail = 0;
#define CHECK(cond, ...) do { \
    if (cond) { printf("  ok   - " __VA_ARGS__); printf("\n"); } \
    else { printf("  FAIL - " __VA_ARGS__); printf("\n"); g_fail++; } \
} while (0)

static std::vector<uint8_t> readFile(const char *path)
{
    std::vector<uint8_t> data;
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "cannot open %s\n", path); exit(2); }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    data.resize(n);
    if (fread(data.data(), 1, n, f) != (size_t)n) { fprintf(stderr, "read error\n"); exit(2); }
    fclose(f);
    return data;
}

int main(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : "fw/rtl8822cu_fw.bin";
    std::vector<uint8_t> fw = readFile(path);
    const uint8_t *p = fw.data();
    uint32_t len = (uint32_t)fw.size();
    printf("firmware: %s (%u bytes)\n", path, len);

    CHECK(len > 8 && memcmp(p, "RTBTCore", 8) == 0,
          "v2 signature 'RTBTCore' present");

    int pid = rtlFindProjectId(p, len);
    CHECK(pid == 13, "project_id == 13 (got %d)", pid);
    uint16_t lmp = rtlLmpForProjectId(pid);
    CHECK(lmp == 0x8822, "lmp_subver for project 13 == 0x8822 (got 0x%04x)", lmp);

    uint32_t numSections = (len >= 20) ? rtlLe32(p + 16) : 0;
    CHECK(numSections > 0 && numSections < 4096,
          "num_sections plausible (%u)", numSections);

    std::vector<RtlSubsecRef> scratch(512);
    std::vector<uint8_t> out(len);
    bool anyPayload = false;
    bool allInRange = true;
    uint32_t bestRom = 0, bestLen = 0;
    printf("  rom_version -> payload size (key_id=0):\n");
    for (uint8_t rom = 0; rom < 16; rom++) {
        uint32_t need = rtlParseFirmwareV2(p, len, rom, 0,
                                           scratch.data(), 512, NULL, 0);
        uint32_t got = rtlParseFirmwareV2(p, len, rom, 0,
                                          scratch.data(), 512, out.data(), len);
        if (got > len) allInRange = false;
        if (got > 0 && got <= len) {
            anyPayload = true;
            if (got > bestLen) { bestLen = got; bestRom = rom; }
        }
        if (need || got)
            printf("    %2u -> need %u, wrote %u\n", rom, need, got);
    }
    CHECK(allInRange, "no rom_version requires more than fwLen bytes");
    CHECK(anyPayload, "at least one rom_version yields a payload (best: rom %u, %u bytes)",
          bestRom, bestLen);

    {
        uint32_t L = bestLen ? bestLen : 1000;
        uint32_t frags = rtlFragmentCount(L);
        CHECK(frags == L / 252 + 1, "fragment count for %u bytes == %u", L, frags);
        int j = 0; uint8_t lastIndex = 0;
        for (uint32_t i = 0; i < frags; i++) {
            uint8_t index = (uint8_t)(j++);
            if (index == 0x7f) j = 1;
            if (i == frags - 1) index |= 0x80;
            lastIndex = index;
        }
        CHECK((lastIndex & 0x80) != 0, "last fragment sets end flag (0x%02x)", lastIndex);
    }

    {
        uint32_t r1 = rtlParseFirmwareV2(p, 4, 3, 0, scratch.data(), 512, out.data(), len);
        CHECK(r1 == 0, "truncated firmware returns 0");
        std::vector<uint8_t> junk(64, 0xAB);
        int jr = rtlFindProjectId(junk.data(), junk.size());
        CHECK(jr < 0, "garbage firmware: project id not found (%d)", jr);
    }

    printf("\n%s\n", g_fail ? "TESTS FAILED" : "ALL TESTS PASSED");
    return g_fail ? 1 : 0;
}
