struct uv_pipe_s {
int something;
};
int uv_pipe_open(struct uv_pipe_s *handle, int fd)
{
int result;
if (result == 0) {
__coverity_escape__(fd);
}
return result;
}
typedef struct {} dictitem_T;
typedef struct {} dict_T;
int tv_dict_add(dict_T *const d, dictitem_T *const item)
{
__coverity_escape__(item);
}
