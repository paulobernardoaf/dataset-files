



















#include "avlanguage.h"
#include "libavutil/avstring.h"
#include "libavutil/common.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct LangEntry {
const char str[4];
uint16_t next_equivalent;
} LangEntry;

static const uint16_t lang_table_counts[] = { 484, 20, 184 };
static const uint16_t lang_table_offsets[] = { 0, 484, 504 };

static const LangEntry lang_table[] = {

{ "aar", 504 },
{ "abk", 505 },
{ "ace", 2 },
{ "ach", 3 },
{ "ada", 4 },
{ "ady", 5 },
{ "afa", 6 },
{ "afh", 7 },
{ "afr", 507 },
{ "ain", 9 },
{ "aka", 508 },
{ "akk", 11 },
{ "alb", 502 },
{ "ale", 13 },
{ "alg", 14 },
{ "alt", 15 },
{ "amh", 509 },
{ "ang", 17 },
{ "anp", 18 },
{ "apa", 19 },
{ "ara", 511 },
{ "arc", 21 },
{ "arg", 510 },
{ "arm", 492 },
{ "arn", 24 },
{ "arp", 25 },
{ "art", 26 },
{ "arw", 27 },
{ "asm", 512 },
{ "ast", 29 },
{ "ath", 30 },
{ "aus", 31 },
{ "ava", 513 },
{ "ave", 506 },
{ "awa", 34 },
{ "aym", 514 },
{ "aze", 515 },
{ "bad", 37 },
{ "bai", 38 },
{ "bak", 516 },
{ "bal", 40 },
{ "bam", 521 },
{ "ban", 42 },
{ "baq", 489 },
{ "bas", 44 },
{ "bat", 45 },
{ "bej", 46 },
{ "bel", 517 },
{ "bem", 48 },
{ "ben", 522 },
{ "ber", 50 },
{ "bho", 51 },
{ "bih", 519 },
{ "bik", 53 },
{ "bin", 54 },
{ "bis", 520 },
{ "bla", 56 },
{ "bnt", 57 },
{ "bos", 525 },
{ "bra", 59 },
{ "bre", 524 },
{ "btk", 61 },
{ "bua", 62 },
{ "bug", 63 },
{ "bul", 518 },
{ "bur", 498 },
{ "byn", 66 },
{ "cad", 67 },
{ "cai", 68 },
{ "car", 69 },
{ "cat", 526 },
{ "cau", 71 },
{ "ceb", 72 },
{ "cel", 73 },
{ "cha", 528 },
{ "chb", 75 },
{ "che", 527 },
{ "chg", 77 },
{ "chi", 503 },
{ "chk", 79 },
{ "chm", 80 },
{ "chn", 81 },
{ "cho", 82 },
{ "chp", 83 },
{ "chr", 84 },
{ "chu", 532 },
{ "chv", 533 },
{ "chy", 87 },
{ "cmc", 88 },
{ "cop", 89 },
{ "cor", 593 },
{ "cos", 529 },
{ "cpe", 92 },
{ "cpf", 93 },
{ "cpp", 94 },
{ "cre", 530 },
{ "crh", 96 },
{ "crp", 97 },
{ "csb", 98 },
{ "cus", 99 },
{ "cze", 485 },
{ "dak", 101 },
{ "dan", 535 },
{ "dar", 103 },
{ "day", 104 },
{ "del", 105 },
{ "den", 106 },
{ "dgr", 107 },
{ "din", 108 },
{ "div", 537 },
{ "doi", 110 },
{ "dra", 111 },
{ "dsb", 112 },
{ "dua", 113 },
{ "dum", 114 },
{ "dut", 499 },
{ "dyu", 116 },
{ "dzo", 538 },
{ "efi", 118 },
{ "egy", 119 },
{ "eka", 120 },
{ "elx", 121 },
{ "eng", 541 },
{ "enm", 123 },
{ "epo", 542 },
{ "est", 544 },
{ "ewe", 539 },
{ "ewo", 127 },
{ "fan", 128 },
{ "fao", 550 },
{ "fat", 130 },
{ "fij", 549 },
{ "fil", 132 },
{ "fin", 548 },
{ "fiu", 134 },
{ "fon", 135 },
{ "fre", 491 },
{ "frm", 137 },
{ "fro", 138 },
{ "frr", 139 },
{ "frs", 140 },
{ "fry", 552 },
{ "ful", 547 },
{ "fur", 143 },
{ "gaa", 144 },
{ "gay", 145 },
{ "gba", 146 },
{ "gem", 147 },
{ "geo", 494 },
{ "ger", 487 },
{ "gez", 150 },
{ "gil", 151 },
{ "gla", 554 },
{ "gle", 553 },
{ "glg", 555 },
{ "glv", 558 },
{ "gmh", 156 },
{ "goh", 157 },
{ "gon", 158 },
{ "gor", 159 },
{ "got", 160 },
{ "grb", 161 },
{ "grc", 162 },
{ "gre", 488 },
{ "grn", 556 },
{ "gsw", 165 },
{ "guj", 557 },
{ "gwi", 167 },
{ "hai", 168 },
{ "hat", 564 },
{ "hau", 559 },
{ "haw", 171 },
{ "heb", 560 },
{ "her", 567 },
{ "hil", 174 },
{ "him", 175 },
{ "hin", 561 },
{ "hit", 177 },
{ "hmn", 178 },
{ "hmo", 562 },
{ "hrv", 563 },
{ "hsb", 181 },
{ "hun", 565 },
{ "hup", 183 },
{ "iba", 184 },
{ "ibo", 571 },
{ "ice", 493 },
{ "ido", 574 },
{ "iii", 572 },
{ "ijo", 189 },
{ "iku", 577 },
{ "ile", 570 },
{ "ilo", 192 },
{ "ina", 568 },
{ "inc", 194 },
{ "ind", 569 },
{ "ine", 196 },
{ "inh", 197 },
{ "ipk", 573 },
{ "ira", 199 },
{ "iro", 200 },
{ "ita", 576 },
{ "jav", 579 },
{ "jbo", 203 },
{ "jpn", 578 },
{ "jpr", 205 },
{ "jrb", 206 },
{ "kaa", 207 },
{ "kab", 208 },
{ "kac", 209 },
{ "kal", 585 },
{ "kam", 211 },
{ "kan", 587 },
{ "kar", 213 },
{ "kas", 590 },
{ "kau", 589 },
{ "kaw", 216 },
{ "kaz", 584 },
{ "kbd", 218 },
{ "kha", 219 },
{ "khi", 220 },
{ "khm", 586 },
{ "kho", 222 },
{ "kik", 582 },
{ "kin", 640 },
{ "kir", 594 },
{ "kmb", 226 },
{ "kok", 227 },
{ "kom", 592 },
{ "kon", 581 },
{ "kor", 588 },
{ "kos", 231 },
{ "kpe", 232 },
{ "krc", 233 },
{ "krl", 234 },
{ "kro", 235 },
{ "kru", 236 },
{ "kua", 583 },
{ "kum", 238 },
{ "kur", 591 },
{ "kut", 240 },
{ "lad", 241 },
{ "lah", 242 },
{ "lam", 243 },
{ "lao", 600 },
{ "lat", 595 },
{ "lav", 603 },
{ "lez", 247 },
{ "lim", 598 },
{ "lin", 599 },
{ "lit", 601 },
{ "lol", 251 },
{ "loz", 252 },
{ "ltz", 596 },
{ "lua", 254 },
{ "lub", 602 },
{ "lug", 597 },
{ "lui", 257 },
{ "lun", 258 },
{ "luo", 259 },
{ "lus", 260 },
{ "mac", 495 },
{ "mad", 262 },
{ "mag", 263 },
{ "mah", 605 },
{ "mai", 265 },
{ "mak", 266 },
{ "mal", 608 },
{ "man", 268 },
{ "mao", 496 },
{ "map", 270 },
{ "mar", 610 },
{ "mas", 272 },
{ "may", 497 },
{ "mdf", 274 },
{ "mdr", 275 },
{ "men", 276 },
{ "mga", 277 },
{ "mic", 278 },
{ "min", 279 },
{ "mis", 280 },
{ "mkh", 281 },
{ "mlg", 604 },
{ "mlt", 612 },
{ "mnc", 284 },
{ "mni", 285 },
{ "mno", 286 },
{ "moh", 287 },
{ "mon", 609 },
{ "mos", 289 },
{ "mul", 290 },
{ "mun", 291 },
{ "mus", 292 },
{ "mwl", 293 },
{ "mwr", 294 },
{ "myn", 295 },
{ "myv", 296 },
{ "nah", 297 },
{ "nai", 298 },
{ "nap", 299 },
{ "nau", 614 },
{ "nav", 623 },
{ "nbl", 622 },
{ "nde", 616 },
{ "ndo", 618 },
{ "nds", 305 },
{ "nep", 617 },
{ "new", 307 },
{ "nia", 308 },
{ "nic", 309 },
{ "niu", 310 },
{ "nno", 620 },
{ "nob", 615 },
{ "nog", 313 },
{ "non", 314 },
{ "nor", 621 },
{ "nqo", 316 },
{ "nso", 317 },
{ "nub", 318 },
{ "nwc", 319 },
{ "nya", 624 },
{ "nym", 321 },
{ "nyn", 322 },
{ "nyo", 323 },
{ "nzi", 324 },
{ "oci", 625 },
{ "oji", 626 },
{ "ori", 628 },
{ "orm", 627 },
{ "osa", 329 },
{ "oss", 629 },
{ "ota", 331 },
{ "oto", 332 },
{ "paa", 333 },
{ "pag", 334 },
{ "pal", 335 },
{ "pam", 336 },
{ "pan", 630 },
{ "pap", 338 },
{ "pau", 339 },
{ "peo", 340 },
{ "per", 490 },
{ "phi", 342 },
{ "phn", 343 },
{ "pli", 631 },
{ "pol", 632 },
{ "pon", 346 },
{ "por", 634 },
{ "pra", 348 },
{ "pro", 349 },
{ "pus", 633 },
{ "que", 635 },
{ "raj", 352 },
{ "rap", 353 },
{ "rar", 354 },
{ "roa", 355 },
{ "roh", 636 },
{ "rom", 357 },
{ "rum", 500 },
{ "run", 637 },
{ "rup", 360 },
{ "rus", 639 },
{ "sad", 362 },
{ "sag", 645 },
{ "sah", 364 },
{ "sai", 365 },
{ "sal", 366 },
{ "sam", 367 },
{ "san", 641 },
{ "sas", 369 },
{ "sat", 370 },
{ "scn", 371 },
{ "sco", 372 },
{ "sel", 373 },
{ "sem", 374 },
{ "sga", 375 },
{ "sgn", 376 },
{ "shn", 377 },
{ "sid", 378 },
{ "sin", 646 },
{ "sio", 380 },
{ "sit", 381 },
{ "sla", 382 },
{ "slo", 501 },
{ "slv", 648 },
{ "sma", 385 },
{ "sme", 644 },
{ "smi", 387 },
{ "smj", 388 },
{ "smn", 389 },
{ "smo", 649 },
{ "sms", 391 },
{ "sna", 650 },
{ "snd", 643 },
{ "snk", 394 },
{ "sog", 395 },
{ "som", 651 },
{ "son", 397 },
{ "sot", 655 },
{ "spa", 543 },
{ "srd", 642 },
{ "srn", 401 },
{ "srp", 653 },
{ "srr", 403 },
{ "ssa", 404 },
{ "ssw", 654 },
{ "suk", 406 },
{ "sun", 656 },
{ "sus", 408 },
{ "sux", 409 },
{ "swa", 658 },
{ "swe", 657 },
{ "syc", 412 },
{ "syr", 413 },
{ "tah", 672 },
{ "tai", 415 },
{ "tam", 659 },
{ "tat", 670 },
{ "tel", 660 },
{ "tem", 419 },
{ "ter", 420 },
{ "tet", 421 },
{ "tgk", 661 },
{ "tgl", 665 },
{ "tha", 662 },
{ "tib", 484 },
{ "tig", 426 },
{ "tir", 663 },
{ "tiv", 428 },
{ "tkl", 429 },
{ "tlh", 430 },
{ "tli", 431 },
{ "tmh", 432 },
{ "tog", 433 },
{ "ton", 667 },
{ "tpi", 435 },
{ "tsi", 436 },
{ "tsn", 666 },
{ "tso", 669 },
{ "tuk", 664 },
{ "tum", 440 },
{ "tup", 441 },
{ "tur", 668 },
{ "tut", 443 },
{ "tvl", 444 },
{ "twi", 671 },
{ "tyv", 446 },
{ "udm", 447 },
{ "uga", 448 },
{ "uig", 673 },
{ "ukr", 674 },
{ "umb", 451 },
{ "und", 452 },
{ "urd", 675 },
{ "uzb", 676 },
{ "vai", 455 },
{ "ven", 677 },
{ "vie", 678 },
{ "vol", 679 },
{ "vot", 459 },
{ "wak", 460 },
{ "wal", 461 },
{ "war", 462 },
{ "was", 463 },
{ "wel", 486 },
{ "wen", 465 },
{ "wln", 680 },
{ "wol", 681 },
{ "xal", 468 },
{ "xho", 682 },
{ "yao", 470 },
{ "yap", 471 },
{ "yid", 683 },
{ "yor", 684 },
{ "ypk", 474 },
{ "zap", 475 },
{ "zbl", 476 },
{ "zen", 477 },
{ "zha", 685 },
{ "znd", 479 },
{ "zul", 687 },
{ "zun", 481 },
{ "zxx", 482 },
{ "zza", 483 },

{ "bod", 523 },
{ "ces", 531 },
{ "cym", 534 },
{ "deu", 536 },
{ "ell", 540 },
{ "eus", 545 },
{ "fas", 546 },
{ "fra", 551 },
{ "hye", 566 },
{ "isl", 575 },
{ "kat", 580 },
{ "mkd", 607 },
{ "mri", 606 },
{ "msa", 611 },
{ "mya", 613 },
{ "nld", 619 },
{ "ron", 638 },
{ "slk", 647 },
{ "sqi", 652 },
{ "zho", 686 },

{ "aa" , 0 },
{ "ab" , 1 },
{ "ae" , 33 },
{ "af" , 8 },
{ "ak" , 10 },
{ "am" , 16 },
{ "an" , 22 },
{ "ar" , 20 },
{ "as" , 28 },
{ "av" , 32 },
{ "ay" , 35 },
{ "az" , 36 },
{ "ba" , 39 },
{ "be" , 47 },
{ "bg" , 64 },
{ "bh" , 52 },
{ "bi" , 55 },
{ "bm" , 41 },
{ "bn" , 49 },
{ "bo" , 425 },
{ "br" , 60 },
{ "bs" , 58 },
{ "ca" , 70 },
{ "ce" , 76 },
{ "ch" , 74 },
{ "co" , 91 },
{ "cr" , 95 },
{ "cs" , 100 },
{ "cu" , 85 },
{ "cv" , 86 },
{ "cy" , 464 },
{ "da" , 102 },
{ "de" , 149 },
{ "dv" , 109 },
{ "dz" , 117 },
{ "ee" , 126 },
{ "el" , 163 },
{ "en" , 122 },
{ "eo" , 124 },
{ "es" , 399 },
{ "et" , 125 },
{ "eu" , 43 },
{ "fa" , 341 },
{ "ff" , 142 },
{ "fi" , 133 },
{ "fj" , 131 },
{ "fo" , 129 },
{ "fr" , 136 },
{ "fy" , 141 },
{ "ga" , 153 },
{ "gd" , 152 },
{ "gl" , 154 },
{ "gn" , 164 },
{ "gu" , 166 },
{ "gv" , 155 },
{ "ha" , 170 },
{ "he" , 172 },
{ "hi" , 176 },
{ "ho" , 179 },
{ "hr" , 180 },
{ "ht" , 169 },
{ "hu" , 182 },
{ "hy" , 23 },
{ "hz" , 173 },
{ "ia" , 193 },
{ "id" , 195 },
{ "ie" , 191 },
{ "ig" , 185 },
{ "ii" , 188 },
{ "ik" , 198 },
{ "io" , 187 },
{ "is" , 186 },
{ "it" , 201 },
{ "iu" , 190 },
{ "ja" , 204 },
{ "jv" , 202 },
{ "ka" , 148 },
{ "kg" , 229 },
{ "ki" , 223 },
{ "kj" , 237 },
{ "kk" , 217 },
{ "kl" , 210 },
{ "km" , 221 },
{ "kn" , 212 },
{ "ko" , 230 },
{ "kr" , 215 },
{ "ks" , 214 },
{ "ku" , 239 },
{ "kv" , 228 },
{ "kw" , 90 },
{ "ky" , 225 },
{ "la" , 245 },
{ "lb" , 253 },
{ "lg" , 256 },
{ "li" , 248 },
{ "ln" , 249 },
{ "lo" , 244 },
{ "lt" , 250 },
{ "lu" , 255 },
{ "lv" , 246 },
{ "mg" , 282 },
{ "mh" , 264 },
{ "mi" , 269 },
{ "mk" , 261 },
{ "ml" , 267 },
{ "mn" , 288 },
{ "mr" , 271 },
{ "ms" , 273 },
{ "mt" , 283 },
{ "my" , 65 },
{ "na" , 300 },
{ "nb" , 312 },
{ "nd" , 303 },
{ "ne" , 306 },
{ "ng" , 304 },
{ "nl" , 115 },
{ "nn" , 311 },
{ "no" , 315 },
{ "nr" , 302 },
{ "nv" , 301 },
{ "ny" , 320 },
{ "oc" , 325 },
{ "oj" , 326 },
{ "om" , 328 },
{ "or" , 327 },
{ "os" , 330 },
{ "pa" , 337 },
{ "pi" , 344 },
{ "pl" , 345 },
{ "ps" , 350 },
{ "pt" , 347 },
{ "qu" , 351 },
{ "rm" , 356 },
{ "rn" , 359 },
{ "ro" , 358 },
{ "ru" , 361 },
{ "rw" , 224 },
{ "sa" , 368 },
{ "sc" , 400 },
{ "sd" , 393 },
{ "se" , 386 },
{ "sg" , 363 },
{ "si" , 379 },
{ "sk" , 383 },
{ "sl" , 384 },
{ "sm" , 390 },
{ "sn" , 392 },
{ "so" , 396 },
{ "sq" , 12 },
{ "sr" , 402 },
{ "ss" , 405 },
{ "st" , 398 },
{ "su" , 407 },
{ "sv" , 411 },
{ "sw" , 410 },
{ "ta" , 416 },
{ "te" , 418 },
{ "tg" , 422 },
{ "th" , 424 },
{ "ti" , 427 },
{ "tk" , 439 },
{ "tl" , 423 },
{ "tn" , 437 },
{ "to" , 434 },
{ "tr" , 442 },
{ "ts" , 438 },
{ "tt" , 417 },
{ "tw" , 445 },
{ "ty" , 414 },
{ "ug" , 449 },
{ "uk" , 450 },
{ "ur" , 453 },
{ "uz" , 454 },
{ "ve" , 456 },
{ "vi" , 457 },
{ "vo" , 458 },
{ "wa" , 466 },
{ "wo" , 467 },
{ "xh" , 469 },
{ "yi" , 472 },
{ "yo" , 473 },
{ "za" , 478 },
{ "zh" , 78 },
{ "zu" , 480 },
{ "", 0 }
};

