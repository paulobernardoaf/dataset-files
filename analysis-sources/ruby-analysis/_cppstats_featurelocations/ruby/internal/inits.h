#if !defined(INTERNAL_INITS_H)
#define INTERNAL_INITS_H











void Init_class_hierarchy(void);


void Init_enc(void);
void Init_ext(void);


void Init_File(void);


void Init_heap(void);


int Init_enc_set_filesystem_encoding(void);


void Init_newline(void);


void Init_BareVM(void);
void Init_vm_objects(void);


void Init_vm_backtrace(void);


void Init_vm_eval(void);


void Init_vm_stack_canary(void);


void Init_eval_method(void);


void rb_call_inits(void);

#endif 
