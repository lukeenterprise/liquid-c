#ifndef VM_H
#define VM_H

#include <ruby.h>
#include "block.h"
#include "context.h"
#include "resource_limits.h"

typedef struct vm {
    c_buffer_t stack;
    VALUE strainer;
    VALUE filter_methods;
    VALUE interrupts;
    VALUE resource_limits_obj;
    resource_limits_t *resource_limits;
    VALUE global_filter;
    bool strict_filters;
    bool invoking_filter;
    context_t context;
} vm_t;

void init_liquid_vm();
vm_t *vm_from_context(VALUE context);
void liquid_vm_render(block_body_t *block, VALUE context, VALUE output);
void liquid_vm_next_instruction(const uint8_t **ip_ptr, const size_t **const_ptr_ptr);
bool liquid_vm_filtering(VALUE context);
VALUE liquid_vm_evaluate(VALUE context, vm_assembler_t *code);

#endif
