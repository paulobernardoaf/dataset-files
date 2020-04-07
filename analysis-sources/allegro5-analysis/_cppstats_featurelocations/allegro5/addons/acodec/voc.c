









































#include "allegro5/allegro_audio.h"
#include "allegro5/internal/aintern_audio.h"
#include "acodec.h"
#include "helper.h"

ALLEGRO_DEBUG_CHANNEL("voc")

typedef struct AL_VOC_DATA AL_VOC_DATA;

struct AL_VOC_DATA {
ALLEGRO_FILE *file;
size_t datapos;
int samplerate;
short bits; 
short channels; 
int sample_size; 
int samples; 
};








#define READNBYTES(f, data, n, retv) do { if (al_fread(f, &data, n) != n) { ALLEGRO_WARN("voc_open: Bad Number of bytes read in last operation"); return retv; } } while(0)








static AL_VOC_DATA *voc_open(ALLEGRO_FILE *fp)
{
AL_VOC_DATA *vocdata;
char hdrbuf[0x16];
size_t readcount = 0;

uint8_t blocktype = 0;
uint8_t x = 0;

uint16_t timeconstant = 0;
uint16_t format = 0; 
uint16_t vocversion = 0;
uint16_t checkver = 0; 

uint32_t blocklength = 0; 

ASSERT(fp);


vocdata = al_malloc(sizeof(AL_VOC_DATA));
memset(vocdata, 0, sizeof(*vocdata));
memset(hdrbuf, 0, sizeof(hdrbuf));
vocdata->file = fp;


readcount = al_fread(fp, hdrbuf, 0x16);
if (readcount != 0x16 
|| !memcmp(hdrbuf, "Creative Voice File\0x1A", 0x14) 
|| !memcmp(hdrbuf+0x15 , "\0x00\0x1A", 0x2)) { 
ALLEGRO_ERROR("voc_open: File does not appear to be a valid VOC file");
return NULL;
}

al_fread(fp, &vocversion, 2);
if (vocversion != 0x10A && vocversion != 0x114) { 
ALLEGRO_ERROR("voc_open: File is of unknown version");
return NULL;
}

al_fread(fp, &checkver, 2);
if (checkver != ~vocversion + 0x1234) {
ALLEGRO_ERROR("voc_open: Bad VOC Version Identification Number");
return NULL;
}





READNBYTES(fp, blocktype, 1, NULL);
READNBYTES(fp, blocklength, 2, NULL);
READNBYTES(fp, x, 1, NULL);
blocklength += x<<16;
switch (blocktype) {
case 1:



blocklength -= 2;
READNBYTES(fp, timeconstant, 1, NULL);
READNBYTES(fp, format, 1, NULL);
vocdata->bits = 8; 
vocdata->channels = 1; 
vocdata->samplerate = 1000000 / (256 - timeconstant);
vocdata->sample_size = 1; 





vocdata->samples = blocklength / vocdata->sample_size;
vocdata->datapos = al_ftell(fp);
break;
case 8:






if (blocklength != 4) {
ALLEGRO_ERROR("voc_open: Got opening Blocktype 8 of wrong length");
return NULL;
}
READNBYTES(fp, timeconstant, 2, NULL);
READNBYTES(fp, format, 1, NULL);
READNBYTES(fp, vocdata->channels, 1, NULL);
vocdata->channels += 1; 
vocdata->bits = 8; 
vocdata->samplerate = 1000000 / (256 - timeconstant);
vocdata->samplerate /= vocdata->channels;
vocdata->sample_size = vocdata->channels * vocdata->bits / 8;




READNBYTES(fp, blocktype, 1, NULL);
if (blocktype != 1) {
ALLEGRO_ERROR("voc_open: Blocktype following type 8 is not 1");
return NULL;
}
READNBYTES(fp, blocklength, 2, NULL);
READNBYTES(fp, x, 1, NULL);
blocklength += x<<16;
blocklength -= 2;
READNBYTES(fp, x, 2, NULL);
vocdata->samples = blocklength / vocdata->sample_size;
vocdata->datapos = al_ftell(fp);
break;
case 9:






blocklength -= 12;
READNBYTES(fp, vocdata->samplerate, 4, NULL); 
READNBYTES(fp, vocdata->bits, 1, NULL); 

READNBYTES(fp, vocdata->channels, 1, NULL); 
READNBYTES(fp, format, 2, NULL);
if ((vocdata->bits != 8 && vocdata->bits != 16) ||
(format != 0 && format != 4)) {
ALLEGRO_ERROR("voc_open: unsupported CODEC in voc data");
return NULL;
}
READNBYTES(fp, x, 4, NULL); 
vocdata->datapos = al_ftell(fp);
break;
case 2: 
case 3: 
case 4: 
case 5: 
case 6: 
case 7: 
default:
ALLEGRO_ERROR("voc_open: opening Block is of unsupported type");
return NULL;
break;
}
return vocdata;
}

