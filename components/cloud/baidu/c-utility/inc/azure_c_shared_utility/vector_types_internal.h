// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef VECTOR_TYPES_INTERNAL_H
#define VECTOR_TYPES_INTERNAL_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

typedef struct VECTOR_TAG
{
    void *storage;
    size_t count;
    size_t elementSize;
} VECTOR;

#endif /* VECTOR_TYPES_INTERNAL_H */
