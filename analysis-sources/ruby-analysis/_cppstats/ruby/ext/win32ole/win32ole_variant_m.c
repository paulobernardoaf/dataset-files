#include "win32ole.h"
void Init_win32ole_variant_m(void)
{
mWIN32OLE_VARIANT = rb_define_module_under(cWIN32OLE, "VARIANT");
rb_define_const(mWIN32OLE_VARIANT, "VT_EMPTY", RB_INT2FIX(VT_EMPTY));
rb_define_const(mWIN32OLE_VARIANT, "VT_NULL", RB_INT2FIX(VT_NULL));
rb_define_const(mWIN32OLE_VARIANT, "VT_I2", RB_INT2FIX(VT_I2));
rb_define_const(mWIN32OLE_VARIANT, "VT_I4", RB_INT2FIX(VT_I4));
rb_define_const(mWIN32OLE_VARIANT, "VT_R4", RB_INT2FIX(VT_R4));
rb_define_const(mWIN32OLE_VARIANT, "VT_R8", RB_INT2FIX(VT_R8));
rb_define_const(mWIN32OLE_VARIANT, "VT_CY", RB_INT2FIX(VT_CY));
rb_define_const(mWIN32OLE_VARIANT, "VT_DATE", RB_INT2FIX(VT_DATE));
rb_define_const(mWIN32OLE_VARIANT, "VT_BSTR", RB_INT2FIX(VT_BSTR));
rb_define_const(mWIN32OLE_VARIANT, "VT_USERDEFINED", RB_INT2FIX(VT_USERDEFINED));
rb_define_const(mWIN32OLE_VARIANT, "VT_PTR", RB_INT2FIX(VT_PTR));
rb_define_const(mWIN32OLE_VARIANT, "VT_DISPATCH", RB_INT2FIX(VT_DISPATCH));
rb_define_const(mWIN32OLE_VARIANT, "VT_ERROR", RB_INT2FIX(VT_ERROR));
rb_define_const(mWIN32OLE_VARIANT, "VT_BOOL", RB_INT2FIX(VT_BOOL));
rb_define_const(mWIN32OLE_VARIANT, "VT_VARIANT", RB_INT2FIX(VT_VARIANT));
rb_define_const(mWIN32OLE_VARIANT, "VT_UNKNOWN", RB_INT2FIX(VT_UNKNOWN));
rb_define_const(mWIN32OLE_VARIANT, "VT_I1", RB_INT2FIX(VT_I1));
rb_define_const(mWIN32OLE_VARIANT, "VT_UI1", RB_INT2FIX(VT_UI1));
rb_define_const(mWIN32OLE_VARIANT, "VT_UI2", RB_INT2FIX(VT_UI2));
rb_define_const(mWIN32OLE_VARIANT, "VT_UI4", RB_INT2FIX(VT_UI4));
#if (_MSC_VER >= 1300) || defined(__CYGWIN__) || defined(__MINGW32__)
rb_define_const(mWIN32OLE_VARIANT, "VT_I8", RB_INT2FIX(VT_I8));
rb_define_const(mWIN32OLE_VARIANT, "VT_UI8", RB_INT2FIX(VT_UI8));
#endif
rb_define_const(mWIN32OLE_VARIANT, "VT_INT", RB_INT2FIX(VT_INT));
rb_define_const(mWIN32OLE_VARIANT, "VT_UINT", RB_INT2FIX(VT_UINT));
rb_define_const(mWIN32OLE_VARIANT, "VT_ARRAY", RB_INT2FIX(VT_ARRAY));
rb_define_const(mWIN32OLE_VARIANT, "VT_BYREF", RB_INT2FIX(VT_BYREF));
}
