

#include <r_util.h>

static const char *ascii_table =
"The following table contains the 128 ASCII characters.\n"
"\n"
"Oct Dec Hex Char Oct Dec Hex Char\n"
"────────────────────────────────────────────────────────────────────────\n"
"000 0 00 NUL '\\0' (null character) 100 64 40 @\n"
"001 1 01 SOH (start of heading) 101 65 41 A\n"
"002 2 02 STX (start of text) 102 66 42 B\n"
"003 3 03 ETX (end of text) 103 67 43 C\n"
"004 4 04 EOT (end of transmission) 104 68 44 D\n"
"005 5 05 ENQ (enquiry) 105 69 45 E\n"
"006 6 06 ACK (acknowledge) 106 70 46 F\n"
"007 7 07 BEL '\\a' (bell) 107 71 47 G\n"
"010 8 08 BS '\\b' (backspace) 110 72 48 H\n"
"011 9 09 HT '\\t' (horizontal tab) 111 73 49 I\n"
"012 10 0A LF '\\n"
"' (new line) 112 74 4A J\n"
"013 11 0B VT '\\v' (vertical tab) 113 75 4B K\n"
"014 12 0C FF '\\f' (form feed) 114 76 4C L\n"
"015 13 0D CR '\\r' (carriage ret) 115 77 4D M\n"
"016 14 0E SO (shift out) 116 78 4E N\n"
"017 15 0F SI (shift in) 117 79 4F O\n"
"020 16 10 DLE (data link escape) 120 80 50 P\n"
"021 17 11 DC1 (device control 1) 121 81 51 Q\n"
"022 18 12 DC2 (device control 2) 122 82 52 R\n"
"023 19 13 DC3 (device control 3) 123 83 53 S\n"
"024 20 14 DC4 (device control 4) 124 84 54 T\n"
"025 21 15 NAK (negative ack.) 125 85 55 U\n"
"026 22 16 SYN (synchronous idle) 126 86 56 V\n"
"027 23 17 ETB (end of trans. blk) 127 87 57 W\n"
"030 24 18 CAN (cancel) 130 88 58 X\n"
"031 25 19 EM (end of medium) 131 89 59 Y\n"
"032 26 1A SUB (substitute) 132 90 5A Z\n"
"033 27 1B ESC (escape) 133 91 5B [\n"
"034 28 1C FS (file separator) 134 92 5C \\ '\\\\'\n"
"035 29 1D GS (group separator) 135 93 5D ]\n"
"036 30 1E RS (record separator) 136 94 5E ^\n"
"037 31 1F US (unit separator) 137 95 5F _\n"
"040 32 20 SPACE 140 96 60 `\n"
"041 33 21 ! 141 97 61 a\n"
"042 34 22 \" 142 98 62 b\n"
"043 35 23 #143 99 63 c\n"
"044 36 24 $ 144 100 64 d\n"
"045 37 25 % 145 101 65 e\n"
"046 38 26 & 146 102 66 f\n"
"047 39 27 ' 147 103 67 g\n"
"050 40 28 ( 150 104 68 h\n"
"051 41 29 ) 151 105 69 i\n"
"052 42 2A * 152 106 6A j\n"
"053 43 2B + 153 107 6B k\n"
"054 44 2C , 154 108 6C l\n"
"055 45 2D - 155 109 6D m\n"
"056 46 2E . 156 110 6E n\n"
"057 47 2F / 157 111 6F o\n"
"060 48 30 0 160 112 70 p\n"
"061 49 31 1 161 113 71 q\n"
"062 50 32 2 162 114 72 r\n"
"063 51 33 3 163 115 73 s\n"
"064 52 34 4 164 116 74 t\n"
"065 53 35 5 165 117 75 u\n"
"066 54 36 6 166 118 76 v\n"
"067 55 37 7 167 119 77 w\n"
"070 56 38 8 170 120 78 x\n"
"071 57 39 9 171 121 79 y\n"
"072 58 3A : 172 122 7A z\n"
"073 59 3B ; 173 123 7B {\n"
"074 60 3C < 174 124 7C |\n"
"075 61 3D = 175 125 7D }\n"
"076 62 3E > 176 126 7E ~\n"
"077 63 3F ? 177 127 7F DEL\n"
;

R_API const char *ret_ascii_table() {
return ascii_table;
}
