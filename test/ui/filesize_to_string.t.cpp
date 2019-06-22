#include <ui/filesize_to_string.hpp>

#include <catch.hpp>

#include <ostream>

std::ostream& operator<<(std::ostream& os, QString const& qstr) {
    return os << qstr.toStdString();
}

TEST_CASE("filesize_to_string()")
{
    SECTION("Sizes < 1 KB")
    {
        CHECK(filesize_to_string(0) == QString("0 bytes"));
        CHECK(filesize_to_string(1) == QString("1 byte"));
        CHECK(filesize_to_string(2) == QString("2 bytes"));
        CHECK(filesize_to_string(3) == QString("3 bytes"));
        CHECK(filesize_to_string(4) == QString("4 bytes"));

        CHECK(filesize_to_string(42) == QString("42 bytes"));
        CHECK(filesize_to_string(422) == QString("422 bytes"));
        CHECK(filesize_to_string(1000) == QLocale().toString(1000) + QString(" bytes"));
        CHECK(filesize_to_string(1023) == QLocale().toString(1023) + QString(" bytes"));
    }

    SECTION("Sizes KB up to MB")
    {
        CHECK(filesize_to_string(1024) == QString("1 KB"));
        CHECK(filesize_to_string(1025) == QString("1 KB"));
    }

    SECTION("Sizes MB up to GB")
    {
        CHECK(filesize_to_string(1048576) == QLocale().toString(1.0, 'f', 1) + QString(" MB"));
        CHECK(filesize_to_string(1048577) == QLocale().toString(1.0, 'f', 1) + QString(" MB"));
        CHECK(filesize_to_string(1073741823) == QLocale().toString(1024.0, 'f', 1) + QString(" MB"));
    }

    SECTION("Sizes GB up to TB")
    {
        CHECK(filesize_to_string(1073741824) == QLocale().toString(1.0, 'f', 2) + QString(" GB"));
        CHECK(filesize_to_string(1073741825) == QLocale().toString(1.0, 'f', 2) + QString(" GB"));
        CHECK(filesize_to_string(1099511627775) == QLocale().toString(1024.0, 'f', 2) + QString(" GB"));
    }

    SECTION("Sizes > 1 TB")
    {
        CHECK(filesize_to_string(1099511627776) == QLocale().toString(1.0, 'f', 2) + QString(" TB"));
        CHECK(filesize_to_string(1099511627777) == QLocale().toString(1.0, 'f', 2) + QString(" TB"));
        CHECK(filesize_to_string(2251799813685248) == QLocale().toString(2048.0, 'f', 2) + QString(" TB"));
    }
}
