#ifndef BLIMP_INCLUDE_GUARD_UI_FILESIZE_TO_STRING_HPP
#define BLIMP_INCLUDE_GUARD_UI_FILESIZE_TO_STRING_HPP

#include <QLocale>
#include <QObject>
#include <QString>

#include <cstdint>

inline QString filesize_to_string(std::uint64_t bytes)
{
    std::uint64_t constexpr kb = 1024;
    std::uint64_t constexpr mb = 1024 * kb;
    std::uint64_t constexpr gb = 1024 * mb;
    std::uint64_t constexpr tb = 1024 * gb;
    if (bytes >= tb) {
        return QObject::tr("%1 TB").arg(QLocale().toString(static_cast<double>(bytes) / tb, 'f', 2));
    } else if (bytes >= gb) {
        return QObject::tr("%1 GB").arg(QLocale().toString(static_cast<double>(bytes) / gb, 'f', 2));
    } else if (bytes >= mb) {
        return QObject::tr("%1 MB").arg(QLocale().toString(static_cast<double>(bytes) / mb, 'f', 1));
    } else if (bytes >= kb) {
        return QObject::tr("%1 KB").arg(QLocale().toString(static_cast<int>(bytes / kb)));
    } else {
        if (bytes == 1) {
            return QObject::tr("1 byte");
        } else {
            return QObject::tr("%1 bytes").arg(QLocale().toString(static_cast<int>(bytes)));
        }
    }
};

#endif
