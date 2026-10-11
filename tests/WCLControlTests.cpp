//
//  WCLControlTests.cpp
//  AirportItlwm
//
//  Created by laobamac on 2026/10/11.
//  Copyright © 2026 laobamac. All rights reserved.
//

#include "../AirportItlwm/WCL/WCL.hpp"
#include <assert.h>
#include <stdio.h>

template <typename T> static T read(const void *buffer, size_t offset)
{
    T value;
    memcpy(&value, static_cast<const uint8_t *>(buffer) + offset, sizeof(value));
    return value;
}

static void channels()
{
    struct {
        uint64_t before;
        AirportWCL::NativeChannelInfo info;
        uint64_t after;
    } buffer = {0x0123456789abcdefULL, {}, 0xfedcba9876543210ULL};
    auto &info = buffer.info;
    memcpy(info.country, "US", 3);
    assert(AirportWCL::appendChannel(info, 6, false, false, false, true, false));
    assert(AirportWCL::appendChannel(info, 52, true, true, true, true, true));
    assert(read<uint16_t>(&info, 0xc) == 2);
    assert(read<uint16_t>(&info, 0xe) == 0x1006);
    assert(read<uint16_t>(&info, 0x10) == 0xd034);
    assert(read<uint8_t>(&info, 0x32f) == 52);
    assert(read<uint8_t>(&info, 0x64f) == 1);
    assert(read<uint8_t>(&info, 0x7df) == 1);
    assert(read<uint8_t>(&info, 0x96f) == 1);
    assert(read<uint8_t>(&info, 0xaff) == 1);
    assert(read<uint32_t>(&info, 0xc94) == 0xd034);
    assert(read<uint32_t>(&info, 0x12d4) == 0x2f);
    assert(memcmp(reinterpret_cast<uint8_t *>(&info) + 8, "US\0\0", 4) == 0);
    for (size_t offset = 0x1910; offset < 0x2720; ++offset)
        assert(read<uint8_t>(&info, offset) == 0);
    assert(!AirportWCL::appendChannel(info, 0, false, false, false, false, false));
    assert(!AirportWCL::appendChannel(info, 256, false, false, false, false, false));
    for (unsigned i = 2; i < 400; ++i)
        assert(AirportWCL::appendChannel(info, 1, false, false, false, false, false));
    assert(!AirportWCL::appendChannel(info, 1, false, false, false, false, false));
    assert(read<uint16_t>(&info, 0xc) == 400);
    assert(buffer.before == 0x0123456789abcdefULL);
    assert(buffer.after == 0xfedcba9876543210ULL);
}

static void traffic()
{
    uint64_t tx[4] = {0x100000000ULL, 11, 19, 23};
    uint64_t rx[4] = {29, 31, 37, 41};
    auto before = AirportWCL::makeTrafficCounters(tx, rx, 1000000000ULL);
    assert(read<uint64_t>(&before, 0) == 42);
    assert(read<uint64_t>(&before, 8) == 78);
    assert(read<uint64_t>(&before, 0x10) == 0x100000035ULL);
    assert(read<uint64_t>(&before, 0x18) == 138);
    assert(read<uint64_t>(&before, 0x20) == 0);
    assert(read<uint64_t>(&before, 0x28) == 0);
    assert(read<uint64_t>(&before, 0x30) == 1000000000ULL);
    ++tx[0];
    rx[3] += 3;
    auto after = AirportWCL::makeTrafficCounters(tx, rx, 1500000000ULL);
    assert(after.infraTx - before.infraTx == 1);
    assert(after.infraRx - before.infraRx == 3);
    assert(after.realtimeInfraTx - before.realtimeInfraTx == 0);
    assert(after.realtimeInfraRx - before.realtimeInfraRx == 3);
    assert(after.timestampNanoseconds - before.timestampNanoseconds == 500000000ULL);
    auto idle = AirportWCL::makeTrafficCounters(tx, rx, 2000000000ULL);
    assert(idle.infraTx == after.infraTx && idle.infraRx == after.infraRx);
}

int main()
{
    channels();
    traffic();
    puts("WCL control regression tests passed");
}
