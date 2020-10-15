#include "liquid.h"
#include "vm_assembler.h"

void vm_assembler_init(vm_assembler_t *code)
{
    code->instructions = c_buffer_allocate(8);
    code->constants = c_buffer_allocate(8 * sizeof(VALUE));
    code->max_stack_size = 0;
    code->stack_size = 0;
}

void vm_assembler_free(vm_assembler_t *code)
{
    c_buffer_free(&code->instructions);
    c_buffer_free(&code->constants);
}

void vm_assembler_gc_mark(vm_assembler_t *code)
{
    c_buffer_rb_gc_mark(&code->constants);
}

void vm_assembler_add_write_raw(vm_assembler_t *code, const char *string, size_t size)
{
    uint8_t instructions[4];
    instructions[0] = OP_WRITE_RAW;
    uint24_to_bytes((unsigned int)size, &instructions[1]);

    c_buffer_write(&code->instructions, &instructions, sizeof(instructions));
    c_buffer_write(&code->instructions, (char *)string, size);
}

void vm_assembler_add_write_node(vm_assembler_t *code, VALUE node)
{
    vm_assembler_write_opcode(code, OP_WRITE_NODE);
    vm_assembler_write_ruby_constant(code, node);
}

void vm_assembler_add_push_fixnum(vm_assembler_t *code, VALUE num)
{
    int x = FIX2INT(num);
    if (x >= INT8_MIN && x <= INT8_MAX) {
        vm_assembler_add_push_int8(code, x);
    } else if (x >= INT16_MIN && x <= INT16_MAX) {
        vm_assembler_add_push_int16(code, x);
    } else {
        vm_assembler_add_push_const(code, num);
    }
}

void vm_assembler_add_push_literal(vm_assembler_t *code, VALUE literal)
{
    switch (literal) {
    case Qnil:
        vm_assembler_add_push_nil(code);
        break;
    case Qtrue:
        vm_assembler_add_push_true(code);
        break;
    case Qfalse:
        vm_assembler_add_push_false(code);
        break;
    default:
        if (RB_FIXNUM_P(literal)) {
            vm_assembler_add_push_fixnum(code, literal);
        } else {
            vm_assembler_add_push_const(code, literal);
        }
        break;
    }
}