static int lang_table_compare(const void *lhs, const void *rhs)
{
return strcmp(lhs, ((const LangEntry *)rhs)->str);
}

const char *ff_convert_lang_to(const char *lang, enum AVLangCodespace target_codespace)
{
int i;
const LangEntry *entry = NULL;
const int NB_CODESPACES = FF_ARRAY_ELEMS(lang_table_counts);

if (target_codespace >= NB_CODESPACES)
return NULL;

for (i=0; !entry && i<NB_CODESPACES; i++)
entry = bsearch(lang,
lang_table + lang_table_offsets[i],
lang_table_counts[i],
sizeof(LangEntry),
lang_table_compare);
if (!entry)
return NULL;

for (i=0; i<NB_CODESPACES; i++)
if (entry >= lang_table + lang_table_offsets[target_codespace] &&
entry < lang_table + lang_table_offsets[target_codespace] + lang_table_counts[target_codespace])
return entry->str;
else
entry = lang_table + entry->next_equivalent;

if (target_codespace == AV_LANG_ISO639_2_TERM)
return ff_convert_lang_to(lang, AV_LANG_ISO639_2_BIBL);

return NULL;
}

#if LIBAVFORMAT_VERSION_MAJOR < 58
const char *av_convert_lang_to(const char *lang, enum AVLangCodespace target_codespace)
{
return ff_convert_lang_to(lang, target_codespace);
}
#endif
