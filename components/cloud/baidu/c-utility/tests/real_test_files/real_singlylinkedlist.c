// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define singlylinkedlist_create         real_singlylinkedlist_create
#define singlylinkedlist_destroy        real_singlylinkedlist_destroy
#define singlylinkedlist_add            real_singlylinkedlist_add
#define singlylinkedlist_remove         real_singlylinkedlist_remove
#define singlylinkedlist_get_head_item  real_singlylinkedlist_get_head_item
#define singlylinkedlist_get_next_item  real_singlylinkedlist_get_next_item
#define singlylinkedlist_find           real_singlylinkedlist_find
#define singlylinkedlist_item_get_value real_singlylinkedlist_item_get_value
#define singlylinkedlist_remove_if      real_singlylinkedlist_remove_if
#define singlylinkedlist_foreach        real_singlylinkedlist_foreach

#define GBALLOC_H

#include "singlylinkedlist.c"
