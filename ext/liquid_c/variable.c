#include "liquid.h"
#include "variable.h"
#include "parser.h"
#include "expression.h"
#include <stdio.h>

static ID id_rescue_strict_parse_syntax_error;

static int push_each_const_keyword_arg(VALUE key, VALUE value, VALUE func_arg)
{
    vm_assembler_t *code = (vm_assembler_t *)func_arg;

    vm_assembler_add_push_const(code, key);
    vm_assembler_add_push_literal(code, value);

    return ST_CONTINUE;
}

static VALUE try_variable_strict_parse(VALUE uncast_args)
{
    variable_parse_args_t *parse_args = (void *)uncast_args;
    parser_t p;
    init_parser(&p, parse_args->markup, parse_args->markup_end);
    vm_assembler_t *code = parse_args->code;

    if (p.cur.type == TOKEN_EOS)
        return Qnil;

    vm_assembler_add_render_variable_rescue(code, parse_args->line_number);

    parse_and_compile_expression(&p, code);

    while (parser_consume(&p, TOKEN_PIPE).type) {
        lexer_token_t filter_name_token = parser_must_consume(&p, TOKEN_IDENTIFIER);
        VALUE filter_name = token_to_rsym(filter_name_token);

        size_t arg_count = 0;
        size_t keyword_arg_count = 0;
        VALUE const_keyword_args = Qnil;
        VALUE push_keywords_obj = Qnil;
        vm_assembler_t *push_keywords_code = NULL;

        if (parser_consume(&p, TOKEN_COLON).type) {
            do {
                if (p.cur.type == TOKEN_IDENTIFIER && p.next.type == TOKEN_COLON) {
                    VALUE key = token_to_rstr(parser_consume_any(&p));
                    parser_consume_any(&p);

                    keyword_arg_count++;

                    if (push_keywords_obj != Qnil) {
                        vm_assembler_add_push_const(push_keywords_code, key);
                        parse_and_compile_expression(&p, push_keywords_code);
                    } else {
                        VALUE const_obj = try_parse_constant_expression(&p);

                        if (const_obj != Qundef) {
                            if (const_keyword_args == Qnil)
                                const_keyword_args = rb_hash_new();
                            rb_hash_aset(const_keyword_args, key, const_obj);
                        } else {
                            expression_t *push_keywords_expr;
                            // use an object to automatically free on an exception
                            push_keywords_obj = expression_new(&push_keywords_expr);
                            rb_obj_hide(push_keywords_obj);
                            push_keywords_code = &push_keywords_expr->code;

                            if (const_keyword_args != Qnil) {
                                rb_hash_foreach(const_keyword_args, push_each_const_keyword_arg, (VALUE)push_keywords_code);
                                const_keyword_args = Qnil;
                            }
                            vm_assembler_add_push_const(push_keywords_code, key);
                            parse_and_compile_expression(&p, push_keywords_code);
                        }
                    }
                } else {
                    parse_and_compile_expression(&p, code);
                    arg_count++;
                }
            } while (parser_consume(&p, TOKEN_COMMA).type);
        }

        if (keyword_arg_count) {
            arg_count++;
            if (keyword_arg_count > 255)
                rb_enc_raise(utf8_encoding, cLiquidSyntaxError, "Too many filter keyword arguments");

            if (const_keyword_args != Qnil) {
                rb_hash_freeze(const_keyword_args);
                vm_assembler_add_push_const(code, const_keyword_args);
            } else {
                vm_assembler_concat(code, push_keywords_code);
                vm_assembler_add_hash_new(code, keyword_arg_count);

                // There are no external references to this temporary object, so we can eagerly free it
                DATA_PTR(push_keywords_obj) = NULL;
                vm_assembler_free(push_keywords_code);
                rb_gc_force_recycle(push_keywords_obj); // also acts as a RB_GC_GUARD(push_keywords_obj);
            }
        }
        if (arg_count > 254) {
            rb_enc_raise(utf8_encoding, cLiquidSyntaxError, "Too many filter arguments");
        }
        vm_assembler_add_filter(code, filter_name, arg_count);
    }

    vm_assembler_add_pop_write_variable(code);

    parser_must_consume(&p, TOKEN_EOS);

    return Qnil;
}

typedef struct variable_strict_parse_rescue {
    variable_parse_args_t *parse_args;
    size_t instructions_size;
    size_t constants_size;
} variable_strict_parse_rescue_t;

static VALUE variable_strict_parse_rescue(VALUE uncast_args, VALUE exception)
{
    variable_strict_parse_rescue_t *rescue_args = (void *)uncast_args;
    variable_parse_args_t *parse_args = rescue_args->parse_args;
    vm_assembler_t *code = parse_args->code;
    code->instructions.data_end = code->instructions.data + rescue_args->instructions_size;
    code->constants.data_end = code->constants.data + rescue_args->constants_size;

    if (rb_obj_is_kind_of(exception, cLiquidSyntaxError) == Qfalse)
        rb_exc_raise(exception);

    VALUE markup_obj = rb_enc_str_new(parse_args->markup, parse_args->markup_end - parse_args->markup, utf8_encoding);
    VALUE variable_obj = rb_funcall(
        cLiquidVariable, id_rescue_strict_parse_syntax_error, 3,
        exception, markup_obj, parse_args->parse_context
    );

    vm_assembler_add_write_node(code, variable_obj);
    return Qnil;
}

void internal_variable_parse(variable_parse_args_t *parse_args)
{
    vm_assembler_t *code = parse_args->code;
    variable_strict_parse_rescue_t rescue_args = {
        .parse_args = parse_args,
        .instructions_size = c_buffer_size(&code->instructions),
        .constants_size = c_buffer_size(&code->constants),
    };
    rb_rescue(try_variable_strict_parse, (VALUE)parse_args, variable_strict_parse_rescue, (VALUE)&rescue_args);
}

void init_liquid_variable(void)
{
    id_rescue_strict_parse_syntax_error = rb_intern("rescue_strict_parse_syntax_error");
}

