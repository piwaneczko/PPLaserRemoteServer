#pragma once
#include <cstdint>

struct file_version_t {
    uint16_t major;
    uint16_t minor;
    uint16_t revision;
    uint16_t build;
};

struct file_info_t
{
    file_version_t version;
    size_t fileSize;
};