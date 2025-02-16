#include <allegro5/allegro.h>

#include "common.c"

static int error = 0;

#define SEP() log_printf("\n#line %d\n", __LINE__)


#define CHECK(_expr, _type, _expect) do { _type res = (_expr); if (res != (_expect)) { log_printf("FAIL %s\n", #_expr); error++; } else { log_printf("OK %s\n", #_expr); } } while (0)










static void read_test(void)
{
ALLEGRO_FILE *f;
uint8_t bs[8];
int32_t sz;

f = al_fopen("data/allegro.pcx", "rb");
if (!f) {
error++;
return;
}


SEP();
CHECK(al_ftell(f), int, 0);
CHECK(al_fgetc(f), int, 0xa);
CHECK(al_fgetc(f), int, 0x5);
CHECK(al_ftell(f), int64_t, 2);


SEP();
CHECK(al_fungetc(f, 0x55), bool, true);
CHECK(al_ftell(f), int64_t, 1);


SEP();
CHECK(al_fread(f, bs, sizeof(bs)), size_t, sizeof(bs));
CHECK(bs[0], uint8_t, 0x55); 
CHECK(bs[1], uint8_t, 0x01);
CHECK(bs[2], uint8_t, 0x08);
CHECK(bs[3], uint8_t, 0x00);
CHECK(bs[4], uint8_t, 0x00);
CHECK(bs[5], uint8_t, 0x00);
CHECK(bs[6], uint8_t, 0x00);
CHECK(bs[7], uint8_t, 0x3f);
CHECK(al_ftell(f), int64_t, 9);


SEP();
CHECK(al_fseek(f, 13, ALLEGRO_SEEK_SET), bool, true);
CHECK(al_ftell(f), int64_t, 13);
CHECK(al_fgetc(f), int, 0x02);


SEP();
CHECK(al_fseek(f, 0, ALLEGRO_SEEK_CUR), bool, true);
CHECK(al_ftell(f), int64_t, 14);
CHECK(al_fgetc(f), int, 0xe0);


SEP();
CHECK(al_fungetc(f, 0x55), bool, true);
CHECK(al_fseek(f, 0, ALLEGRO_SEEK_CUR), bool, true);
CHECK(al_ftell(f), int64_t, 14);
CHECK(al_fgetc(f), int, 0xe0);


SEP();
CHECK(al_fseek(f, -3, ALLEGRO_SEEK_CUR), bool, true);
CHECK(al_ftell(f), int64_t, 12);
CHECK(al_fgetc(f), int, 0x80);


SEP();
CHECK(al_ftell(f), int64_t, 13);
CHECK(al_fungetc(f, 0x66), bool, true);
CHECK(al_ftell(f), int64_t, 12);
CHECK(al_fseek(f, -2, ALLEGRO_SEEK_CUR), bool, true);
CHECK(al_ftell(f), int64_t, 10);
CHECK(al_fgetc(f), int, 0xc7);


SEP();
CHECK(al_fseek(f, 0, ALLEGRO_SEEK_END), bool, true);
CHECK(al_feof(f), bool, false);
CHECK(al_ftell(f), int64_t, 0xab06);


SEP();
CHECK(al_fgetc(f), int, -1);
CHECK(al_feof(f), bool, true);
CHECK(al_ferror(f), int, 0);


SEP();
CHECK(al_fseek(f, 0, ALLEGRO_SEEK_END), bool, true);
CHECK(al_feof(f), bool, false);
CHECK(al_ftell(f), int64_t, 0xab06);


SEP();
CHECK(al_fseek(f, -20, ALLEGRO_SEEK_END), bool, true);
CHECK(al_ftell(f), int64_t, 0xaaf2);


SEP();
CHECK(al_fseek(f, 20, ALLEGRO_SEEK_END), bool, true);
CHECK(al_ftell(f), int64_t, 0xab1a);
CHECK(al_fgetc(f), int, -1);
CHECK(al_feof(f), bool, true);


SEP();
CHECK(sz = al_fsize(f), int64_t, sz);
if (sz != -1)
CHECK(sz, int64_t, 0xab06);


SEP();
CHECK(al_fclose(f), bool, true);
}

int main(int argc, char *argv[])
{
(void)argc;
(void)argv;

if (!al_init())
return 1;
init_platform_specific();
open_log_monospace();

read_test();

close_log(true);

if (error) {
exit(EXIT_FAILURE);
}

return 0;
}


