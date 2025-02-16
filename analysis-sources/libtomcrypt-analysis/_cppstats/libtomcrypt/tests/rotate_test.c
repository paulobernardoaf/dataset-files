#include <tomcrypt_test.h>
int rotate_test(void)
{
ulong32 L32 = 0x12345678UL;
ulong64 L64 = CONST64(0x1122334455667788);
int err = 0;
if (ROR(L32, 0 ) != 0x12345678UL) { fprintf(stderr, "ROR 0 failed\n"); err++; }
if (ROR(L32, 1 ) != 0x091a2b3cUL) { fprintf(stderr, "ROR 1 failed\n"); err++; }
if (ROR(L32, 2 ) != 0x048d159eUL) { fprintf(stderr, "ROR 2 failed\n"); err++; }
if (ROR(L32, 3 ) != 0x02468acfUL) { fprintf(stderr, "ROR 3 failed\n"); err++; }
if (ROR(L32, 4 ) != 0x81234567UL) { fprintf(stderr, "ROR 4 failed\n"); err++; }
if (ROR(L32, 5 ) != 0xc091a2b3UL) { fprintf(stderr, "ROR 5 failed\n"); err++; }
if (ROR(L32, 6 ) != 0xe048d159UL) { fprintf(stderr, "ROR 6 failed\n"); err++; }
if (ROR(L32, 7 ) != 0xf02468acUL) { fprintf(stderr, "ROR 7 failed\n"); err++; }
if (ROR(L32, 8 ) != 0x78123456UL) { fprintf(stderr, "ROR 8 failed\n"); err++; }
if (ROR(L32, 9 ) != 0x3c091a2bUL) { fprintf(stderr, "ROR 9 failed\n"); err++; }
if (ROR(L32, 10) != 0x9e048d15UL) { fprintf(stderr, "ROR 10 failed\n"); err++; }
if (ROR(L32, 11) != 0xcf02468aUL) { fprintf(stderr, "ROR 11 failed\n"); err++; }
if (ROR(L32, 12) != 0x67812345UL) { fprintf(stderr, "ROR 12 failed\n"); err++; }
if (ROR(L32, 13) != 0xb3c091a2UL) { fprintf(stderr, "ROR 13 failed\n"); err++; }
if (ROR(L32, 14) != 0x59e048d1UL) { fprintf(stderr, "ROR 14 failed\n"); err++; }
if (ROR(L32, 15) != 0xacf02468UL) { fprintf(stderr, "ROR 15 failed\n"); err++; }
if (ROR(L32, 16) != 0x56781234UL) { fprintf(stderr, "ROR 16 failed\n"); err++; }
if (ROR(L32, 17) != 0x2b3c091aUL) { fprintf(stderr, "ROR 17 failed\n"); err++; }
if (ROR(L32, 18) != 0x159e048dUL) { fprintf(stderr, "ROR 18 failed\n"); err++; }
if (ROR(L32, 19) != 0x8acf0246UL) { fprintf(stderr, "ROR 19 failed\n"); err++; }
if (ROR(L32, 20) != 0x45678123UL) { fprintf(stderr, "ROR 20 failed\n"); err++; }
if (ROR(L32, 21) != 0xa2b3c091UL) { fprintf(stderr, "ROR 21 failed\n"); err++; }
if (ROR(L32, 22) != 0xd159e048UL) { fprintf(stderr, "ROR 22 failed\n"); err++; }
if (ROR(L32, 23) != 0x68acf024UL) { fprintf(stderr, "ROR 23 failed\n"); err++; }
if (ROR(L32, 24) != 0x34567812UL) { fprintf(stderr, "ROR 24 failed\n"); err++; }
if (ROR(L32, 25) != 0x1a2b3c09UL) { fprintf(stderr, "ROR 25 failed\n"); err++; }
if (ROR(L32, 26) != 0x8d159e04UL) { fprintf(stderr, "ROR 26 failed\n"); err++; }
if (ROR(L32, 27) != 0x468acf02UL) { fprintf(stderr, "ROR 27 failed\n"); err++; }
if (ROR(L32, 28) != 0x23456781UL) { fprintf(stderr, "ROR 28 failed\n"); err++; }
if (ROR(L32, 29) != 0x91a2b3c0UL) { fprintf(stderr, "ROR 29 failed\n"); err++; }
if (ROR(L32, 30) != 0x48d159e0UL) { fprintf(stderr, "ROR 30 failed\n"); err++; }
if (ROR(L32, 31) != 0x2468acf0UL) { fprintf(stderr, "ROR 31 failed\n"); err++; }
if (ROR64(L64, 0 ) != CONST64(0x1122334455667788)) { fprintf(stderr, "ROR64 0 failed\n"); err++; }
if (ROR64(L64, 1 ) != CONST64(0x089119a22ab33bc4)) { fprintf(stderr, "ROR64 1 failed\n"); err++; }
if (ROR64(L64, 2 ) != CONST64(0x04488cd115599de2)) { fprintf(stderr, "ROR64 2 failed\n"); err++; }
if (ROR64(L64, 3 ) != CONST64(0x022446688aaccef1)) { fprintf(stderr, "ROR64 3 failed\n"); err++; }
if (ROR64(L64, 4 ) != CONST64(0x8112233445566778)) { fprintf(stderr, "ROR64 4 failed\n"); err++; }
if (ROR64(L64, 5 ) != CONST64(0x4089119a22ab33bc)) { fprintf(stderr, "ROR64 5 failed\n"); err++; }
if (ROR64(L64, 6 ) != CONST64(0x204488cd115599de)) { fprintf(stderr, "ROR64 6 failed\n"); err++; }
if (ROR64(L64, 7 ) != CONST64(0x1022446688aaccef)) { fprintf(stderr, "ROR64 7 failed\n"); err++; }
if (ROR64(L64, 8 ) != CONST64(0x8811223344556677)) { fprintf(stderr, "ROR64 8 failed\n"); err++; }
if (ROR64(L64, 9 ) != CONST64(0xc4089119a22ab33b)) { fprintf(stderr, "ROR64 9 failed\n"); err++; }
if (ROR64(L64, 10) != CONST64(0xe204488cd115599d)) { fprintf(stderr, "ROR64 10 failed\n"); err++; }
if (ROR64(L64, 11) != CONST64(0xf1022446688aacce)) { fprintf(stderr, "ROR64 11 failed\n"); err++; }
if (ROR64(L64, 12) != CONST64(0x7881122334455667)) { fprintf(stderr, "ROR64 12 failed\n"); err++; }
if (ROR64(L64, 13) != CONST64(0xbc4089119a22ab33)) { fprintf(stderr, "ROR64 13 failed\n"); err++; }
if (ROR64(L64, 14) != CONST64(0xde204488cd115599)) { fprintf(stderr, "ROR64 14 failed\n"); err++; }
if (ROR64(L64, 15) != CONST64(0xef1022446688aacc)) { fprintf(stderr, "ROR64 15 failed\n"); err++; }
if (ROR64(L64, 16) != CONST64(0x7788112233445566)) { fprintf(stderr, "ROR64 16 failed\n"); err++; }
if (ROR64(L64, 17) != CONST64(0x3bc4089119a22ab3)) { fprintf(stderr, "ROR64 17 failed\n"); err++; }
if (ROR64(L64, 18) != CONST64(0x9de204488cd11559)) { fprintf(stderr, "ROR64 18 failed\n"); err++; }
if (ROR64(L64, 19) != CONST64(0xcef1022446688aac)) { fprintf(stderr, "ROR64 19 failed\n"); err++; }
if (ROR64(L64, 20) != CONST64(0x6778811223344556)) { fprintf(stderr, "ROR64 20 failed\n"); err++; }
if (ROR64(L64, 21) != CONST64(0x33bc4089119a22ab)) { fprintf(stderr, "ROR64 21 failed\n"); err++; }
if (ROR64(L64, 22) != CONST64(0x99de204488cd1155)) { fprintf(stderr, "ROR64 22 failed\n"); err++; }
if (ROR64(L64, 23) != CONST64(0xccef1022446688aa)) { fprintf(stderr, "ROR64 23 failed\n"); err++; }
if (ROR64(L64, 24) != CONST64(0x6677881122334455)) { fprintf(stderr, "ROR64 24 failed\n"); err++; }
if (ROR64(L64, 25) != CONST64(0xb33bc4089119a22a)) { fprintf(stderr, "ROR64 25 failed\n"); err++; }
if (ROR64(L64, 26) != CONST64(0x599de204488cd115)) { fprintf(stderr, "ROR64 26 failed\n"); err++; }
if (ROR64(L64, 27) != CONST64(0xaccef1022446688a)) { fprintf(stderr, "ROR64 27 failed\n"); err++; }
if (ROR64(L64, 28) != CONST64(0x5667788112233445)) { fprintf(stderr, "ROR64 28 failed\n"); err++; }
if (ROR64(L64, 29) != CONST64(0xab33bc4089119a22)) { fprintf(stderr, "ROR64 29 failed\n"); err++; }
if (ROR64(L64, 30) != CONST64(0x5599de204488cd11)) { fprintf(stderr, "ROR64 30 failed\n"); err++; }
if (ROR64(L64, 31) != CONST64(0xaaccef1022446688)) { fprintf(stderr, "ROR64 31 failed\n"); err++; }
if (ROR64(L64, 32) != CONST64(0x5566778811223344)) { fprintf(stderr, "ROR64 32 failed\n"); err++; }
if (ROR64(L64, 33) != CONST64(0x2ab33bc4089119a2)) { fprintf(stderr, "ROR64 33 failed\n"); err++; }
if (ROR64(L64, 34) != CONST64(0x15599de204488cd1)) { fprintf(stderr, "ROR64 34 failed\n"); err++; }
if (ROR64(L64, 35) != CONST64(0x8aaccef102244668)) { fprintf(stderr, "ROR64 35 failed\n"); err++; }
if (ROR64(L64, 36) != CONST64(0x4556677881122334)) { fprintf(stderr, "ROR64 36 failed\n"); err++; }
if (ROR64(L64, 37) != CONST64(0x22ab33bc4089119a)) { fprintf(stderr, "ROR64 37 failed\n"); err++; }
if (ROR64(L64, 38) != CONST64(0x115599de204488cd)) { fprintf(stderr, "ROR64 38 failed\n"); err++; }
if (ROR64(L64, 39) != CONST64(0x88aaccef10224466)) { fprintf(stderr, "ROR64 39 failed\n"); err++; }
if (ROR64(L64, 40) != CONST64(0x4455667788112233)) { fprintf(stderr, "ROR64 40 failed\n"); err++; }
if (ROR64(L64, 41) != CONST64(0xa22ab33bc4089119)) { fprintf(stderr, "ROR64 41 failed\n"); err++; }
if (ROR64(L64, 42) != CONST64(0xd115599de204488c)) { fprintf(stderr, "ROR64 42 failed\n"); err++; }
if (ROR64(L64, 43) != CONST64(0x688aaccef1022446)) { fprintf(stderr, "ROR64 43 failed\n"); err++; }
if (ROR64(L64, 44) != CONST64(0x3445566778811223)) { fprintf(stderr, "ROR64 44 failed\n"); err++; }
if (ROR64(L64, 45) != CONST64(0x9a22ab33bc408911)) { fprintf(stderr, "ROR64 45 failed\n"); err++; }
if (ROR64(L64, 46) != CONST64(0xcd115599de204488)) { fprintf(stderr, "ROR64 46 failed\n"); err++; }
if (ROR64(L64, 47) != CONST64(0x6688aaccef102244)) { fprintf(stderr, "ROR64 47 failed\n"); err++; }
if (ROR64(L64, 48) != CONST64(0x3344556677881122)) { fprintf(stderr, "ROR64 48 failed\n"); err++; }
if (ROR64(L64, 49) != CONST64(0x19a22ab33bc40891)) { fprintf(stderr, "ROR64 49 failed\n"); err++; }
if (ROR64(L64, 50) != CONST64(0x8cd115599de20448)) { fprintf(stderr, "ROR64 50 failed\n"); err++; }
if (ROR64(L64, 51) != CONST64(0x46688aaccef10224)) { fprintf(stderr, "ROR64 51 failed\n"); err++; }
if (ROR64(L64, 52) != CONST64(0x2334455667788112)) { fprintf(stderr, "ROR64 52 failed\n"); err++; }
if (ROR64(L64, 53) != CONST64(0x119a22ab33bc4089)) { fprintf(stderr, "ROR64 53 failed\n"); err++; }
if (ROR64(L64, 54) != CONST64(0x88cd115599de2044)) { fprintf(stderr, "ROR64 54 failed\n"); err++; }
if (ROR64(L64, 55) != CONST64(0x446688aaccef1022)) { fprintf(stderr, "ROR64 55 failed\n"); err++; }
if (ROR64(L64, 56) != CONST64(0x2233445566778811)) { fprintf(stderr, "ROR64 56 failed\n"); err++; }
if (ROR64(L64, 57) != CONST64(0x9119a22ab33bc408)) { fprintf(stderr, "ROR64 57 failed\n"); err++; }
if (ROR64(L64, 58) != CONST64(0x488cd115599de204)) { fprintf(stderr, "ROR64 58 failed\n"); err++; }
if (ROR64(L64, 59) != CONST64(0x2446688aaccef102)) { fprintf(stderr, "ROR64 59 failed\n"); err++; }
if (ROR64(L64, 60) != CONST64(0x1223344556677881)) { fprintf(stderr, "ROR64 60 failed\n"); err++; }
if (ROR64(L64, 61) != CONST64(0x89119a22ab33bc40)) { fprintf(stderr, "ROR64 61 failed\n"); err++; }
if (ROR64(L64, 62) != CONST64(0x4488cd115599de20)) { fprintf(stderr, "ROR64 62 failed\n"); err++; }
if (ROR64(L64, 63) != CONST64(0x22446688aaccef10)) { fprintf(stderr, "ROR64 63 failed\n"); err++; }
if (ROL(L32, 0 ) != 0x12345678UL) { fprintf(stderr, "ROL 0 failed\n"); err++; }
if (ROL(L32, 1 ) != 0x2468acf0UL) { fprintf(stderr, "ROL 1 failed\n"); err++; }
if (ROL(L32, 2 ) != 0x48d159e0UL) { fprintf(stderr, "ROL 2 failed\n"); err++; }
if (ROL(L32, 3 ) != 0x91a2b3c0UL) { fprintf(stderr, "ROL 3 failed\n"); err++; }
if (ROL(L32, 4 ) != 0x23456781UL) { fprintf(stderr, "ROL 4 failed\n"); err++; }
if (ROL(L32, 5 ) != 0x468acf02UL) { fprintf(stderr, "ROL 5 failed\n"); err++; }
if (ROL(L32, 6 ) != 0x8d159e04UL) { fprintf(stderr, "ROL 6 failed\n"); err++; }
if (ROL(L32, 7 ) != 0x1a2b3c09UL) { fprintf(stderr, "ROL 7 failed\n"); err++; }
if (ROL(L32, 8 ) != 0x34567812UL) { fprintf(stderr, "ROL 8 failed\n"); err++; }
if (ROL(L32, 9 ) != 0x68acf024UL) { fprintf(stderr, "ROL 9 failed\n"); err++; }
if (ROL(L32, 10) != 0xd159e048UL) { fprintf(stderr, "ROL 10 failed\n"); err++; }
if (ROL(L32, 11) != 0xa2b3c091UL) { fprintf(stderr, "ROL 11 failed\n"); err++; }
if (ROL(L32, 12) != 0x45678123UL) { fprintf(stderr, "ROL 12 failed\n"); err++; }
if (ROL(L32, 13) != 0x8acf0246UL) { fprintf(stderr, "ROL 13 failed\n"); err++; }
if (ROL(L32, 14) != 0x159e048dUL) { fprintf(stderr, "ROL 14 failed\n"); err++; }
if (ROL(L32, 15) != 0x2b3c091aUL) { fprintf(stderr, "ROL 15 failed\n"); err++; }
if (ROL(L32, 16) != 0x56781234UL) { fprintf(stderr, "ROL 16 failed\n"); err++; }
if (ROL(L32, 17) != 0xacf02468UL) { fprintf(stderr, "ROL 17 failed\n"); err++; }
if (ROL(L32, 18) != 0x59e048d1UL) { fprintf(stderr, "ROL 18 failed\n"); err++; }
if (ROL(L32, 19) != 0xb3c091a2UL) { fprintf(stderr, "ROL 19 failed\n"); err++; }
if (ROL(L32, 20) != 0x67812345UL) { fprintf(stderr, "ROL 20 failed\n"); err++; }
if (ROL(L32, 21) != 0xcf02468aUL) { fprintf(stderr, "ROL 21 failed\n"); err++; }
if (ROL(L32, 22) != 0x9e048d15UL) { fprintf(stderr, "ROL 22 failed\n"); err++; }
if (ROL(L32, 23) != 0x3c091a2bUL) { fprintf(stderr, "ROL 23 failed\n"); err++; }
if (ROL(L32, 24) != 0x78123456UL) { fprintf(stderr, "ROL 24 failed\n"); err++; }
if (ROL(L32, 25) != 0xf02468acUL) { fprintf(stderr, "ROL 25 failed\n"); err++; }
if (ROL(L32, 26) != 0xe048d159UL) { fprintf(stderr, "ROL 26 failed\n"); err++; }
if (ROL(L32, 27) != 0xc091a2b3UL) { fprintf(stderr, "ROL 27 failed\n"); err++; }
if (ROL(L32, 28) != 0x81234567UL) { fprintf(stderr, "ROL 28 failed\n"); err++; }
if (ROL(L32, 29) != 0x02468acfUL) { fprintf(stderr, "ROL 29 failed\n"); err++; }
if (ROL(L32, 30) != 0x048d159eUL) { fprintf(stderr, "ROL 30 failed\n"); err++; }
if (ROL(L32, 31) != 0x091a2b3cUL) { fprintf(stderr, "ROL 31 failed\n"); err++; }
if (ROL64(L64, 0 ) != CONST64(0x1122334455667788)) { fprintf(stderr, "ROL64 0 failed\n"); err++; }
if (ROL64(L64, 1 ) != CONST64(0x22446688aaccef10)) { fprintf(stderr, "ROL64 1 failed\n"); err++; }
if (ROL64(L64, 2 ) != CONST64(0x4488cd115599de20)) { fprintf(stderr, "ROL64 2 failed\n"); err++; }
if (ROL64(L64, 3 ) != CONST64(0x89119a22ab33bc40)) { fprintf(stderr, "ROL64 3 failed\n"); err++; }
if (ROL64(L64, 4 ) != CONST64(0x1223344556677881)) { fprintf(stderr, "ROL64 4 failed\n"); err++; }
if (ROL64(L64, 5 ) != CONST64(0x2446688aaccef102)) { fprintf(stderr, "ROL64 5 failed\n"); err++; }
if (ROL64(L64, 6 ) != CONST64(0x488cd115599de204)) { fprintf(stderr, "ROL64 6 failed\n"); err++; }
if (ROL64(L64, 7 ) != CONST64(0x9119a22ab33bc408)) { fprintf(stderr, "ROL64 7 failed\n"); err++; }
if (ROL64(L64, 8 ) != CONST64(0x2233445566778811)) { fprintf(stderr, "ROL64 8 failed\n"); err++; }
if (ROL64(L64, 9 ) != CONST64(0x446688aaccef1022)) { fprintf(stderr, "ROL64 9 failed\n"); err++; }
if (ROL64(L64, 10) != CONST64(0x88cd115599de2044)) { fprintf(stderr, "ROL64 10 failed\n"); err++; }
if (ROL64(L64, 11) != CONST64(0x119a22ab33bc4089)) { fprintf(stderr, "ROL64 11 failed\n"); err++; }
if (ROL64(L64, 12) != CONST64(0x2334455667788112)) { fprintf(stderr, "ROL64 12 failed\n"); err++; }
if (ROL64(L64, 13) != CONST64(0x46688aaccef10224)) { fprintf(stderr, "ROL64 13 failed\n"); err++; }
if (ROL64(L64, 14) != CONST64(0x8cd115599de20448)) { fprintf(stderr, "ROL64 14 failed\n"); err++; }
if (ROL64(L64, 15) != CONST64(0x19a22ab33bc40891)) { fprintf(stderr, "ROL64 15 failed\n"); err++; }
if (ROL64(L64, 16) != CONST64(0x3344556677881122)) { fprintf(stderr, "ROL64 16 failed\n"); err++; }
if (ROL64(L64, 17) != CONST64(0x6688aaccef102244)) { fprintf(stderr, "ROL64 17 failed\n"); err++; }
if (ROL64(L64, 18) != CONST64(0xcd115599de204488)) { fprintf(stderr, "ROL64 18 failed\n"); err++; }
if (ROL64(L64, 19) != CONST64(0x9a22ab33bc408911)) { fprintf(stderr, "ROL64 19 failed\n"); err++; }
if (ROL64(L64, 20) != CONST64(0x3445566778811223)) { fprintf(stderr, "ROL64 20 failed\n"); err++; }
if (ROL64(L64, 21) != CONST64(0x688aaccef1022446)) { fprintf(stderr, "ROL64 21 failed\n"); err++; }
if (ROL64(L64, 22) != CONST64(0xd115599de204488c)) { fprintf(stderr, "ROL64 22 failed\n"); err++; }
if (ROL64(L64, 23) != CONST64(0xa22ab33bc4089119)) { fprintf(stderr, "ROL64 23 failed\n"); err++; }
if (ROL64(L64, 24) != CONST64(0x4455667788112233)) { fprintf(stderr, "ROL64 24 failed\n"); err++; }
if (ROL64(L64, 25) != CONST64(0x88aaccef10224466)) { fprintf(stderr, "ROL64 25 failed\n"); err++; }
if (ROL64(L64, 26) != CONST64(0x115599de204488cd)) { fprintf(stderr, "ROL64 26 failed\n"); err++; }
if (ROL64(L64, 27) != CONST64(0x22ab33bc4089119a)) { fprintf(stderr, "ROL64 27 failed\n"); err++; }
if (ROL64(L64, 28) != CONST64(0x4556677881122334)) { fprintf(stderr, "ROL64 28 failed\n"); err++; }
if (ROL64(L64, 29) != CONST64(0x8aaccef102244668)) { fprintf(stderr, "ROL64 29 failed\n"); err++; }
if (ROL64(L64, 30) != CONST64(0x15599de204488cd1)) { fprintf(stderr, "ROL64 30 failed\n"); err++; }
if (ROL64(L64, 31) != CONST64(0x2ab33bc4089119a2)) { fprintf(stderr, "ROL64 31 failed\n"); err++; }
if (ROL64(L64, 32) != CONST64(0x5566778811223344)) { fprintf(stderr, "ROL64 32 failed\n"); err++; }
if (ROL64(L64, 33) != CONST64(0xaaccef1022446688)) { fprintf(stderr, "ROL64 33 failed\n"); err++; }
if (ROL64(L64, 34) != CONST64(0x5599de204488cd11)) { fprintf(stderr, "ROL64 34 failed\n"); err++; }
if (ROL64(L64, 35) != CONST64(0xab33bc4089119a22)) { fprintf(stderr, "ROL64 35 failed\n"); err++; }
if (ROL64(L64, 36) != CONST64(0x5667788112233445)) { fprintf(stderr, "ROL64 36 failed\n"); err++; }
if (ROL64(L64, 37) != CONST64(0xaccef1022446688a)) { fprintf(stderr, "ROL64 37 failed\n"); err++; }
if (ROL64(L64, 38) != CONST64(0x599de204488cd115)) { fprintf(stderr, "ROL64 38 failed\n"); err++; }
if (ROL64(L64, 39) != CONST64(0xb33bc4089119a22a)) { fprintf(stderr, "ROL64 39 failed\n"); err++; }
if (ROL64(L64, 40) != CONST64(0x6677881122334455)) { fprintf(stderr, "ROL64 40 failed\n"); err++; }
if (ROL64(L64, 41) != CONST64(0xccef1022446688aa)) { fprintf(stderr, "ROL64 41 failed\n"); err++; }
if (ROL64(L64, 42) != CONST64(0x99de204488cd1155)) { fprintf(stderr, "ROL64 42 failed\n"); err++; }
if (ROL64(L64, 43) != CONST64(0x33bc4089119a22ab)) { fprintf(stderr, "ROL64 43 failed\n"); err++; }
if (ROL64(L64, 44) != CONST64(0x6778811223344556)) { fprintf(stderr, "ROL64 44 failed\n"); err++; }
if (ROL64(L64, 45) != CONST64(0xcef1022446688aac)) { fprintf(stderr, "ROL64 45 failed\n"); err++; }
if (ROL64(L64, 46) != CONST64(0x9de204488cd11559)) { fprintf(stderr, "ROL64 46 failed\n"); err++; }
if (ROL64(L64, 47) != CONST64(0x3bc4089119a22ab3)) { fprintf(stderr, "ROL64 47 failed\n"); err++; }
if (ROL64(L64, 48) != CONST64(0x7788112233445566)) { fprintf(stderr, "ROL64 48 failed\n"); err++; }
if (ROL64(L64, 49) != CONST64(0xef1022446688aacc)) { fprintf(stderr, "ROL64 49 failed\n"); err++; }
if (ROL64(L64, 50) != CONST64(0xde204488cd115599)) { fprintf(stderr, "ROL64 50 failed\n"); err++; }
if (ROL64(L64, 51) != CONST64(0xbc4089119a22ab33)) { fprintf(stderr, "ROL64 51 failed\n"); err++; }
if (ROL64(L64, 52) != CONST64(0x7881122334455667)) { fprintf(stderr, "ROL64 52 failed\n"); err++; }
if (ROL64(L64, 53) != CONST64(0xf1022446688aacce)) { fprintf(stderr, "ROL64 53 failed\n"); err++; }
if (ROL64(L64, 54) != CONST64(0xe204488cd115599d)) { fprintf(stderr, "ROL64 54 failed\n"); err++; }
if (ROL64(L64, 55) != CONST64(0xc4089119a22ab33b)) { fprintf(stderr, "ROL64 55 failed\n"); err++; }
if (ROL64(L64, 56) != CONST64(0x8811223344556677)) { fprintf(stderr, "ROL64 56 failed\n"); err++; }
if (ROL64(L64, 57) != CONST64(0x1022446688aaccef)) { fprintf(stderr, "ROL64 57 failed\n"); err++; }
if (ROL64(L64, 58) != CONST64(0x204488cd115599de)) { fprintf(stderr, "ROL64 58 failed\n"); err++; }
if (ROL64(L64, 59) != CONST64(0x4089119a22ab33bc)) { fprintf(stderr, "ROL64 59 failed\n"); err++; }
if (ROL64(L64, 60) != CONST64(0x8112233445566778)) { fprintf(stderr, "ROL64 60 failed\n"); err++; }
if (ROL64(L64, 61) != CONST64(0x022446688aaccef1)) { fprintf(stderr, "ROL64 61 failed\n"); err++; }
if (ROL64(L64, 62) != CONST64(0x04488cd115599de2)) { fprintf(stderr, "ROL64 62 failed\n"); err++; }
if (ROL64(L64, 63) != CONST64(0x089119a22ab33bc4)) { fprintf(stderr, "ROL64 63 failed\n"); err++; }
if (RORc(L32, 0 ) != 0x12345678UL) { fprintf(stderr, "RORc 0 failed\n"); err++; }
if (RORc(L32, 1 ) != 0x091a2b3cUL) { fprintf(stderr, "RORc 1 failed\n"); err++; }
if (RORc(L32, 2 ) != 0x048d159eUL) { fprintf(stderr, "RORc 2 failed\n"); err++; }
if (RORc(L32, 3 ) != 0x02468acfUL) { fprintf(stderr, "RORc 3 failed\n"); err++; }
if (RORc(L32, 4 ) != 0x81234567UL) { fprintf(stderr, "RORc 4 failed\n"); err++; }
if (RORc(L32, 5 ) != 0xc091a2b3UL) { fprintf(stderr, "RORc 5 failed\n"); err++; }
if (RORc(L32, 6 ) != 0xe048d159UL) { fprintf(stderr, "RORc 6 failed\n"); err++; }
if (RORc(L32, 7 ) != 0xf02468acUL) { fprintf(stderr, "RORc 7 failed\n"); err++; }
if (RORc(L32, 8 ) != 0x78123456UL) { fprintf(stderr, "RORc 8 failed\n"); err++; }
if (RORc(L32, 9 ) != 0x3c091a2bUL) { fprintf(stderr, "RORc 9 failed\n"); err++; }
if (RORc(L32, 10) != 0x9e048d15UL) { fprintf(stderr, "RORc 10 failed\n"); err++; }
if (RORc(L32, 11) != 0xcf02468aUL) { fprintf(stderr, "RORc 11 failed\n"); err++; }
if (RORc(L32, 12) != 0x67812345UL) { fprintf(stderr, "RORc 12 failed\n"); err++; }
if (RORc(L32, 13) != 0xb3c091a2UL) { fprintf(stderr, "RORc 13 failed\n"); err++; }
if (RORc(L32, 14) != 0x59e048d1UL) { fprintf(stderr, "RORc 14 failed\n"); err++; }
if (RORc(L32, 15) != 0xacf02468UL) { fprintf(stderr, "RORc 15 failed\n"); err++; }
if (RORc(L32, 16) != 0x56781234UL) { fprintf(stderr, "RORc 16 failed\n"); err++; }
if (RORc(L32, 17) != 0x2b3c091aUL) { fprintf(stderr, "RORc 17 failed\n"); err++; }
if (RORc(L32, 18) != 0x159e048dUL) { fprintf(stderr, "RORc 18 failed\n"); err++; }
if (RORc(L32, 19) != 0x8acf0246UL) { fprintf(stderr, "RORc 19 failed\n"); err++; }
if (RORc(L32, 20) != 0x45678123UL) { fprintf(stderr, "RORc 20 failed\n"); err++; }
if (RORc(L32, 21) != 0xa2b3c091UL) { fprintf(stderr, "RORc 21 failed\n"); err++; }
if (RORc(L32, 22) != 0xd159e048UL) { fprintf(stderr, "RORc 22 failed\n"); err++; }
if (RORc(L32, 23) != 0x68acf024UL) { fprintf(stderr, "RORc 23 failed\n"); err++; }
if (RORc(L32, 24) != 0x34567812UL) { fprintf(stderr, "RORc 24 failed\n"); err++; }
if (RORc(L32, 25) != 0x1a2b3c09UL) { fprintf(stderr, "RORc 25 failed\n"); err++; }
if (RORc(L32, 26) != 0x8d159e04UL) { fprintf(stderr, "RORc 26 failed\n"); err++; }
if (RORc(L32, 27) != 0x468acf02UL) { fprintf(stderr, "RORc 27 failed\n"); err++; }
if (RORc(L32, 28) != 0x23456781UL) { fprintf(stderr, "RORc 28 failed\n"); err++; }
if (RORc(L32, 29) != 0x91a2b3c0UL) { fprintf(stderr, "RORc 29 failed\n"); err++; }
if (RORc(L32, 30) != 0x48d159e0UL) { fprintf(stderr, "RORc 30 failed\n"); err++; }
if (RORc(L32, 31) != 0x2468acf0UL) { fprintf(stderr, "RORc 31 failed\n"); err++; }
if (ROR64c(L64, 0 ) != CONST64(0x1122334455667788)) { fprintf(stderr, "ROR64c 0 failed\n"); err++; }
if (ROR64c(L64, 1 ) != CONST64(0x089119a22ab33bc4)) { fprintf(stderr, "ROR64c 1 failed\n"); err++; }
if (ROR64c(L64, 2 ) != CONST64(0x04488cd115599de2)) { fprintf(stderr, "ROR64c 2 failed\n"); err++; }
if (ROR64c(L64, 3 ) != CONST64(0x022446688aaccef1)) { fprintf(stderr, "ROR64c 3 failed\n"); err++; }
if (ROR64c(L64, 4 ) != CONST64(0x8112233445566778)) { fprintf(stderr, "ROR64c 4 failed\n"); err++; }
if (ROR64c(L64, 5 ) != CONST64(0x4089119a22ab33bc)) { fprintf(stderr, "ROR64c 5 failed\n"); err++; }
if (ROR64c(L64, 6 ) != CONST64(0x204488cd115599de)) { fprintf(stderr, "ROR64c 6 failed\n"); err++; }
if (ROR64c(L64, 7 ) != CONST64(0x1022446688aaccef)) { fprintf(stderr, "ROR64c 7 failed\n"); err++; }
if (ROR64c(L64, 8 ) != CONST64(0x8811223344556677)) { fprintf(stderr, "ROR64c 8 failed\n"); err++; }
if (ROR64c(L64, 9 ) != CONST64(0xc4089119a22ab33b)) { fprintf(stderr, "ROR64c 9 failed\n"); err++; }
if (ROR64c(L64, 10) != CONST64(0xe204488cd115599d)) { fprintf(stderr, "ROR64c 10 failed\n"); err++; }
if (ROR64c(L64, 11) != CONST64(0xf1022446688aacce)) { fprintf(stderr, "ROR64c 11 failed\n"); err++; }
if (ROR64c(L64, 12) != CONST64(0x7881122334455667)) { fprintf(stderr, "ROR64c 12 failed\n"); err++; }
if (ROR64c(L64, 13) != CONST64(0xbc4089119a22ab33)) { fprintf(stderr, "ROR64c 13 failed\n"); err++; }
if (ROR64c(L64, 14) != CONST64(0xde204488cd115599)) { fprintf(stderr, "ROR64c 14 failed\n"); err++; }
if (ROR64c(L64, 15) != CONST64(0xef1022446688aacc)) { fprintf(stderr, "ROR64c 15 failed\n"); err++; }
if (ROR64c(L64, 16) != CONST64(0x7788112233445566)) { fprintf(stderr, "ROR64c 16 failed\n"); err++; }
if (ROR64c(L64, 17) != CONST64(0x3bc4089119a22ab3)) { fprintf(stderr, "ROR64c 17 failed\n"); err++; }
if (ROR64c(L64, 18) != CONST64(0x9de204488cd11559)) { fprintf(stderr, "ROR64c 18 failed\n"); err++; }
if (ROR64c(L64, 19) != CONST64(0xcef1022446688aac)) { fprintf(stderr, "ROR64c 19 failed\n"); err++; }
if (ROR64c(L64, 20) != CONST64(0x6778811223344556)) { fprintf(stderr, "ROR64c 20 failed\n"); err++; }
if (ROR64c(L64, 21) != CONST64(0x33bc4089119a22ab)) { fprintf(stderr, "ROR64c 21 failed\n"); err++; }
if (ROR64c(L64, 22) != CONST64(0x99de204488cd1155)) { fprintf(stderr, "ROR64c 22 failed\n"); err++; }
if (ROR64c(L64, 23) != CONST64(0xccef1022446688aa)) { fprintf(stderr, "ROR64c 23 failed\n"); err++; }
if (ROR64c(L64, 24) != CONST64(0x6677881122334455)) { fprintf(stderr, "ROR64c 24 failed\n"); err++; }
if (ROR64c(L64, 25) != CONST64(0xb33bc4089119a22a)) { fprintf(stderr, "ROR64c 25 failed\n"); err++; }
if (ROR64c(L64, 26) != CONST64(0x599de204488cd115)) { fprintf(stderr, "ROR64c 26 failed\n"); err++; }
if (ROR64c(L64, 27) != CONST64(0xaccef1022446688a)) { fprintf(stderr, "ROR64c 27 failed\n"); err++; }
if (ROR64c(L64, 28) != CONST64(0x5667788112233445)) { fprintf(stderr, "ROR64c 28 failed\n"); err++; }
if (ROR64c(L64, 29) != CONST64(0xab33bc4089119a22)) { fprintf(stderr, "ROR64c 29 failed\n"); err++; }
if (ROR64c(L64, 30) != CONST64(0x5599de204488cd11)) { fprintf(stderr, "ROR64c 30 failed\n"); err++; }
if (ROR64c(L64, 31) != CONST64(0xaaccef1022446688)) { fprintf(stderr, "ROR64c 31 failed\n"); err++; }
if (ROR64c(L64, 32) != CONST64(0x5566778811223344)) { fprintf(stderr, "ROR64c 32 failed\n"); err++; }
if (ROR64c(L64, 33) != CONST64(0x2ab33bc4089119a2)) { fprintf(stderr, "ROR64c 33 failed\n"); err++; }
if (ROR64c(L64, 34) != CONST64(0x15599de204488cd1)) { fprintf(stderr, "ROR64c 34 failed\n"); err++; }
if (ROR64c(L64, 35) != CONST64(0x8aaccef102244668)) { fprintf(stderr, "ROR64c 35 failed\n"); err++; }
if (ROR64c(L64, 36) != CONST64(0x4556677881122334)) { fprintf(stderr, "ROR64c 36 failed\n"); err++; }
if (ROR64c(L64, 37) != CONST64(0x22ab33bc4089119a)) { fprintf(stderr, "ROR64c 37 failed\n"); err++; }
if (ROR64c(L64, 38) != CONST64(0x115599de204488cd)) { fprintf(stderr, "ROR64c 38 failed\n"); err++; }
if (ROR64c(L64, 39) != CONST64(0x88aaccef10224466)) { fprintf(stderr, "ROR64c 39 failed\n"); err++; }
if (ROR64c(L64, 40) != CONST64(0x4455667788112233)) { fprintf(stderr, "ROR64c 40 failed\n"); err++; }
if (ROR64c(L64, 41) != CONST64(0xa22ab33bc4089119)) { fprintf(stderr, "ROR64c 41 failed\n"); err++; }
if (ROR64c(L64, 42) != CONST64(0xd115599de204488c)) { fprintf(stderr, "ROR64c 42 failed\n"); err++; }
if (ROR64c(L64, 43) != CONST64(0x688aaccef1022446)) { fprintf(stderr, "ROR64c 43 failed\n"); err++; }
if (ROR64c(L64, 44) != CONST64(0x3445566778811223)) { fprintf(stderr, "ROR64c 44 failed\n"); err++; }
if (ROR64c(L64, 45) != CONST64(0x9a22ab33bc408911)) { fprintf(stderr, "ROR64c 45 failed\n"); err++; }
if (ROR64c(L64, 46) != CONST64(0xcd115599de204488)) { fprintf(stderr, "ROR64c 46 failed\n"); err++; }
if (ROR64c(L64, 47) != CONST64(0x6688aaccef102244)) { fprintf(stderr, "ROR64c 47 failed\n"); err++; }
if (ROR64c(L64, 48) != CONST64(0x3344556677881122)) { fprintf(stderr, "ROR64c 48 failed\n"); err++; }
if (ROR64c(L64, 49) != CONST64(0x19a22ab33bc40891)) { fprintf(stderr, "ROR64c 49 failed\n"); err++; }
if (ROR64c(L64, 50) != CONST64(0x8cd115599de20448)) { fprintf(stderr, "ROR64c 50 failed\n"); err++; }
if (ROR64c(L64, 51) != CONST64(0x46688aaccef10224)) { fprintf(stderr, "ROR64c 51 failed\n"); err++; }
if (ROR64c(L64, 52) != CONST64(0x2334455667788112)) { fprintf(stderr, "ROR64c 52 failed\n"); err++; }
if (ROR64c(L64, 53) != CONST64(0x119a22ab33bc4089)) { fprintf(stderr, "ROR64c 53 failed\n"); err++; }
if (ROR64c(L64, 54) != CONST64(0x88cd115599de2044)) { fprintf(stderr, "ROR64c 54 failed\n"); err++; }
if (ROR64c(L64, 55) != CONST64(0x446688aaccef1022)) { fprintf(stderr, "ROR64c 55 failed\n"); err++; }
if (ROR64c(L64, 56) != CONST64(0x2233445566778811)) { fprintf(stderr, "ROR64c 56 failed\n"); err++; }
if (ROR64c(L64, 57) != CONST64(0x9119a22ab33bc408)) { fprintf(stderr, "ROR64c 57 failed\n"); err++; }
if (ROR64c(L64, 58) != CONST64(0x488cd115599de204)) { fprintf(stderr, "ROR64c 58 failed\n"); err++; }
if (ROR64c(L64, 59) != CONST64(0x2446688aaccef102)) { fprintf(stderr, "ROR64c 59 failed\n"); err++; }
if (ROR64c(L64, 60) != CONST64(0x1223344556677881)) { fprintf(stderr, "ROR64c 60 failed\n"); err++; }
if (ROR64c(L64, 61) != CONST64(0x89119a22ab33bc40)) { fprintf(stderr, "ROR64c 61 failed\n"); err++; }
if (ROR64c(L64, 62) != CONST64(0x4488cd115599de20)) { fprintf(stderr, "ROR64c 62 failed\n"); err++; }
if (ROR64c(L64, 63) != CONST64(0x22446688aaccef10)) { fprintf(stderr, "ROR64c 63 failed\n"); err++; }
if (ROLc(L32, 0 ) != 0x12345678UL) { fprintf(stderr, "ROLc 0 failed\n"); err++; }
if (ROLc(L32, 1 ) != 0x2468acf0UL) { fprintf(stderr, "ROLc 1 failed\n"); err++; }
if (ROLc(L32, 2 ) != 0x48d159e0UL) { fprintf(stderr, "ROLc 2 failed\n"); err++; }
if (ROLc(L32, 3 ) != 0x91a2b3c0UL) { fprintf(stderr, "ROLc 3 failed\n"); err++; }
if (ROLc(L32, 4 ) != 0x23456781UL) { fprintf(stderr, "ROLc 4 failed\n"); err++; }
if (ROLc(L32, 5 ) != 0x468acf02UL) { fprintf(stderr, "ROLc 5 failed\n"); err++; }
if (ROLc(L32, 6 ) != 0x8d159e04UL) { fprintf(stderr, "ROLc 6 failed\n"); err++; }
if (ROLc(L32, 7 ) != 0x1a2b3c09UL) { fprintf(stderr, "ROLc 7 failed\n"); err++; }
if (ROLc(L32, 8 ) != 0x34567812UL) { fprintf(stderr, "ROLc 8 failed\n"); err++; }
if (ROLc(L32, 9 ) != 0x68acf024UL) { fprintf(stderr, "ROLc 9 failed\n"); err++; }
if (ROLc(L32, 10) != 0xd159e048UL) { fprintf(stderr, "ROLc 10 failed\n"); err++; }
if (ROLc(L32, 11) != 0xa2b3c091UL) { fprintf(stderr, "ROLc 11 failed\n"); err++; }
if (ROLc(L32, 12) != 0x45678123UL) { fprintf(stderr, "ROLc 12 failed\n"); err++; }
if (ROLc(L32, 13) != 0x8acf0246UL) { fprintf(stderr, "ROLc 13 failed\n"); err++; }
if (ROLc(L32, 14) != 0x159e048dUL) { fprintf(stderr, "ROLc 14 failed\n"); err++; }
if (ROLc(L32, 15) != 0x2b3c091aUL) { fprintf(stderr, "ROLc 15 failed\n"); err++; }
if (ROLc(L32, 16) != 0x56781234UL) { fprintf(stderr, "ROLc 16 failed\n"); err++; }
if (ROLc(L32, 17) != 0xacf02468UL) { fprintf(stderr, "ROLc 17 failed\n"); err++; }
if (ROLc(L32, 18) != 0x59e048d1UL) { fprintf(stderr, "ROLc 18 failed\n"); err++; }
if (ROLc(L32, 19) != 0xb3c091a2UL) { fprintf(stderr, "ROLc 19 failed\n"); err++; }
if (ROLc(L32, 20) != 0x67812345UL) { fprintf(stderr, "ROLc 20 failed\n"); err++; }
if (ROLc(L32, 21) != 0xcf02468aUL) { fprintf(stderr, "ROLc 21 failed\n"); err++; }
if (ROLc(L32, 22) != 0x9e048d15UL) { fprintf(stderr, "ROLc 22 failed\n"); err++; }
if (ROLc(L32, 23) != 0x3c091a2bUL) { fprintf(stderr, "ROLc 23 failed\n"); err++; }
if (ROLc(L32, 24) != 0x78123456UL) { fprintf(stderr, "ROLc 24 failed\n"); err++; }
if (ROLc(L32, 25) != 0xf02468acUL) { fprintf(stderr, "ROLc 25 failed\n"); err++; }
if (ROLc(L32, 26) != 0xe048d159UL) { fprintf(stderr, "ROLc 26 failed\n"); err++; }
if (ROLc(L32, 27) != 0xc091a2b3UL) { fprintf(stderr, "ROLc 27 failed\n"); err++; }
if (ROLc(L32, 28) != 0x81234567UL) { fprintf(stderr, "ROLc 28 failed\n"); err++; }
if (ROLc(L32, 29) != 0x02468acfUL) { fprintf(stderr, "ROLc 29 failed\n"); err++; }
if (ROLc(L32, 30) != 0x048d159eUL) { fprintf(stderr, "ROLc 30 failed\n"); err++; }
if (ROLc(L32, 31) != 0x091a2b3cUL) { fprintf(stderr, "ROLc 31 failed\n"); err++; }
if (ROL64c(L64, 0 ) != CONST64(0x1122334455667788)) { fprintf(stderr, "ROL64c 0 failed\n"); err++; }
if (ROL64c(L64, 1 ) != CONST64(0x22446688aaccef10)) { fprintf(stderr, "ROL64c 1 failed\n"); err++; }
if (ROL64c(L64, 2 ) != CONST64(0x4488cd115599de20)) { fprintf(stderr, "ROL64c 2 failed\n"); err++; }
if (ROL64c(L64, 3 ) != CONST64(0x89119a22ab33bc40)) { fprintf(stderr, "ROL64c 3 failed\n"); err++; }
if (ROL64c(L64, 4 ) != CONST64(0x1223344556677881)) { fprintf(stderr, "ROL64c 4 failed\n"); err++; }
if (ROL64c(L64, 5 ) != CONST64(0x2446688aaccef102)) { fprintf(stderr, "ROL64c 5 failed\n"); err++; }
if (ROL64c(L64, 6 ) != CONST64(0x488cd115599de204)) { fprintf(stderr, "ROL64c 6 failed\n"); err++; }
if (ROL64c(L64, 7 ) != CONST64(0x9119a22ab33bc408)) { fprintf(stderr, "ROL64c 7 failed\n"); err++; }
if (ROL64c(L64, 8 ) != CONST64(0x2233445566778811)) { fprintf(stderr, "ROL64c 8 failed\n"); err++; }
if (ROL64c(L64, 9 ) != CONST64(0x446688aaccef1022)) { fprintf(stderr, "ROL64c 9 failed\n"); err++; }
if (ROL64c(L64, 10) != CONST64(0x88cd115599de2044)) { fprintf(stderr, "ROL64c 10 failed\n"); err++; }
if (ROL64c(L64, 11) != CONST64(0x119a22ab33bc4089)) { fprintf(stderr, "ROL64c 11 failed\n"); err++; }
if (ROL64c(L64, 12) != CONST64(0x2334455667788112)) { fprintf(stderr, "ROL64c 12 failed\n"); err++; }
if (ROL64c(L64, 13) != CONST64(0x46688aaccef10224)) { fprintf(stderr, "ROL64c 13 failed\n"); err++; }
if (ROL64c(L64, 14) != CONST64(0x8cd115599de20448)) { fprintf(stderr, "ROL64c 14 failed\n"); err++; }
if (ROL64c(L64, 15) != CONST64(0x19a22ab33bc40891)) { fprintf(stderr, "ROL64c 15 failed\n"); err++; }
if (ROL64c(L64, 16) != CONST64(0x3344556677881122)) { fprintf(stderr, "ROL64c 16 failed\n"); err++; }
if (ROL64c(L64, 17) != CONST64(0x6688aaccef102244)) { fprintf(stderr, "ROL64c 17 failed\n"); err++; }
if (ROL64c(L64, 18) != CONST64(0xcd115599de204488)) { fprintf(stderr, "ROL64c 18 failed\n"); err++; }
if (ROL64c(L64, 19) != CONST64(0x9a22ab33bc408911)) { fprintf(stderr, "ROL64c 19 failed\n"); err++; }
if (ROL64c(L64, 20) != CONST64(0x3445566778811223)) { fprintf(stderr, "ROL64c 20 failed\n"); err++; }
if (ROL64c(L64, 21) != CONST64(0x688aaccef1022446)) { fprintf(stderr, "ROL64c 21 failed\n"); err++; }
if (ROL64c(L64, 22) != CONST64(0xd115599de204488c)) { fprintf(stderr, "ROL64c 22 failed\n"); err++; }
if (ROL64c(L64, 23) != CONST64(0xa22ab33bc4089119)) { fprintf(stderr, "ROL64c 23 failed\n"); err++; }
if (ROL64c(L64, 24) != CONST64(0x4455667788112233)) { fprintf(stderr, "ROL64c 24 failed\n"); err++; }
if (ROL64c(L64, 25) != CONST64(0x88aaccef10224466)) { fprintf(stderr, "ROL64c 25 failed\n"); err++; }
if (ROL64c(L64, 26) != CONST64(0x115599de204488cd)) { fprintf(stderr, "ROL64c 26 failed\n"); err++; }
if (ROL64c(L64, 27) != CONST64(0x22ab33bc4089119a)) { fprintf(stderr, "ROL64c 27 failed\n"); err++; }
if (ROL64c(L64, 28) != CONST64(0x4556677881122334)) { fprintf(stderr, "ROL64c 28 failed\n"); err++; }
if (ROL64c(L64, 29) != CONST64(0x8aaccef102244668)) { fprintf(stderr, "ROL64c 29 failed\n"); err++; }
if (ROL64c(L64, 30) != CONST64(0x15599de204488cd1)) { fprintf(stderr, "ROL64c 30 failed\n"); err++; }
if (ROL64c(L64, 31) != CONST64(0x2ab33bc4089119a2)) { fprintf(stderr, "ROL64c 31 failed\n"); err++; }
if (ROL64c(L64, 32) != CONST64(0x5566778811223344)) { fprintf(stderr, "ROL64c 32 failed\n"); err++; }
if (ROL64c(L64, 33) != CONST64(0xaaccef1022446688)) { fprintf(stderr, "ROL64c 33 failed\n"); err++; }
if (ROL64c(L64, 34) != CONST64(0x5599de204488cd11)) { fprintf(stderr, "ROL64c 34 failed\n"); err++; }
if (ROL64c(L64, 35) != CONST64(0xab33bc4089119a22)) { fprintf(stderr, "ROL64c 35 failed\n"); err++; }
if (ROL64c(L64, 36) != CONST64(0x5667788112233445)) { fprintf(stderr, "ROL64c 36 failed\n"); err++; }
if (ROL64c(L64, 37) != CONST64(0xaccef1022446688a)) { fprintf(stderr, "ROL64c 37 failed\n"); err++; }
if (ROL64c(L64, 38) != CONST64(0x599de204488cd115)) { fprintf(stderr, "ROL64c 38 failed\n"); err++; }
if (ROL64c(L64, 39) != CONST64(0xb33bc4089119a22a)) { fprintf(stderr, "ROL64c 39 failed\n"); err++; }
if (ROL64c(L64, 40) != CONST64(0x6677881122334455)) { fprintf(stderr, "ROL64c 40 failed\n"); err++; }
if (ROL64c(L64, 41) != CONST64(0xccef1022446688aa)) { fprintf(stderr, "ROL64c 41 failed\n"); err++; }
if (ROL64c(L64, 42) != CONST64(0x99de204488cd1155)) { fprintf(stderr, "ROL64c 42 failed\n"); err++; }
if (ROL64c(L64, 43) != CONST64(0x33bc4089119a22ab)) { fprintf(stderr, "ROL64c 43 failed\n"); err++; }
if (ROL64c(L64, 44) != CONST64(0x6778811223344556)) { fprintf(stderr, "ROL64c 44 failed\n"); err++; }
if (ROL64c(L64, 45) != CONST64(0xcef1022446688aac)) { fprintf(stderr, "ROL64c 45 failed\n"); err++; }
if (ROL64c(L64, 46) != CONST64(0x9de204488cd11559)) { fprintf(stderr, "ROL64c 46 failed\n"); err++; }
if (ROL64c(L64, 47) != CONST64(0x3bc4089119a22ab3)) { fprintf(stderr, "ROL64c 47 failed\n"); err++; }
if (ROL64c(L64, 48) != CONST64(0x7788112233445566)) { fprintf(stderr, "ROL64c 48 failed\n"); err++; }
if (ROL64c(L64, 49) != CONST64(0xef1022446688aacc)) { fprintf(stderr, "ROL64c 49 failed\n"); err++; }
if (ROL64c(L64, 50) != CONST64(0xde204488cd115599)) { fprintf(stderr, "ROL64c 50 failed\n"); err++; }
if (ROL64c(L64, 51) != CONST64(0xbc4089119a22ab33)) { fprintf(stderr, "ROL64c 51 failed\n"); err++; }
if (ROL64c(L64, 52) != CONST64(0x7881122334455667)) { fprintf(stderr, "ROL64c 52 failed\n"); err++; }
if (ROL64c(L64, 53) != CONST64(0xf1022446688aacce)) { fprintf(stderr, "ROL64c 53 failed\n"); err++; }
if (ROL64c(L64, 54) != CONST64(0xe204488cd115599d)) { fprintf(stderr, "ROL64c 54 failed\n"); err++; }
if (ROL64c(L64, 55) != CONST64(0xc4089119a22ab33b)) { fprintf(stderr, "ROL64c 55 failed\n"); err++; }
if (ROL64c(L64, 56) != CONST64(0x8811223344556677)) { fprintf(stderr, "ROL64c 56 failed\n"); err++; }
if (ROL64c(L64, 57) != CONST64(0x1022446688aaccef)) { fprintf(stderr, "ROL64c 57 failed\n"); err++; }
if (ROL64c(L64, 58) != CONST64(0x204488cd115599de)) { fprintf(stderr, "ROL64c 58 failed\n"); err++; }
if (ROL64c(L64, 59) != CONST64(0x4089119a22ab33bc)) { fprintf(stderr, "ROL64c 59 failed\n"); err++; }
if (ROL64c(L64, 60) != CONST64(0x8112233445566778)) { fprintf(stderr, "ROL64c 60 failed\n"); err++; }
if (ROL64c(L64, 61) != CONST64(0x022446688aaccef1)) { fprintf(stderr, "ROL64c 61 failed\n"); err++; }
if (ROL64c(L64, 62) != CONST64(0x04488cd115599de2)) { fprintf(stderr, "ROL64c 62 failed\n"); err++; }
if (ROL64c(L64, 63) != CONST64(0x089119a22ab33bc4)) { fprintf(stderr, "ROL64c 63 failed\n"); err++; }
return err;
}
