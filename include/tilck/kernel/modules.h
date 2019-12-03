/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once

#include <tilck/common/basic_defs.h>
#include <tilck/kernel/list.h>

struct module {

   struct list_node node;

   const char *name;
   void (*init)(void);
};

void init_modules(void);
void register_module(struct module *m);

#define REGISTER_MODULE(m)                             \
   __attribute__((constructor))                        \
   static void __register_module(void)                 \
   {                                                   \
      register_module(m);                              \
   }

