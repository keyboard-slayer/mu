/*
MIT License

Copyright (c) 2021-2022 The brutal operating system contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <mu-traits/alloc.h>

#include "str.h"

MaybeStr str_concat(Str const lhs, Str const rhs, Alloc *alloc)
{
    Str result = {
        .len = lhs.len + rhs.len,
    };

    u8 *buf = Try(MaybeStr, alloc->malloc(alloc, result.len));

    memcpy(buf, lhs.buf, lhs.len);
    memcpy(buf + lhs.len, rhs.buf, rhs.len);

    result.buf = buf;
    return Some(MaybeStr, result);
}

MaybeStr str_dup(Str const str, Alloc *alloc)
{
    if (str_empty(str))
    {
        return Some(MaybeStr, str_make_from_cstr(""));
    }

    u8 *buf = (u8 *)Try(MaybeStr, alloc->malloc(alloc, str.len));
    memcpy(buf, str.buf, str.len);
    return Some(MaybeStr, str_n(str.len, buf));
}

static int tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + ('a' - 'A');
    }
    return c;
}


bool str_eq_ci(Str const lhs, Str const rhs)
{
    if (lhs.len != rhs.len)
    {
        return false;
    }

    for (usize i = 0; i < lhs.len; i++)
    {
        if (tolower(lhs.buf[i]) != tolower(rhs.buf[i]))
        {
            return false;
        }
    }

    return true;
}

int str_count(Str const haystack, Str const needle)
{
    if (haystack.len < needle.len)
    {
        return 0;
    }

    int count = 0;
    for (usize i = 0; i < haystack.len - needle.len; i++)
    {
        Str slice = str_n(needle.len, haystack.buf + i);

        if (str_eq(slice, needle))
        {
            count++;
        }
    }

    return count;
}

int str_count_chr(Str const str, u8 chr)
{
    int result = 0;

    for (usize i = 0; i < str.len; i++)
    {
        if (str.buf[i] == chr)
        {
            result++;
        }
    }

    return result;
}

int str_last(Str const lStr, Str const rStr)
{
    if (lStr.len < rStr.len)
    {
        return 0;
    }

    int pos = -1;

    for (size_t i = 0; i < lStr.len - rStr.len; i++)
    {
        Str substr = str_n(rStr.len, lStr.buf + i);

        if (str_eq(substr, rStr))
        {
            pos = i;
        }
    }

    return pos;
}

int str_last_chr(Str const str, u8 chr)
{
    int result = -1;

    for (size_t i = 0; i < str.len; i++)
    {
        if (str.buf[i] == chr)
        {
            result = i;
        }
    }

    return result;
}

int str_first(Str const lStr, Str const rStr)
{
    if (lStr.len < rStr.len)
    {
        return -1;
    }

    for (size_t i = 0; i <= lStr.len - rStr.len; i++)
    {
        Str substr = str_n(rStr.len, lStr.buf + i);

        if (str_eq(substr, rStr))
        {
            return i;
        }
    }

    return -1;
}

int str_first_chr(Str const str, u8 chr)
{
    for (usize i = 0; i < str.len; i++)
    {
        if (str.buf[i] == chr)
        {
            return i;
        }
    }

    return -1;
}