static void voc_close(AL_VOC_DATA *vocdata)
{
ASSERT(vocdata);

al_free(vocdata);
}


ALLEGRO_SAMPLE *_al_load_voc(const char *filename)
{
ALLEGRO_FILE *f;
ALLEGRO_SAMPLE *spl;
ASSERT(filename);

ALLEGRO_INFO("Loading VOC sample %s.\n", filename);
f = al_fopen(filename, "rb");
if (!f) {
ALLEGRO_ERROR("Unable to open %s for reading.\n", filename);
return NULL;
}

spl = _al_load_voc_f(f);

al_fclose(f);

return spl;
}


ALLEGRO_SAMPLE *_al_load_voc_f(ALLEGRO_FILE *file)
{
AL_VOC_DATA *vocdata;
ALLEGRO_SAMPLE *sample = NULL;
size_t pos = 0; 
size_t read = 0; 
char* buffer;

size_t bytestoread = 0;
bool endofvoc = false;

vocdata = al_malloc(sizeof(AL_VOC_DATA));
memset(vocdata, 0, sizeof(*vocdata));





vocdata = voc_open(file);
if (!vocdata) return NULL;

ALLEGRO_DEBUG("channels %d\n", vocdata->channels);
ALLEGRO_DEBUG("word_size %d\n", vocdata->sample_size);
ALLEGRO_DEBUG("rate %d\n", vocdata->samplerate);
ALLEGRO_DEBUG("first_block_samples %d\n", vocdata->samples);
ALLEGRO_DEBUG("first_block_size %d\n", vocdata->samples * vocdata->sample_size);




buffer = al_malloc(vocdata->samples * vocdata->sample_size);
if (!buffer) {
return NULL;
}




bytestoread = vocdata->samples * vocdata->sample_size;
while(!endofvoc && !al_feof(vocdata->file)) {
uint32_t blocktype = 0;
uint32_t x = 0, len = 0;
read = al_fread(vocdata->file, buffer, bytestoread);
pos += read;
READNBYTES(vocdata->file, blocktype, 1, NULL); 
if (al_feof(vocdata->file)) break;
switch (blocktype) {
case 0:{ 
endofvoc = true;
break;
}
case 2:{ 
x = 0;
bytestoread = 0;
READNBYTES(vocdata->file, bytestoread, 2, NULL);
READNBYTES(vocdata->file, x, 1, NULL);
bytestoread += x<<16;

buffer = al_realloc(buffer, sizeof(buffer) + bytestoread);
break;
}
case 1: 
case 8: 
case 9:{ 
endofvoc = true;
break;
}
case 3: 
case 4: 
case 5: 
case 6: 
case 7:{ 

unsigned int ii;
len = 0;
x = 0;
READNBYTES(vocdata->file, len, 2, NULL);
READNBYTES(vocdata->file, x, 1, NULL);
len += x<<16; 
for (ii = 0; ii < len ; ++ii) {
al_fgetc(vocdata->file);
}
bytestoread = 0; 
break;
}
default:
break;
}
}

sample = al_create_sample(buffer, pos, vocdata->samplerate,
_al_word_size_to_depth_conf(vocdata->sample_size),
_al_count_to_channel_conf(vocdata->channels),
true);
if (!sample)
al_free(buffer);

voc_close(vocdata);

return sample;
}






