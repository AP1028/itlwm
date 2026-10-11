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

int main()
{
    channels();
    puts("WCL control regression tests passed");
}
