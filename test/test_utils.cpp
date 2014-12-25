#include "virtmem.h"
#include "stdioalloc.h"
#include "test.h"

typedef CAllocFixture CUtilsFixture;

// Clamp between -1 - +1
inline int clampOne(int n)
{
    return (n < 0) ? -1 : (n == 0) ? 0 : 1;
}

TEST_F(CUtilsFixture, memcmpTest)
{
    const int bufsize = 10;
    TUCharVirtPtr vbuf1 = vbuf1.alloc(bufsize);
    TUCharVirtPtr vbuf2 = vbuf2.alloc(bufsize);

    uint8_t buf1[bufsize], buf2[bufsize];

    for (int i=0; i<bufsize; ++i)
        vbuf1[i] = vbuf2[i] = buf1[i] = buf2[i] = bufsize - i;

    valloc.clearPages();
    EXPECT_EQ(memcmp(vbuf1, vbuf2, bufsize), 0);
    EXPECT_EQ(memcmp(vbuf1, buf1, bufsize), 0);
    EXPECT_EQ(memcmp(buf1, vbuf1, bufsize), 0);
    EXPECT_EQ(clampOne(memcmp(vbuf1, vbuf2, bufsize)), clampOne(memcmp(buf1, buf2, bufsize)));
    EXPECT_EQ(clampOne(memcmp(vbuf2, vbuf1, bufsize)), clampOne(memcmp(buf2, buf1, bufsize)));

    vbuf2[2] = buf2[2] = 33;
    valloc.clearPages();
    EXPECT_EQ(clampOne(memcmp(vbuf1, vbuf2, bufsize)), clampOne(memcmp(buf1, buf2, bufsize)));
    EXPECT_EQ(clampOne(memcmp(buf1, vbuf2, bufsize)), clampOne(memcmp(buf1, buf2, bufsize)));
    EXPECT_EQ(clampOne(memcmp(vbuf1, buf2, bufsize)), clampOne(memcmp(buf1, buf2, bufsize)));
    EXPECT_EQ(clampOne(memcmp(vbuf2, vbuf1, bufsize)), clampOne(memcmp(buf2, buf1, bufsize)));
    EXPECT_EQ(clampOne(memcmp(buf2, vbuf1, bufsize)), clampOne(memcmp(buf2, buf1, bufsize)));
    EXPECT_EQ(clampOne(memcmp(vbuf2, buf1, bufsize)), clampOne(memcmp(buf2, buf1, bufsize)));

    vbuf1[3] = buf1[3] = 66;
    valloc.clearPages();
    EXPECT_EQ(clampOne(memcmp(vbuf1, vbuf2, bufsize)), clampOne(memcmp(buf1, buf2, bufsize)));
    EXPECT_EQ(clampOne(memcmp(buf1, vbuf2, bufsize)), clampOne(memcmp(buf1, buf2, bufsize)));
    EXPECT_EQ(clampOne(memcmp(vbuf1, buf2, bufsize)), clampOne(memcmp(buf1, buf2, bufsize)));
    EXPECT_EQ(clampOne(memcmp(vbuf2, vbuf1, bufsize)), clampOne(memcmp(buf2, buf1, bufsize)));
    EXPECT_EQ(clampOne(memcmp(buf2, vbuf1, bufsize)), clampOne(memcmp(buf2, buf1, bufsize)));
    EXPECT_EQ(clampOne(memcmp(vbuf2, buf1, bufsize)), clampOne(memcmp(buf2, buf1, bufsize)));
}

TEST_F(CUtilsFixture, memcpyTest)
{
    const int bufsize = 10;
    TUCharVirtPtr vbuf1 = vbuf1.alloc(bufsize);
    TUCharVirtPtr vbuf2 = vbuf2.alloc(bufsize);
    uint8_t buf[bufsize];

    for (int i=0; i<bufsize; ++i)
        vbuf1[i] = bufsize - i;

    valloc.clearPages();

    EXPECT_EQ(memcpy(vbuf2, vbuf1, bufsize), vbuf2);
    valloc.clearPages();
    EXPECT_EQ(memcmp(vbuf1, vbuf2, bufsize), 0);

    memset(buf, 0, bufsize);
    EXPECT_EQ(memcpy(buf, vbuf1, bufsize), buf);
    EXPECT_EQ(memcmp(vbuf1, buf, bufsize), 0);

    buf[3] = 65;
    EXPECT_EQ(memcpy(vbuf2, buf, bufsize), vbuf2);
    valloc.clearPages();
    EXPECT_EQ(memcmp(buf, vbuf2, bufsize), 0);
}

