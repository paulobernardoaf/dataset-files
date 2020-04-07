

















#if !defined(UNIMOTION_H)
#define UNIMOTION_H

#if defined(__cplusplus)
extern "C" {
#endif


enum sms_hardware {
unknown = 0,
powerbook = 1,
ibook = 2,
highrespb = 3,
macbookpro = 4
};




int detect_sms(void);






int read_sms_raw(int type, int *x, int *y, int *z);

int read_sms(int type, int *x, int *y, int *z);



int read_sms_real(int type, double *x, double *y, double *z);

#if defined(__cplusplus)
}
#endif

#endif

