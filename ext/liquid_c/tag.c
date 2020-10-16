#include "liquid.h"
#include "vm_assembler.h"
#include "block.h"

static ID id_parse;

static VALUE cLiquidTag;

static VALUE tag_class_compile(VALUE klass, VALUE tag_name, VALUE markup,
        VALUE tokenizer_obj, VALUE parse_context_obj, VALUE block_body_obj)
{
    block_body_t *body;
    BlockBody_Get_Struct(block_body_obj, body);

    VALUE new_tag = rb_funcall(klass, id_parse, 4, tag_name, markup,
            tokenizer_obj, parse_context_obj);

    if (body->blank && !RTEST(rb_funcall(new_tag, id_blank_p, 0)))
        body->blank = false;

    rb_funcall(new_tag, id_compile, 1, block_body_obj);

    return Qnil;
}

static VALUE tag_compile(VALUE self, VALUE block_body_obj)
{
    block_body_t *body;
    BlockBody_Get_Struct(block_body_obj, body);
    vm_assembler_add_write_node_from_ruby(&body->code, self);
    return Qnil;
}

void init_liquid_tag()
{
    id_parse = rb_intern("parse");

    cLiquidTag = rb_const_get(mLiquid, rb_intern("Tag"));
    rb_global_variable(&cLiquidTag);

    rb_define_singleton_method(cLiquidTag, "compile", tag_class_compile, 5);
    rb_define_method(cLiquidTag, "compile", tag_compile, 1);
}
