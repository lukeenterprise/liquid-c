#if !defined(LIQUID_H)
#define LIQUID_H

#include <ruby.h>
#include <ruby/encoding.h>
#include <stdbool.h>

extern ID id_evaluate;
extern ID id_to_liquid;
extern ID id_to_s;
extern ID id_call;
extern ID id_compile_evaluate;

extern VALUE mLiquid, mLiquidC, cLiquidVariable, cLiquidTemplate, cLiquidBlockBody;
extern VALUE cLiquidVariableLookup, cLiquidRangeLookup;
extern VALUE cLiquidArgumentError, cLiquidSyntaxError;
extern rb_encoding *utf8_encoding;

#ifndef RB_LIKELY
// RB_LIKELY added in Ruby 2.4
#define RB_LIKELY(x) (__builtin_expect(!!(x), 1))
#endif

#endif

