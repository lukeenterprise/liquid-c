#include "liquid.h"
#include "tokenizer.h"
#include "variable.h"
#include "lexer.h"
#include "parser.h"
#include "expression.h"
#include "block.h"
#include "context.h"
#include "variable_lookup.h"
#include "vm.h"

ID id_evaluate;
ID id_to_liquid;
ID id_to_s;
ID id_call;
ID id_compile_evaluate;

VALUE mLiquid, mLiquidC, cLiquidVariable, cLiquidTemplate, cLiquidBlockBody;
VALUE cLiquidVariableLookup, cLiquidRangeLookup;
VALUE cLiquidArgumentError, cLiquidSyntaxError;

rb_encoding *utf8_encoding;

void Init_liquid_c(void)
{
    id_evaluate = rb_intern("evaluate");
    id_to_liquid = rb_intern("to_liquid");
    id_to_s = rb_intern("to_s");
    id_call = rb_intern("call");
    id_compile_evaluate = rb_intern("compile_evaluate");

    utf8_encoding = rb_utf8_encoding();
    mLiquid = rb_define_module("Liquid");
    rb_global_variable(&mLiquid);

    mLiquidC = rb_define_module_under(mLiquid, "C");
    rb_global_variable(&mLiquidC);

    cLiquidArgumentError = rb_const_get(mLiquid, rb_intern("ArgumentError"));
    rb_global_variable(&cLiquidArgumentError);

    cLiquidSyntaxError = rb_const_get(mLiquid, rb_intern("SyntaxError"));
    rb_global_variable(&cLiquidSyntaxError);

    cLiquidVariable = rb_const_get(mLiquid, rb_intern("Variable"));
    rb_global_variable(&cLiquidVariable);

    cLiquidTemplate = rb_const_get(mLiquid, rb_intern("Template"));
    rb_global_variable(&cLiquidTemplate);

    cLiquidBlockBody = rb_const_get(mLiquid, rb_intern("BlockBody"));
    rb_global_variable(&cLiquidBlockBody);

    cLiquidVariableLookup = rb_const_get(mLiquid, rb_intern("VariableLookup"));
    rb_global_variable(&cLiquidVariableLookup);

    cLiquidRangeLookup = rb_const_get(mLiquid, rb_intern("RangeLookup"));
    rb_global_variable(&cLiquidRangeLookup);

    init_liquid_tokenizer();
    init_liquid_parser();
    init_liquid_expression();
    init_liquid_variable();
    init_liquid_block();
    init_liquid_context();
    init_liquid_variable_lookup();
    init_liquid_vm();
}

