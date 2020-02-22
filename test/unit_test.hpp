// Copyright 2018 Quattro Ace. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include <stdio.h>
#include <assert.h>

#define UT_RUN(func) \
    printf("----- %s:%d %s() start -----\n", __FILE__, __LINE__, #func); \
    func(); \
    printf("----- %s:%d %s() end   -----\n", __FILE__, __LINE__, #func); \

#define UT_ASSERT_INT(expected, actual)                                 \
    if (expected != actual) {                                           \
        printf("assertion fail %s:%d\n", __FILE__, __LINE__);           \
        printf("  expected: %d\n  actual:   %d\n",                      \
               expected, actual);                                       \
        assert(false);                                                  \
    }                                                                   \

#define UT_ASSERT_STRING(expected, actual)                              \
    if (strcmp(expected, actual) != 0) {                                \
        printf("assertion fail %s:%d\n", __FILE__, __LINE__);           \
        printf("  expected: %s\n  actual:   %s\n",                      \
               expected, actual);                                       \
        assert(false);                                                  \
    }                                                                   \

#define UT_ASSERT_POINTER(expected, actual)                             \
    if (expected != actual) {                                           \
        printf("assertion fail %s:%d\n", __FILE__, __LINE__);           \
        printf("  expected: %p\n  actual:   %p\n",                      \
               expected, actual);                                       \
        assert(false);                                                  \
    }                                                                   \

#define UT_ASSERT_TRUE(actual)  assert(actual)
#define UT_ASSERT_FALSE(actual) assert(!actual)
