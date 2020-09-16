#include "liquid.h"
#include "context.h"

static ID id_has_key, id_aref, id_fetch;

VALUE variable_lookup_key(VALUE context, VALUE object, VALUE key, bool is_command)
{
    if (rb_respond_to(object, id_aref) && (
        (rb_respond_to(object, id_has_key) && rb_funcall(object, id_has_key, 1, key)) ||
        (rb_obj_is_kind_of(key, rb_cInteger) && rb_respond_to(object, id_fetch))
    )) {
        VALUE next_object = rb_funcall(object, id_aref, 1, key);
        next_object = materialize_proc(context, object, key, next_object);
        object = value_to_liquid_and_set_context(next_object, context);
        return next_object;
    }

    if (is_command) {
        Check_Type(key, T_STRING);
        ID intern_key = rb_intern(RSTRING_PTR(key));
        if (rb_respond_to(object, intern_key)) {
            VALUE next_object = rb_funcall(object, rb_intern(RSTRING_PTR(key)), 0);
            object = value_to_liquid_and_set_context(next_object, context);
            return next_object;
        }
    }

    context_maybe_raise_undefined_variable(context, key);
    return Qnil;
}

void init_liquid_variable_lookup()
{
    id_has_key = rb_intern("key?");
    id_aref = rb_intern("[]");
    id_fetch = rb_intern("fetch");
}