TEST_F(CUtilsFixture, memcpyLargeTest)
{
    const int bufsize = valloc.getPoolSize() / 3;
    std::vector<uint8_t> buf;

    buf.reserve(bufsize);
    for (int i=0; i<bufsize; ++i)
        buf[i] = bufsize - i;

    TUCharVirtPtr vbuf = vbuf.alloc(bufsize);
    memcpy(vbuf, &buf[0], bufsize);
    valloc.clearPages();
    EXPECT_EQ(memcmp(&buf[0], vbuf, bufsize), 0);

    TUCharVirtPtr vbuf2 = vbuf2.alloc(bufsize);
    memcpy(vbuf2, vbuf, bufsize);
    valloc.clearPages();
    EXPECT_EQ(memcmp(vbuf, vbuf2, bufsize), 0);
}

TEST_F(CUtilsFixture, memsetTest)
{
    const int bufsize = valloc.getPageSize() * 3;
    const uint8_t fill = 'A';

    TUCharVirtPtr vbuf = vbuf.alloc(bufsize);
    EXPECT_EQ(memset(vbuf, fill, bufsize), vbuf);

    valloc.clearPages();

    std::vector<uint8_t> buf(bufsize, fill);
    EXPECT_EQ(memcmp(vbuf, &buf[0], bufsize), 0);
}

TEST_F(CUtilsFixture, memcpyLargeMultiAllocTest)
{
    // Second allocator. NOTE: the type is slightly different (different poolsize), so should give no singleton problems
    typedef CStdioVirtMemAlloc<1024*1024, 512, 4> TAlloc2;
    TAlloc2 valloc2;
    valloc2.start();

    const int bufsize = valloc2.getPoolSize() / 2;

    TUCharVirtPtr vbuf1 = vbuf1.alloc(bufsize);
    CVirtPtr<uint8_t, TAlloc2> vbuf2 = vbuf2.alloc(bufsize);

    memset(vbuf1, 'A', bufsize);
    memcpy(vbuf2, vbuf1, bufsize);
    EXPECT_EQ(memcmp(vbuf1, vbuf2, bufsize), 0);
}

TEST_F(CUtilsFixture, strlenTest)
{
    const int strsize = 10;
    TCharVirtPtr vstr = vstr.alloc(strsize);

    vstr[0] = 0;
    valloc.clearPages();
    EXPECT_EQ(strlen(vstr), 0);

    memset(vstr, 'A', strsize-1);
    vstr[strsize-1] = 0;
    valloc.clearPages();
    EXPECT_EQ(strlen(vstr), strsize-1);

    vstr[5] = 0;
    EXPECT_EQ(strlen(vstr), 5);

    vstr[1] = 0;
    EXPECT_EQ(strlen(vstr), 1);
}

TEST_F(CUtilsFixture, strncpyTest)
{
    const int strsize = 10;
    TCharVirtPtr vstr = vstr.alloc(strsize);

    char str[strsize] = "Howdy!", str2[strsize];
    EXPECT_EQ(strncpy(vstr, str, strsize), vstr);
    EXPECT_EQ(strncpy(str2, vstr, strsize), str2);
    EXPECT_EQ(strncmp(str, str2, strsize), 0);

    // non length versions. Zero terminated, so should be the same
    memset(vstr, 0, strsize);
    memset(str2, 0, strsize);
    EXPECT_EQ(strcpy(vstr, str), vstr);
    EXPECT_EQ(strcpy(str2, vstr), str2);
    EXPECT_EQ(strncmp(str, str2, strsize), 0);

}

TEST_F(CUtilsFixture, strncmpTest)
{
    const int strsize = 10;
    TCharVirtPtr vstr = vstr.alloc(strsize);
    TCharVirtPtr vstr2 = vstr.alloc(strsize);
    char str[strsize] = "Howdy!", str2[strsize];

    strncpy(vstr, str, strsize);
    strncpy(vstr2, str, strsize);
    EXPECT_EQ(strncmp(vstr, str, strsize), 0);
    EXPECT_EQ(strncmp(str, vstr, strsize), 0);
    EXPECT_EQ(strncmp(vstr, vstr2, strsize), 0);

    strcpy(str2, str);

    str2[1] = 'a'; vstr2[1] = 'a';
    EXPECT_EQ(clampOne(strncmp(vstr, vstr2, strsize)), clampOne(strncmp(str, str2, strsize)));
    EXPECT_EQ(clampOne(strncmp(vstr2, vstr, strsize)), clampOne(strncmp(str2, str, strsize)));

    // non length versions. Zero terminated, so should be the same
    EXPECT_EQ(strncmp(vstr, vstr2, strsize), strcmp(vstr, vstr2));
    EXPECT_EQ(strncmp(vstr2, vstr, strsize), strcmp(vstr2, vstr));
    EXPECT_EQ(strncmp(vstr, str2, strsize), strcmp(vstr, str2));
    EXPECT_EQ(strncmp(str2, vstr, strsize), strcmp(str2, vstr));

    CVirtPtr<const char, CStdioVirtMemAlloc<> > cvstr = vstr;
    EXPECT_EQ(strcmp(cvstr, vstr), 0);
}
