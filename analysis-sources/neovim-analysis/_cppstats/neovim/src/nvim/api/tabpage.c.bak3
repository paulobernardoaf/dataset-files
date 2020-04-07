


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "nvim/api/tabpage.h"
#include "nvim/api/vim.h"
#include "nvim/api/private/defs.h"
#include "nvim/api/private/helpers.h"
#include "nvim/memory.h"
#include "nvim/window.h"






ArrayOf(Window) nvim_tabpage_list_wins(Tabpage tabpage, Error *err)
FUNC_API_SINCE(1)
{
Array rv = ARRAY_DICT_INIT;
tabpage_T *tab = find_tab_by_handle(tabpage, err);

if (!tab || !valid_tabpage(tab)) {
return rv;
}

FOR_ALL_WINDOWS_IN_TAB(wp, tab) {
rv.size++;
}

rv.items = xmalloc(sizeof(Object) * rv.size);
size_t i = 0;

FOR_ALL_WINDOWS_IN_TAB(wp, tab) {
rv.items[i++] = WINDOW_OBJ(wp->handle);
}

return rv;
}







Object nvim_tabpage_get_var(Tabpage tabpage, String name, Error *err)
FUNC_API_SINCE(1)
{
tabpage_T *tab = find_tab_by_handle(tabpage, err);

if (!tab) {
return (Object) OBJECT_INIT;
}

return dict_get_value(tab->tp_vars, name, err);
}







void nvim_tabpage_set_var(Tabpage tabpage,
String name,
Object value,
Error *err)
FUNC_API_SINCE(1)
{
tabpage_T *tab = find_tab_by_handle(tabpage, err);

if (!tab) {
return;
}

dict_set_var(tab->tp_vars, name, value, false, false, err);
}






void nvim_tabpage_del_var(Tabpage tabpage, String name, Error *err)
FUNC_API_SINCE(1)
{
tabpage_T *tab = find_tab_by_handle(tabpage, err);

if (!tab) {
return;
}

dict_set_var(tab->tp_vars, name, NIL, true, false, err);
}













Object tabpage_set_var(Tabpage tabpage, String name, Object value, Error *err)
{
tabpage_T *tab = find_tab_by_handle(tabpage, err);

if (!tab) {
return (Object) OBJECT_INIT;
}

return dict_set_var(tab->tp_vars, name, value, false, true, err);
}









Object tabpage_del_var(Tabpage tabpage, String name, Error *err)
{
tabpage_T *tab = find_tab_by_handle(tabpage, err);

if (!tab) {
return (Object) OBJECT_INIT;
}

return dict_set_var(tab->tp_vars, name, NIL, true, true, err);
}






Window nvim_tabpage_get_win(Tabpage tabpage, Error *err)
FUNC_API_SINCE(1)
{
Window rv = 0;
tabpage_T *tab = find_tab_by_handle(tabpage, err);

if (!tab || !valid_tabpage(tab)) {
return rv;
}

if (tab == curtab) {
return nvim_get_current_win();
} else {
FOR_ALL_WINDOWS_IN_TAB(wp, tab) {
if (wp == tab->tp_curwin) {
return wp->handle;
}
}

abort();
}
}






Integer nvim_tabpage_get_number(Tabpage tabpage, Error *err)
FUNC_API_SINCE(1)
{
Integer rv = 0;
tabpage_T *tab = find_tab_by_handle(tabpage, err);

if (!tab) {
return rv;
}

return tabpage_index(tab);
}





Boolean nvim_tabpage_is_valid(Tabpage tabpage)
FUNC_API_SINCE(1)
{
Error stub = ERROR_INIT;
Boolean ret = find_tab_by_handle(tabpage, &stub) != NULL;
api_clear_error(&stub);
return ret;
}

