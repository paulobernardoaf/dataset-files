#include "sysdep.h"
#include "aarch64-dis.h"
static int
aarch64_opcode_lookup_1 (uint32_t word)
{
if (((word >> 26) & 0x1) == 0)
{
if (((word >> 25) & 0x1) == 0)
{
if (((word >> 27) & 0x1) == 0)
{
if (((word >> 24) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 1155;
}
else
{
return 1156;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 12;
}
else
{
return 16;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 14;
}
else
{
return 17;
}
}
}
}
else
{
if (((word >> 28) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 910;
}
else
{
return 916;
}
}
else
{
return 922;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 990;
}
else
{
return 924;
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 911;
}
else
{
return 917;
}
}
else
{
return 923;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 992;
}
else
{
return 925;
}
}
}
}
else
{
return 941;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 939;
}
else
{
return 938;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 978;
}
else
{
return 979;
}
}
else
{
return 980;
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 914;
}
else
{
return 920;
}
}
else
{
return 930;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 982;
}
else
{
return 985;
}
}
else
{
return 988;
}
}
}
}
else
{
return 950;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 912;
}
else
{
return 918;
}
}
else
{
return 926;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 991;
}
else
{
return 928;
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 913;
}
else
{
return 919;
}
}
else
{
return 927;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 993;
}
else
{
return 929;
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 942;
}
else
{
return 949;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 936;
}
else
{
return 937;
}
}
else
{
return 935;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 981;
}
else
{
return 984;
}
}
else
{
return 987;
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 915;
}
else
{
return 921;
}
}
else
{
return 931;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 983;
}
else
{
return 986;
}
}
else
{
return 989;
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 951;
}
else
{
return 954;
}
}
}
}
}
else
{
if (((word >> 24) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 955;
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 957;
}
else
{
return 958;
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 896;
}
else
{
return 901;
}
}
else
{
return 904;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 897;
}
else
{
return 902;
}
}
else
{
return 905;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 898;
}
else
{
return 906;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 903;
}
else
{
return 907;
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1006;
}
else
{
return 1007;
}
}
else
{
return 1008;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1009;
}
else
{
return 1012;
}
}
else
{
return 1015;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1010;
}
else
{
return 1013;
}
}
else
{
return 1016;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1011;
}
else
{
return 1014;
}
}
else
{
return 1017;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 994;
}
else
{
return 995;
}
}
else
{
return 996;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 997;
}
else
{
return 1000;
}
}
else
{
return 1003;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 998;
}
else
{
return 1001;
}
}
else
{
return 1004;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 999;
}
else
{
return 1002;
}
}
else
{
return 1005;
}
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1054;
}
else
{
return 1055;
}
}
else
{
return 1056;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1057;
}
else
{
return 1060;
}
}
else
{
return 1063;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1058;
}
else
{
return 1061;
}
}
else
{
return 1064;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1059;
}
else
{
return 1062;
}
}
else
{
return 1065;
}
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 932;
}
else
{
return 933;
}
}
else
{
return 934;
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1030;
}
else
{
return 1031;
}
}
else
{
return 1032;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1033;
}
else
{
return 1036;
}
}
else
{
return 1039;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1034;
}
else
{
return 1037;
}
}
else
{
return 1040;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1035;
}
else
{
return 1038;
}
}
else
{
return 1041;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1078;
}
else
{
return 1079;
}
}
else
{
return 1080;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1081;
}
else
{
return 1084;
}
}
else
{
return 1087;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1082;
}
else
{
return 1085;
}
}
else
{
return 1088;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1083;
}
else
{
return 1086;
}
}
else
{
return 1089;
}
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1018;
}
else
{
return 1019;
}
}
else
{
return 1020;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1021;
}
else
{
return 1024;
}
}
else
{
return 1027;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1022;
}
else
{
return 1025;
}
}
else
{
return 1028;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1023;
}
else
{
return 1026;
}
}
else
{
return 1029;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1066;
}
else
{
return 1067;
}
}
else
{
return 1068;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1069;
}
else
{
return 1072;
}
}
else
{
return 1075;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1070;
}
else
{
return 1073;
}
}
else
{
return 1076;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1071;
}
else
{
return 1074;
}
}
else
{
return 1077;
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1042;
}
else
{
return 1043;
}
}
else
{
return 1044;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1045;
}
else
{
return 1048;
}
}
else
{
return 1051;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1046;
}
else
{
return 1049;
}
}
else
{
return 1052;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1047;
}
else
{
return 1050;
}
}
else
{
return 1053;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1090;
}
else
{
return 1091;
}
}
else
{
return 1092;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1093;
}
else
{
return 1096;
}
}
else
{
return 1099;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1094;
}
else
{
return 1097;
}
}
else
{
return 1100;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1095;
}
else
{
return 1098;
}
}
else
{
return 1101;
}
}
}
}
}
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 887;
}
else
{
return 890;
}
}
else
{
return 893;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 888;
}
else
{
return 891;
}
}
else
{
return 894;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 889;
}
else
{
return 895;
}
}
else
{
return 892;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 875;
}
else
{
return 880;
}
}
else
{
return 883;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 876;
}
else
{
return 881;
}
}
else
{
return 884;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 877;
}
else
{
return 885;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 882;
}
else
{
return 886;
}
}
}
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 852;
}
else
{
return 857;
}
}
else
{
return 860;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 853;
}
else
{
return 858;
}
}
else
{
return 861;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 854;
}
else
{
return 862;
}
}
else
{
return 859;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 908;
}
else
{
return 909;
}
}
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 2046;
}
else
{
return 2054;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 2050;
}
else
{
return 2057;
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 863;
}
else
{
return 868;
}
}
else
{
return 871;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 2047;
}
else
{
return 2055;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 2051;
}
else
{
return 2058;
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 864;
}
else
{
return 869;
}
}
else
{
return 872;
}
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 2049;
}
else
{
return 2056;
}
}
else
{
return 2053;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 2048;
}
else
{
return 2052;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 865;
}
else
{
return 873;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 870;
}
else
{
return 874;
}
}
}
}
}
}
}
}
else
{
if (((word >> 24) & 0x1) == 0)
{
if (((word >> 27) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 959;
}
else
{
return 963;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 961;
}
else
{
return 964;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1150;
}
else
{
return 1152;
}
}
else
{
return 1154;
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 28) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 966;
}
else
{
return 973;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 968;
}
else
{
return 975;
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 0;
}
else
{
return 2;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 1;
}
else
{
return 4;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 656;
}
else
{
return 660;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 654;
}
else
{
return 655;
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
return 679;
}
else
{
return 687;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 716;
}
else
{
return 703;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 707;
}
else
{
return 695;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
return 684;
}
else
{
return 691;
}
}
else
{
return 720;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 715;
}
else
{
return 699;
}
}
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 2044;
}
else
{
return 2045;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 652;
}
else
{
return 653;
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 705;
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 681;
}
else
{
return 686;
}
}
else
{
return 689;
}
}
}
else
{
return 718;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 711;
}
else
{
return 697;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 693;
}
else
{
return 722;
}
}
else
{
return 701;
}
}
}
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 2043;
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 657;
}
else
{
return 663;
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
return 680;
}
else
{
return 688;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 717;
}
else
{
return 704;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 709;
}
else
{
return 696;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
return 685;
}
else
{
return 692;
}
}
else
{
return 721;
}
}
else
{
return 700;
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 706;
}
else
{
if (((word >> 16) & 0x1) == 0)
{
return 682;
}
else
{
return 690;
}
}
}
else
{
return 719;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 713;
}
else
{
return 698;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 694;
}
else
{
return 723;
}
}
else
{
return 702;
}
}
}
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 967;
}
else
{
return 974;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 971;
}
else
{
return 977;
}
}
}
}
}
else
{
if (((word >> 27) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 612;
}
else
{
return 623;
}
}
else
{
return 619;
}
}
else
{
return 746;
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 28) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 19;
}
else
{
return 22;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 20;
}
else
{
return 24;
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 724;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 732;
}
else
{
return 737;
}
}
}
else
{
return 726;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 28) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 6;
}
else
{
return 9;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 7;
}
else
{
return 10;
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
return 728;
}
else
{
return 730;
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
return 733;
}
else
{
return 735;
}
}
}
}
}
}
}
else
{
if (((word >> 27) & 0x1) == 0)
{
if (((word >> 28) & 0x1) == 0)
{
if (((word >> 24) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1235;
}
else
{
return 1704;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1783;
}
else
{
return 1715;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 1774;
}
else
{
return 1765;
}
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1788;
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1786;
}
else
{
return 1243;
}
}
}
else
{
return 1775;
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1904;
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1932;
}
else
{
return 1330;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 1926;
}
else
{
return 1917;
}
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1906;
}
else
{
return 1937;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1935;
}
else
{
return 1255;
}
}
}
else
{
return 1927;
}
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1517;
}
else
{
return 1528;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1521;
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 10) & 0x1) == 0)
{
return 1776;
}
else
{
return 1928;
}
}
else
{
return 1532;
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 1233;
}
else
{
return 1790;
}
}
else
{
return 1820;
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 1902;
}
else
{
return 1938;
}
}
else
{
return 1968;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1723;
}
else
{
return 1529;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1724;
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1777;
}
else
{
return 1778;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1929;
}
else
{
return 1930;
}
}
}
else
{
return 1533;
}
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1251;
}
else
{
return 1249;
}
}
else
{
return 1250;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1252;
}
else
{
return 1253;
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1695;
}
else
{
return 1693;
}
}
else
{
return 1694;
}
}
else
{
return 1696;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1689;
}
else
{
return 1687;
}
}
else
{
return 1688;
}
}
else
{
return 1690;
}
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 1247;
}
else
{
return 1248;
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 1691;
}
else
{
return 1692;
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
return 1685;
}
else
{
return 1686;
}
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1523;
}
else
{
return 1536;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1493;
}
else
{
return 1514;
}
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1522;
}
else
{
return 1534;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1527;
}
else
{
return 1539;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1725;
}
else
{
return 1535;
}
}
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 1698;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1459;
}
else
{
return 1479;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1464;
}
else
{
return 1484;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 10) & 0x1) == 0)
{
return 1450;
}
else
{
return 1451;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1237;
}
else
{
return 1759;
}
}
else
{
return 1236;
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
return 1452;
}
else
{
return 1449;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1743;
}
else
{
return 1480;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1745;
}
else
{
return 1485;
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 1697;
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1817;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1811;
}
else
{
return 1808;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1447;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1443;
}
else
{
return 1441;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1803;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1797;
}
else
{
return 1794;
}
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1965;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1959;
}
else
{
return 1956;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1322;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1318;
}
else
{
return 1316;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1951;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1945;
}
else
{
return 1942;
}
}
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1722;
}
else
{
return 1737;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1466;
}
else
{
return 1488;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1495;
}
else
{
return 1499;
}
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1465;
}
else
{
return 1486;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1471;
}
else
{
return 1492;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1746;
}
else
{
return 1487;
}
}
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
return 1766;
}
else
{
return 1918;
}
}
else
{
return 1701;
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1784;
}
else
{
return 1718;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1933;
}
else
{
return 1333;
}
}
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1787;
}
else
{
return 1246;
}
}
else
{
return 1936;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1617;
}
else
{
return 1628;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1624;
}
else
{
return 1634;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1241;
}
else
{
return 1713;
}
}
else
{
return 1736;
}
}
else
{
return 1738;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1328;
}
else
{
return 1254;
}
}
else
{
return 1629;
}
}
else
{
return 1635;
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1909;
}
else
{
return 1274;
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 1911;
}
else
{
return 1336;
}
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1910;
}
else
{
return 1303;
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 1232;
}
else
{
return 1710;
}
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1972;
}
else
{
return 1275;
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 1974;
}
else
{
return 1413;
}
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1973;
}
else
{
return 1302;
}
}
else
{
return 1707;
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1238;
}
else
{
return 1239;
}
}
else
{
return 1240;
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
return 1439;
}
else
{
return 1383;
}
}
else
{
return 1700;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1626;
}
else
{
return 1638;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1494;
}
else
{
return 1498;
}
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1625;
}
else
{
return 1636;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1627;
}
else
{
return 1639;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1739;
}
else
{
return 1637;
}
}
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 1699;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1583;
}
else
{
return 1603;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1592;
}
else
{
return 1611;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 1729;
}
else
{
return 1731;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 1604;
}
else
{
return 1612;
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 1702;
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1304;
}
else
{
return 1308;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1306;
}
else
{
return 1305;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1440;
}
else
{
return 1448;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1444;
}
else
{
return 1442;
}
}
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1807;
}
else
{
return 1819;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1813;
}
else
{
return 1810;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1806;
}
else
{
return 1818;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1812;
}
else
{
return 1809;
}
}
}
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1793;
}
else
{
return 1805;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1799;
}
else
{
return 1796;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1792;
}
else
{
return 1804;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1798;
}
else
{
return 1795;
}
}
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1315;
}
else
{
return 1323;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1319;
}
else
{
return 1317;
}
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1954;
}
else
{
return 1966;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1960;
}
else
{
return 1957;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1955;
}
else
{
return 1967;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1961;
}
else
{
return 1958;
}
}
}
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1940;
}
else
{
return 1952;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1946;
}
else
{
return 1943;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1941;
}
else
{
return 1953;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1947;
}
else
{
return 1944;
}
}
}
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1726;
}
else
{
return 1740;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1594;
}
else
{
return 1615;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1496;
}
else
{
return 1500;
}
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1728;
}
else
{
return 1742;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1593;
}
else
{
return 1613;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1595;
}
else
{
return 1616;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1732;
}
else
{
return 1614;
}
}
}
}
}
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1288;
}
else
{
return 1285;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1502;
}
else
{
return 1506;
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1279;
}
else
{
return 1282;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1460;
}
else
{
return 1540;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1462;
}
else
{
return 1482;
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1276;
}
else
{
return 1299;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1501;
}
else
{
return 1505;
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1297;
}
else
{
return 1291;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1584;
}
else
{
return 1640;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1588;
}
else
{
return 1607;
}
}
}
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 1345;
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1822;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1826;
}
else
{
return 1847;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1892;
}
else
{
return 1896;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1876;
}
else
{
return 1880;
}
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1280;
}
else
{
return 1283;
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1289;
}
else
{
return 1286;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1675;
}
else
{
return 1679;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1567;
}
else
{
return 1571;
}
}
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 1344;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1823;
}
else
{
return 1842;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1827;
}
else
{
return 1848;
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1277;
}
else
{
return 1300;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1467;
}
else
{
return 1545;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1469;
}
else
{
return 1490;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1659;
}
else
{
return 1672;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1661;
}
else
{
return 1665;
}
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1824;
}
else
{
return 1843;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1831;
}
else
{
return 1852;
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1293;
}
else
{
return 1295;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1676;
}
else
{
return 1680;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1568;
}
else
{
return 1572;
}
}
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1829;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1832;
}
else
{
return 1853;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1893;
}
else
{
return 1897;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1877;
}
else
{
return 1881;
}
}
}
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1290;
}
else
{
return 1287;
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1461;
}
else
{
return 1481;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1463;
}
else
{
return 1483;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1559;
}
else
{
return 1563;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1575;
}
else
{
return 1579;
}
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1398;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1399;
}
else
{
return 1400;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1402;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1403;
}
else
{
return 1404;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1346;
}
else
{
return 1347;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1825;
}
else
{
return 1844;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1828;
}
else
{
return 1849;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1845;
}
else
{
return 1850;
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1294;
}
else
{
return 1296;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1468;
}
else
{
return 1489;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1470;
}
else
{
return 1491;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1660;
}
else
{
return 1664;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1662;
}
else
{
return 1666;
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1586;
}
else
{
return 1605;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1590;
}
else
{
return 1609;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1560;
}
else
{
return 1564;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1576;
}
else
{
return 1580;
}
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1409;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1410;
}
else
{
return 1411;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1846;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1833;
}
else
{
return 1854;
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1868;
}
else
{
return 1872;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1884;
}
else
{
return 1888;
}
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1830;
}
else
{
return 1851;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1834;
}
else
{
return 1855;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1869;
}
else
{
return 1873;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1885;
}
else
{
return 1889;
}
}
}
}
}
}
}
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1714;
}
else
{
return 1242;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1329;
}
else
{
return 1327;
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
return 1312;
}
else
{
return 1359;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
return 1334;
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1310;
}
else
{
return 1268;
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 1762;
}
else
{
return 1789;
}
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1456;
}
else
{
return 1269;
}
}
else
{
return 1764;
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1309;
}
else
{
return 1271;
}
}
else
{
return 1763;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1458;
}
else
{
return 1272;
}
}
else
{
return 1755;
}
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 1325;
}
else
{
return 1912;
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1324;
}
else
{
return 1908;
}
}
else
{
return 1761;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1453;
}
else
{
return 1454;
}
}
}
else
{
return 1971;
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
return 1907;
}
else
{
return 1970;
}
}
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1455;
}
else
{
return 1270;
}
}
else
{
return 1311;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1457;
}
else
{
return 1273;
}
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1988;
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
return 1754;
}
else
{
return 1760;
}
}
else
{
return 1753;
}
}
}
else
{
return 1989;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 1913;
}
else
{
return 1914;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 1975;
}
else
{
return 1976;
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
return 1990;
}
else
{
return 1991;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 1915;
}
else
{
return 1916;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 1977;
}
else
{
return 1978;
}
}
}
}
else
{
return 1779;
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1683;
}
else
{
return 1727;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1512;
}
else
{
return 1509;
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 1547;
}
else
{
return 1548;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1684;
}
else
{
return 1748;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1744;
}
else
{
return 1730;
}
}
else
{
return 1555;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1515;
}
else
{
return 1511;
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
return 1741;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1513;
}
else
{
return 1510;
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 1647;
}
else
{
return 1648;
}
}
else
{
return 1734;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1747;
}
else
{
return 1733;
}
}
else
{
return 1657;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1516;
}
else
{
return 1497;
}
}
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 1541;
}
else
{
return 1542;
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
return 1543;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1546;
}
else
{
return 1544;
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1551;
}
else
{
return 1473;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1552;
}
else
{
return 1474;
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1553;
}
else
{
return 1475;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1558;
}
else
{
return 1478;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1554;
}
else
{
return 1476;
}
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 1642;
}
else
{
return 1643;
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
return 1644;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1646;
}
else
{
return 1645;
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1653;
}
else
{
return 1598;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1654;
}
else
{
return 1599;
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1749;
}
else
{
return 1735;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1655;
}
else
{
return 1600;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1658;
}
else
{
return 1602;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1656;
}
else
{
return 1601;
}
}
}
}
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1281;
}
else
{
return 1284;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1508;
}
else
{
return 1504;
}
}
}
else
{
if (((word >> 4) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 1985;
}
else
{
return 1986;
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
return 1981;
}
else
{
return 1982;
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 1979;
}
else
{
return 1980;
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
return 1983;
}
else
{
return 1984;
}
}
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
return 1339;
}
else
{
if (((word >> 12) & 0x1) == 0)
{
return 1406;
}
else
{
return 1419;
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
return 1432;
}
else
{
if (((word >> 12) & 0x1) == 0)
{
return 1438;
}
else
{
return 1429;
}
}
}
}
else
{
return 1397;
}
}
else
{
return 1900;
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1298;
}
else
{
return 1292;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1507;
}
else
{
return 1503;
}
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1343;
}
else
{
if (((word >> 4) & 0x1) == 0)
{
return 1350;
}
else
{
return 1352;
}
}
}
else
{
return 1342;
}
}
else
{
return 1389;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
return 1348;
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 1390;
}
else
{
return 1418;
}
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1355;
}
else
{
return 1354;
}
}
else
{
return 1395;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
return 1356;
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 1396;
}
else
{
return 1428;
}
}
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1313;
}
else
{
return 1314;
}
}
else
{
return 1401;
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 9) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1244;
}
else
{
return 1256;
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 1258;
}
else
{
return 1262;
}
}
}
else
{
if (((word >> 4) & 0x1) == 0)
{
return 1331;
}
else
{
return 1780;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 1530;
}
else
{
return 1630;
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 9) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1716;
}
else
{
return 1711;
}
}
else
{
return 1260;
}
}
else
{
if (((word >> 4) & 0x1) == 0)
{
return 1708;
}
else
{
return 1705;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 1518;
}
else
{
return 1618;
}
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
if (((word >> 9) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1245;
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 1259;
}
else
{
return 1263;
}
}
}
else
{
return 1332;
}
}
else
{
return 1257;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 1549;
}
else
{
return 1649;
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
if (((word >> 9) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1717;
}
else
{
return 1261;
}
}
else
{
return 1709;
}
}
else
{
if (((word >> 9) & 0x1) == 0)
{
return 1712;
}
else
{
return 1706;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 1520;
}
else
{
return 1622;
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1531;
}
else
{
return 1519;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1550;
}
else
{
return 1472;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1632;
}
else
{
return 1620;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1651;
}
else
{
return 1596;
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1351;
}
else
{
return 1353;
}
}
else
{
return 1415;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1901;
}
else
{
if (((word >> 21) & 0x1) == 0)
{
return 1860;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1862;
}
else
{
return 1839;
}
}
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1349;
}
else
{
return 1357;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1898;
}
else
{
return 1894;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1882;
}
else
{
return 1878;
}
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 1416;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1874;
}
else
{
return 1870;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1890;
}
else
{
return 1886;
}
}
}
}
}
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 1278;
}
else
{
return 1301;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1537;
}
else
{
return 1524;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1556;
}
else
{
return 1526;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1670;
}
else
{
return 1667;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1673;
}
else
{
return 1669;
}
}
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1264;
}
else
{
return 1265;
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1750;
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
return 1720;
}
else
{
return 1751;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1757;
}
else
{
return 1758;
}
}
}
else
{
return 1719;
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 1752;
}
else
{
return 1756;
}
}
else
{
return 1721;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1266;
}
else
{
return 1267;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1681;
}
else
{
return 1677;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1573;
}
else
{
return 1569;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1682;
}
else
{
return 1678;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1574;
}
else
{
return 1570;
}
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
return 1340;
}
else
{
return 1387;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
return 1407;
}
else
{
return 1385;
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
return 1433;
}
else
{
return 1393;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
return 1435;
}
else
{
return 1391;
}
}
}
}
else
{
return 1437;
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1335;
}
else
{
return 1341;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1381;
}
else
{
return 1388;
}
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1412;
}
else
{
return 1408;
}
}
else
{
return 1386;
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1430;
}
else
{
return 1434;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1380;
}
else
{
return 1394;
}
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
return 1436;
}
else
{
return 1392;
}
}
}
}
}
else
{
if (((word >> 4) & 0x1) == 0)
{
return 1358;
}
else
{
return 1337;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1856;
}
else
{
return 1835;
}
}
else
{
return 1861;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1424;
}
else
{
return 1770;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1360;
}
else
{
return 1362;
}
}
else
{
return 1370;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1421;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1769;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1768;
}
else
{
return 1772;
}
}
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1420;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1369;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1367;
}
else
{
return 1371;
}
}
}
}
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1423;
}
else
{
return 1767;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1364;
}
else
{
return 1366;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1426;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1771;
}
else
{
return 1773;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1368;
}
else
{
return 1372;
}
}
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1425;
}
else
{
return 1922;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1361;
}
else
{
return 1363;
}
}
else
{
return 1377;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 1921;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1920;
}
else
{
return 1924;
}
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1431;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 1376;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1374;
}
else
{
return 1378;
}
}
}
}
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1427;
}
else
{
return 1919;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1365;
}
else
{
return 1373;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1422;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1923;
}
else
{
return 1925;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1375;
}
else
{
return 1379;
}
}
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1857;
}
else
{
return 1836;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1864;
}
else
{
return 1840;
}
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 1338;
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1865;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1899;
}
else
{
return 1895;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1883;
}
else
{
return 1879;
}
}
}
}
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1307;
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 1814;
}
else
{
return 1987;
}
}
else
{
return 1816;
}
}
else
{
return 1815;
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 1445;
}
else
{
return 1781;
}
}
else
{
return 1446;
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
return 1800;
}
else
{
return 1802;
}
}
else
{
return 1801;
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
return 1962;
}
else
{
return 1320;
}
}
else
{
return 1948;
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
return 1963;
}
else
{
return 1321;
}
}
else
{
return 1949;
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
return 1964;
}
else
{
return 1950;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1538;
}
else
{
return 1525;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1557;
}
else
{
return 1477;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1671;
}
else
{
return 1668;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1674;
}
else
{
return 1663;
}
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 1234;
}
else
{
return 1703;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1782;
}
else
{
return 1326;
}
}
}
else
{
return 1791;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
return 1785;
}
else
{
return 1821;
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1903;
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 1931;
}
else
{
return 1382;
}
}
}
else
{
return 1939;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 1905;
}
else
{
return 1934;
}
}
else
{
return 1969;
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1565;
}
else
{
return 1561;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1581;
}
else
{
return 1577;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1566;
}
else
{
return 1562;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1582;
}
else
{
return 1578;
}
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 1384;
}
else
{
return 1414;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1858;
}
else
{
return 1837;
}
}
else
{
return 1863;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 1405;
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1859;
}
else
{
return 1838;
}
}
else
{
return 1866;
}
}
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 1417;
}
else
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1867;
}
else
{
return 1841;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 1875;
}
else
{
return 1871;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 1891;
}
else
{
return 1887;
}
}
}
}
}
}
}
}
}
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 630;
}
else
{
return 631;
}
}
else
{
if (((word >> 24) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
if (((word >> 25) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 651;
}
else
{
if (((word >> 0) & 0x1) == 0)
{
if (((word >> 1) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 742;
}
else
{
return 741;
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
return 739;
}
else
{
return 744;
}
}
}
else
{
if (((word >> 1) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 738;
}
else
{
return 743;
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
return 740;
}
else
{
return 745;
}
}
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 632;
}
else
{
return 635;
}
}
else
{
return 634;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 633;
}
else
{
return 636;
}
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 641;
}
else
{
return 647;
}
}
else
{
return 645;
}
}
else
{
return 643;
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 642;
}
else
{
return 648;
}
}
else
{
return 646;
}
}
else
{
return 644;
}
}
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 25) & 0x1) == 0)
{
return 1157;
}
else
{
if (((word >> 10) & 0x1) == 0)
{
return 637;
}
else
{
return 638;
}
}
}
else
{
if (((word >> 25) & 0x1) == 0)
{
return 1185;
}
else
{
if (((word >> 10) & 0x1) == 0)
{
return 639;
}
else
{
return 640;
}
}
}
}
}
}
else
{
if (((word >> 24) & 0x1) == 0)
{
if (((word >> 25) & 0x1) == 0)
{
return 649;
}
else
{
return 1195;
}
}
else
{
if (((word >> 25) & 0x1) == 0)
{
return 650;
}
else
{
return 1196;
}
}
}
}
}
else
{
if (((word >> 25) & 0x1) == 0)
{
if (((word >> 28) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 24) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 434;
}
else
{
return 943;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 450;
}
else
{
return 452;
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
return 451;
}
else
{
return 453;
}
}
}
else
{
return 947;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 24) & 0x1) == 0)
{
return 442;
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 462;
}
else
{
return 463;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 464;
}
else
{
return 465;
}
}
}
else
{
return 952;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 24) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 438;
}
else
{
return 944;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 454;
}
else
{
return 458;
}
}
else
{
if (((word >> 21) & 0x1) == 0)
{
return 455;
}
else
{
return 459;
}
}
}
else
{
return 948;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 24) & 0x1) == 0)
{
return 446;
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 466;
}
else
{
return 467;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 470;
}
else
{
return 471;
}
}
}
else
{
return 953;
}
}
}
}
else
{
if (((word >> 24) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 956;
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 899;
}
else
{
return 900;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 878;
}
else
{
return 879;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 855;
}
else
{
return 856;
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 866;
}
else
{
return 867;
}
}
}
}
else
{
if (((word >> 24) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
if (((word >> 28) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 414;
}
else
{
return 415;
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 257;
}
else
{
return 260;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 256;
}
else
{
return 259;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 258;
}
else
{
return 261;
}
}
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 147;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 286;
}
else
{
return 302;
}
}
}
else
{
return 294;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 290;
}
else
{
return 306;
}
}
else
{
return 2006;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 296;
}
else
{
return 308;
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 148;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 288;
}
else
{
return 304;
}
}
}
else
{
return 149;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 152;
}
else
{
return 292;
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
return 150;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 298;
}
else
{
return 310;
}
}
}
}
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 2013;
}
else
{
return 2015;
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
return 2017;
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 2018;
}
else
{
return 2011;
}
}
else
{
return 2020;
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 2019;
}
else
{
return 2024;
}
}
else
{
return 2021;
}
}
}
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
return 130;
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 154;
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 337;
}
else
{
return 353;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 343;
}
else
{
return 357;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 339;
}
else
{
return 355;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 347;
}
else
{
return 361;
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 345;
}
else
{
return 359;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 341;
}
else
{
return 349;
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 364;
}
else
{
return 2005;
}
}
else
{
return 365;
}
}
else
{
return 366;
}
}
else
{
return 367;
}
}
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
return 752;
}
else
{
return 748;
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
return 754;
}
else
{
return 750;
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 672;
}
else
{
return 676;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 674;
}
else
{
return 678;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 673;
}
else
{
return 677;
}
}
else
{
return 675;
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
return 529;
}
else
{
return 550;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 548;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 552;
}
else
{
return 554;
}
}
}
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
return 582;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 567;
}
else
{
return 573;
}
}
}
else
{
return 571;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 583;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 569;
}
else
{
return 575;
}
}
}
}
}
}
else
{
if (((word >> 28) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 42;
}
else
{
return 43;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 50;
}
else
{
return 51;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 46;
}
else
{
return 47;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 54;
}
else
{
return 55;
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 44;
}
else
{
return 45;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 52;
}
else
{
return 53;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 48;
}
else
{
return 49;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 56;
}
else
{
return 57;
}
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 156;
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 160;
}
else
{
return 665;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 170;
}
else
{
return 171;
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
return 158;
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 168;
}
else
{
return 169;
}
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 162;
}
else
{
return 667;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 172;
}
else
{
return 173;
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 157;
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 161;
}
else
{
return 666;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 159;
}
else
{
return 27;
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
return 163;
}
else
{
return 668;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 174;
}
else
{
return 175;
}
}
}
}
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 262;
}
else
{
return 269;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 265;
}
else
{
return 273;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 264;
}
else
{
return 271;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 267;
}
else
{
return 275;
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 263;
}
else
{
return 270;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 266;
}
else
{
return 274;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 299;
}
else
{
return 311;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 300;
}
else
{
return 313;
}
}
}
else
{
return 272;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 268;
}
else
{
return 276;
}
}
}
}
}
}
else
{
return 2016;
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 74;
}
else
{
return 75;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 82;
}
else
{
return 83;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 78;
}
else
{
return 79;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 86;
}
else
{
return 87;
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 76;
}
else
{
return 77;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 84;
}
else
{
return 85;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 80;
}
else
{
return 81;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 88;
}
else
{
return 89;
}
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 207;
}
else
{
if (((word >> 16) & 0x1) == 0)
{
return 210;
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 220;
}
else
{
return 221;
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
return 208;
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 216;
}
else
{
return 217;
}
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
return 211;
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 222;
}
else
{
return 223;
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
return 236;
}
else
{
return 238;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 209;
}
else
{
return 31;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 218;
}
else
{
return 219;
}
}
}
else
{
return 212;
}
}
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 314;
}
else
{
return 321;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 317;
}
else
{
return 325;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 316;
}
else
{
return 323;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 319;
}
else
{
return 327;
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 315;
}
else
{
return 322;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 318;
}
else
{
return 326;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 350;
}
else
{
return 362;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 351;
}
else
{
return 363;
}
}
}
else
{
return 324;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 320;
}
else
{
return 328;
}
}
}
}
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 58;
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 59;
}
else
{
return 2009;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 90;
}
else
{
return 91;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 66;
}
else
{
if (((word >> 31) & 0x1) == 0)
{
return 67;
}
else
{
return 2022;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 94;
}
else
{
return 95;
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 62;
}
else
{
return 63;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 92;
}
else
{
return 93;
}
}
}
else
{
if (((word >> 22) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 70;
}
else
{
return 72;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 71;
}
else
{
return 73;
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 60;
}
else
{
return 61;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 68;
}
else
{
return 69;
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 64;
}
else
{
return 65;
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
return 164;
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 176;
}
else
{
return 196;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 177;
}
else
{
return 197;
}
}
}
}
else
{
return 2012;
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
return 213;
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 224;
}
else
{
return 225;
}
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
return 35;
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 184;
}
else
{
return 185;
}
}
}
else
{
return 2025;
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
return 34;
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 232;
}
else
{
return 233;
}
}
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 188;
}
else
{
return 239;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 39;
}
else
{
return 38;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 189;
}
else
{
return 240;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 204;
}
else
{
return 251;
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 166;
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 28;
}
else
{
return 32;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 180;
}
else
{
return 228;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 200;
}
else
{
return 247;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 29;
}
else
{
return 33;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 181;
}
else
{
return 229;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 201;
}
else
{
return 248;
}
}
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 192;
}
else
{
return 193;
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 165;
}
else
{
return 214;
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 178;
}
else
{
return 226;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 198;
}
else
{
return 245;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 179;
}
else
{
return 227;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 199;
}
else
{
return 246;
}
}
}
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 190;
}
else
{
return 241;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 191;
}
else
{
return 242;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 186;
}
else
{
return 234;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 205;
}
else
{
return 252;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 187;
}
else
{
return 235;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 206;
}
else
{
return 253;
}
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 167;
}
else
{
return 215;
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 182;
}
else
{
return 230;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 202;
}
else
{
return 249;
}
}
}
else
{
return 30;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 183;
}
else
{
return 231;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 203;
}
else
{
return 250;
}
}
}
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 194;
}
else
{
return 243;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 37;
}
else
{
return 36;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 41;
}
else
{
return 40;
}
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 195;
}
else
{
return 244;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 254;
}
else
{
return 255;
}
}
}
}
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 277;
}
else
{
return 2010;
}
}
else
{
return 329;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 285;
}
else
{
return 2023;
}
}
else
{
return 336;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 301;
}
else
{
return 352;
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 281;
}
else
{
return 333;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 293;
}
else
{
return 342;
}
}
else
{
return 356;
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 279;
}
else
{
return 331;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 289;
}
else
{
return 338;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 305;
}
else
{
return 354;
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 283;
}
else
{
return 335;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 295;
}
else
{
return 346;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 307;
}
else
{
return 360;
}
}
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 31) & 0x1) == 0)
{
return 278;
}
else
{
return 2014;
}
}
else
{
return 330;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 287;
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 2028;
}
else
{
return 2032;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 303;
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 2029;
}
else
{
return 2033;
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 282;
}
else
{
return 334;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 2026;
}
else
{
return 2030;
}
}
else
{
return 344;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 2027;
}
else
{
return 2031;
}
}
else
{
return 358;
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 280;
}
else
{
return 332;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 291;
}
else
{
return 340;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 284;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 297;
}
else
{
return 348;
}
}
else
{
return 309;
}
}
}
}
}
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 756;
}
else
{
return 776;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 772;
}
else
{
return 780;
}
}
}
else
{
return 764;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
return 760;
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 768;
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 784;
}
else
{
return 786;
}
}
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
return 758;
}
else
{
return 778;
}
}
else
{
if (((word >> 20) & 0x1) == 0)
{
return 774;
}
else
{
return 782;
}
}
}
else
{
return 766;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
return 762;
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 770;
}
else
{
return 785;
}
}
}
}
}
else
{
if (((word >> 17) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
return 799;
}
else
{
return 808;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
return 803;
}
else
{
return 812;
}
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 18) & 0x1) == 0)
{
return 801;
}
else
{
return 810;
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
return 805;
}
else
{
return 814;
}
}
}
}
else
{
if (((word >> 18) & 0x1) == 0)
{
return 807;
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 16) & 0x1) == 0)
{
return 816;
}
else
{
return 818;
}
}
else
{
return 820;
}
}
}
}
}
else
{
if (((word >> 3) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 791;
}
else
{
return 793;
}
}
else
{
if (((word >> 4) & 0x1) == 0)
{
return 795;
}
else
{
return 797;
}
}
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 848;
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 416;
}
else
{
return 418;
}
}
else
{
return 417;
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 822;
}
else
{
return 669;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 838;
}
else
{
return 476;
}
}
else
{
return 505;
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 830;
}
else
{
return 480;
}
}
else
{
return 509;
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 489;
}
else
{
return 519;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 533;
}
else
{
return 532;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 539;
}
else
{
return 538;
}
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 490;
}
else
{
return 520;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 485;
}
else
{
return 515;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 486;
}
else
{
return 516;
}
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 826;
}
else
{
return 671;
}
}
else
{
return 508;
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
return 478;
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 481;
}
else
{
return 511;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 495;
}
else
{
return 523;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 482;
}
else
{
return 512;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 496;
}
else
{
return 524;
}
}
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 834;
}
else
{
return 510;
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 493;
}
else
{
return 494;
}
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 824;
}
else
{
return 670;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 477;
}
else
{
return 506;
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
return 832;
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 491;
}
else
{
return 521;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 535;
}
else
{
return 534;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 492;
}
else
{
return 522;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 487;
}
else
{
return 517;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 499;
}
else
{
return 527;
}
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 488;
}
else
{
return 518;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 500;
}
else
{
return 528;
}
}
}
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 828;
}
else
{
return 474;
}
}
else
{
return 503;
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 479;
}
else
{
return 507;
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
if (((word >> 20) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 483;
}
else
{
return 513;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 497;
}
else
{
return 525;
}
}
}
else
{
return 531;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 484;
}
else
{
return 514;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 498;
}
else
{
return 526;
}
}
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 836;
}
else
{
return 475;
}
}
else
{
return 504;
}
}
else
{
if (((word >> 16) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 537;
}
else
{
return 536;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 541;
}
else
{
return 540;
}
}
}
else
{
if (((word >> 19) & 0x1) == 0)
{
return 501;
}
else
{
return 502;
}
}
}
}
}
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 4) & 0x1) == 0)
{
return 787;
}
else
{
return 789;
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 559;
}
else
{
return 557;
}
}
else
{
return 549;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 558;
}
else
{
if (((word >> 15) & 0x1) == 0)
{
return 555;
}
else
{
return 546;
}
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 580;
}
else
{
return 578;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 566;
}
else
{
return 572;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
return 579;
}
else
{
return 570;
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
return 576;
}
else
{
return 565;
}
}
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 850;
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
return 542;
}
else
{
return 560;
}
}
else
{
return 544;
}
}
else
{
return 543;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
return 545;
}
else
{
return 547;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 556;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 551;
}
else
{
return 553;
}
}
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 15) & 0x1) == 0)
{
return 561;
}
else
{
return 581;
}
}
else
{
return 563;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 562;
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 568;
}
else
{
return 574;
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 564;
}
else
{
return 577;
}
}
}
}
}
}
}
}
else
{
if (((word >> 15) & 0x1) == 0)
{
if (((word >> 28) & 0x1) == 0)
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 2034;
}
else
{
return 2038;
}
}
else
{
return 117;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 2035;
}
else
{
return 2039;
}
}
else
{
return 120;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 96;
}
else
{
return 97;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 118;
}
else
{
return 119;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 100;
}
else
{
return 101;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 121;
}
else
{
return 122;
}
}
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 112;
}
else
{
return 111;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 114;
}
else
{
return 113;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 98;
}
else
{
return 99;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 102;
}
else
{
return 103;
}
}
}
}
else
{
return 129;
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 131;
}
else
{
return 139;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 132;
}
else
{
return 140;
}
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 840;
}
else
{
return 844;
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 425;
}
else
{
return 424;
}
}
else
{
if (((word >> 23) & 0x1) == 0)
{
return 427;
}
else
{
return 426;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 419;
}
else
{
return 420;
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
return 584;
}
else
{
return 586;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 585;
}
else
{
return 588;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 587;
}
else
{
return 589;
}
}
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 596;
}
else
{
return 600;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 598;
}
else
{
return 602;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
return 597;
}
else
{
return 601;
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
return 599;
}
else
{
return 603;
}
}
}
}
}
}
else
{
if (((word >> 28) & 0x1) == 0)
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 104;
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 2036;
}
else
{
return 2040;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 109;
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 2037;
}
else
{
return 2041;
}
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 105;
}
else
{
return 106;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 123;
}
else
{
return 124;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 2008;
}
else
{
return 2007;
}
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 116;
}
else
{
return 126;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 115;
}
else
{
return 125;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 110;
}
else
{
return 127;
}
}
}
else
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 107;
}
else
{
return 108;
}
}
else
{
return 128;
}
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 133;
}
else
{
return 141;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 134;
}
else
{
return 142;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 135;
}
else
{
return 143;
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
return 136;
}
else
{
return 144;
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 137;
}
else
{
return 146;
}
}
}
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 376;
}
else
{
return 377;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 400;
}
else
{
return 401;
}
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
return 380;
}
else
{
return 381;
}
}
else
{
if (((word >> 30) & 0x1) == 0)
{
return 404;
}
else
{
return 405;
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
return 138;
}
else
{
return 412;
}
}
}
}
}
}
else
{
if (((word >> 29) & 0x1) == 0)
{
if (((word >> 30) & 0x1) == 0)
{
if (((word >> 21) & 0x1) == 0)
{
return 842;
}
else
{
return 846;
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 422;
}
else
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 429;
}
else
{
return 428;
}
}
else
{
return 423;
}
}
else
{
return 421;
}
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
return 592;
}
else
{
return 590;
}
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 591;
}
else
{
return 594;
}
}
}
}
}
else
{
if (((word >> 10) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
if (((word >> 14) & 0x1) == 0)
{
if (((word >> 23) & 0x1) == 0)
{
return 431;
}
else
{
return 430;
}
}
else
{
return 432;
}
}
else
{
return 433;
}
}
else
{
if (((word >> 11) & 0x1) == 0)
{
if (((word >> 12) & 0x1) == 0)
{
if (((word >> 13) & 0x1) == 0)
{
return 604;
}
else
{
return 608;
}
}
else
{
return 606;
}
}
else
{
if (((word >> 12) & 0x1) == 0)
{
return 605;
}
else
{
if (((word >> 13) & 0x1) == 0)
{
return 607;
}
else
{
return 610;
}
}
}
}
}
}
}
}
}
}
}
}
const aarch64_opcode *
aarch64_opcode_lookup (uint32_t word)
{
return aarch64_opcode_table + aarch64_opcode_lookup_1 (word);
}
const aarch64_opcode *
aarch64_find_next_opcode (const aarch64_opcode *opcode)
{
int key = opcode - aarch64_opcode_table;
int value;
switch (key)
{
case 941: value = 945; break; 
case 945: return NULL; 
case 939: value = 940; break; 
case 940: return NULL; 
case 942: value = 946; break; 
case 946: return NULL; 
case 1584: value = 1585; break; 
case 1585: return NULL; 
case 1640: value = 1641; break; 
case 1641: return NULL; 
case 1588: value = 1589; break; 
case 1589: return NULL; 
case 1607: value = 1608; break; 
case 1608: return NULL; 
case 1586: value = 1587; break; 
case 1587: return NULL; 
case 1605: value = 1606; break; 
case 1606: return NULL; 
case 1590: value = 1591; break; 
case 1591: return NULL; 
case 1609: value = 1610; break; 
case 1610: return NULL; 
case 1630: value = 1631; break; 
case 1631: return NULL; 
case 1618: value = 1619; break; 
case 1619: return NULL; 
case 1649: value = 1650; break; 
case 1650: return NULL; 
case 1622: value = 1623; break; 
case 1623: return NULL; 
case 1632: value = 1633; break; 
case 1633: return NULL; 
case 1620: value = 1621; break; 
case 1621: return NULL; 
case 1651: value = 1652; break; 
case 1652: return NULL; 
case 1596: value = 1597; break; 
case 1597: return NULL; 
case 1157: value = 1158; break; 
case 1158: value = 1173; break; 
case 1173: value = 1174; break; 
case 1174: value = 1177; break; 
case 1177: value = 1178; break; 
case 1178: value = 1179; break; 
case 1179: value = 1184; break; 
case 1184: value = 2042; break; 
case 2042: return NULL; 
case 1185: value = 1186; break; 
case 1186: return NULL; 
case 434: value = 435; break; 
case 435: value = 436; break; 
case 436: value = 437; break; 
case 437: return NULL; 
case 442: value = 443; break; 
case 443: value = 444; break; 
case 444: value = 445; break; 
case 445: return NULL; 
case 438: value = 439; break; 
case 439: value = 440; break; 
case 440: value = 441; break; 
case 441: return NULL; 
case 454: value = 456; break; 
case 456: return NULL; 
case 458: value = 460; break; 
case 460: return NULL; 
case 455: value = 457; break; 
case 457: return NULL; 
case 459: value = 461; break; 
case 461: return NULL; 
case 446: value = 447; break; 
case 447: value = 448; break; 
case 448: value = 449; break; 
case 449: return NULL; 
case 466: value = 468; break; 
case 468: return NULL; 
case 467: value = 469; break; 
case 469: return NULL; 
case 470: value = 472; break; 
case 472: return NULL; 
case 471: value = 473; break; 
case 473: return NULL; 
case 752: value = 753; break; 
case 753: return NULL; 
case 748: value = 749; break; 
case 749: return NULL; 
case 754: value = 755; break; 
case 755: return NULL; 
case 750: value = 751; break; 
case 751: return NULL; 
case 756: value = 757; break; 
case 757: return NULL; 
case 776: value = 777; break; 
case 777: return NULL; 
case 772: value = 773; break; 
case 773: return NULL; 
case 780: value = 781; break; 
case 781: return NULL; 
case 764: value = 765; break; 
case 765: return NULL; 
case 760: value = 761; break; 
case 761: return NULL; 
case 768: value = 769; break; 
case 769: return NULL; 
case 758: value = 759; break; 
case 759: return NULL; 
case 778: value = 779; break; 
case 779: return NULL; 
case 774: value = 775; break; 
case 775: return NULL; 
case 782: value = 783; break; 
case 783: return NULL; 
case 766: value = 767; break; 
case 767: return NULL; 
case 762: value = 763; break; 
case 763: return NULL; 
case 770: value = 771; break; 
case 771: return NULL; 
case 799: value = 800; break; 
case 800: return NULL; 
case 808: value = 809; break; 
case 809: return NULL; 
case 803: value = 804; break; 
case 804: return NULL; 
case 812: value = 813; break; 
case 813: return NULL; 
case 801: value = 802; break; 
case 802: return NULL; 
case 810: value = 811; break; 
case 811: return NULL; 
case 805: value = 806; break; 
case 806: return NULL; 
case 814: value = 815; break; 
case 815: return NULL; 
case 816: value = 817; break; 
case 817: return NULL; 
case 818: value = 819; break; 
case 819: return NULL; 
case 820: value = 821; break; 
case 821: return NULL; 
case 791: value = 792; break; 
case 792: return NULL; 
case 793: value = 794; break; 
case 794: return NULL; 
case 795: value = 796; break; 
case 796: return NULL; 
case 797: value = 798; break; 
case 798: return NULL; 
case 848: value = 849; break; 
case 849: return NULL; 
case 822: value = 823; break; 
case 823: return NULL; 
case 838: value = 839; break; 
case 839: return NULL; 
case 830: value = 831; break; 
case 831: return NULL; 
case 826: value = 827; break; 
case 827: return NULL; 
case 834: value = 835; break; 
case 835: return NULL; 
case 824: value = 825; break; 
case 825: return NULL; 
case 832: value = 833; break; 
case 833: return NULL; 
case 828: value = 829; break; 
case 829: return NULL; 
case 836: value = 837; break; 
case 837: return NULL; 
case 787: value = 788; break; 
case 788: return NULL; 
case 789: value = 790; break; 
case 790: return NULL; 
case 850: value = 851; break; 
case 851: return NULL; 
case 131: value = 368; break; 
case 368: value = 370; break; 
case 370: return NULL; 
case 139: value = 390; break; 
case 390: value = 392; break; 
case 392: value = 394; break; 
case 394: value = 396; break; 
case 396: return NULL; 
case 132: value = 369; break; 
case 369: value = 371; break; 
case 371: value = 372; break; 
case 372: value = 373; break; 
case 373: return NULL; 
case 140: value = 391; break; 
case 391: value = 393; break; 
case 393: value = 395; break; 
case 395: value = 397; break; 
case 397: return NULL; 
case 840: value = 841; break; 
case 841: return NULL; 
case 844: value = 845; break; 
case 845: return NULL; 
case 133: value = 374; break; 
case 374: value = 375; break; 
case 375: value = 382; break; 
case 382: value = 384; break; 
case 384: return NULL; 
case 141: value = 398; break; 
case 398: value = 399; break; 
case 399: value = 406; break; 
case 406: value = 408; break; 
case 408: return NULL; 
case 134: value = 378; break; 
case 378: value = 379; break; 
case 379: return NULL; 
case 142: value = 402; break; 
case 402: value = 403; break; 
case 403: return NULL; 
case 136: value = 386; break; 
case 386: value = 387; break; 
case 387: return NULL; 
case 144: value = 145; break; 
case 145: value = 410; break; 
case 410: value = 411; break; 
case 411: return NULL; 
case 138: value = 388; break; 
case 388: value = 389; break; 
case 389: return NULL; 
case 412: value = 413; break; 
case 413: return NULL; 
case 842: value = 843; break; 
case 843: return NULL; 
case 846: value = 847; break; 
case 847: return NULL; 
case 592: value = 593; break; 
case 593: return NULL; 
case 594: value = 595; break; 
case 595: return NULL; 
case 608: value = 609; break; 
case 609: return NULL; 
case 610: value = 611; break; 
case 611: return NULL; 
default: return NULL;
}
return aarch64_opcode_table + value;
}
const aarch64_opcode *
aarch64_find_alias_opcode (const aarch64_opcode *opcode)
{
int key = opcode - aarch64_opcode_table;
int value;
switch (key)
{
case 2: value = 3; break; 
case 4: value = 5; break; 
case 7: value = 8; break; 
case 10: value = 11; break; 
case 12: value = 13; break; 
case 14: value = 15; break; 
case 17: value = 18; break; 
case 20: value = 21; break; 
case 22: value = 23; break; 
case 24: value = 25; break; 
case 150: value = 151; break; 
case 152: value = 153; break; 
case 154: value = 155; break; 
case 236: value = 237; break; 
case 311: value = 312; break; 
case 382: value = 383; break; 
case 384: value = 385; break; 
case 406: value = 407; break; 
case 408: value = 409; break; 
case 529: value = 530; break; 
case 612: value = 617; break; 
case 619: value = 621; break; 
case 623: value = 627; break; 
case 657: value = 659; break; 
case 660: value = 662; break; 
case 663: value = 664; break; 
case 682: value = 682; break; 
case 707: value = 708; break; 
case 709: value = 710; break; 
case 711: value = 712; break; 
case 713: value = 714; break; 
case 724: value = 725; break; 
case 726: value = 727; break; 
case 728: value = 729; break; 
case 730: value = 731; break; 
case 733: value = 734; break; 
case 735: value = 736; break; 
case 746: value = 747; break; 
case 959: value = 960; break; 
case 961: value = 962; break; 
case 964: value = 965; break; 
case 968: value = 970; break; 
case 971: value = 972; break; 
case 975: value = 976; break; 
case 1006: value = 1102; break; 
case 1007: value = 1103; break; 
case 1008: value = 1104; break; 
case 1010: value = 1105; break; 
case 1013: value = 1106; break; 
case 1016: value = 1107; break; 
case 1018: value = 1108; break; 
case 1019: value = 1109; break; 
case 1020: value = 1110; break; 
case 1022: value = 1111; break; 
case 1025: value = 1112; break; 
case 1028: value = 1113; break; 
case 1030: value = 1114; break; 
case 1031: value = 1115; break; 
case 1032: value = 1116; break; 
case 1034: value = 1117; break; 
case 1037: value = 1118; break; 
case 1040: value = 1119; break; 
case 1042: value = 1120; break; 
case 1043: value = 1121; break; 
case 1044: value = 1122; break; 
case 1046: value = 1123; break; 
case 1049: value = 1124; break; 
case 1052: value = 1125; break; 
case 1054: value = 1126; break; 
case 1055: value = 1127; break; 
case 1056: value = 1128; break; 
case 1058: value = 1129; break; 
case 1061: value = 1130; break; 
case 1064: value = 1131; break; 
case 1066: value = 1132; break; 
case 1067: value = 1133; break; 
case 1068: value = 1134; break; 
case 1070: value = 1135; break; 
case 1073: value = 1136; break; 
case 1076: value = 1137; break; 
case 1078: value = 1138; break; 
case 1079: value = 1139; break; 
case 1080: value = 1140; break; 
case 1082: value = 1141; break; 
case 1085: value = 1142; break; 
case 1088: value = 1143; break; 
case 1090: value = 1144; break; 
case 1091: value = 1145; break; 
case 1092: value = 1146; break; 
case 1094: value = 1147; break; 
case 1097: value = 1148; break; 
case 1100: value = 1149; break; 
case 1150: value = 1151; break; 
case 1152: value = 1153; break; 
case 1158: value = 1194; break; 
case 1174: value = 1176; break; 
case 1179: value = 1183; break; 
case 1242: value = 1992; break; 
case 1244: value = 1225; break; 
case 1245: value = 1229; break; 
case 1280: value = 1993; break; 
case 1283: value = 1996; break; 
case 1285: value = 1994; break; 
case 1288: value = 1995; break; 
case 1310: value = 1222; break; 
case 1311: value = 1224; break; 
case 1312: value = 2003; break; 
case 1324: value = 1217; break; 
case 1325: value = 1219; break; 
case 1326: value = 2002; break; 
case 1327: value = 1220; break; 
case 1329: value = 1997; break; 
case 1331: value = 1230; break; 
case 1332: value = 1231; break; 
case 1337: value = 1998; break; 
case 1338: value = 1999; break; 
case 1351: value = 2000; break; 
case 1353: value = 2001; break; 
case 1359: value = 1214; break; 
case 1382: value = 1213; break; 
case 1713: value = 1215; break; 
case 1714: value = 2004; break; 
case 1716: value = 1218; break; 
case 1717: value = 1228; break; 
case 1779: value = 1223; break; 
case 1780: value = 1226; break; 
default: return NULL;
}
return aarch64_opcode_table + value;
}
const aarch64_opcode *
aarch64_find_next_alias_opcode (const aarch64_opcode *opcode)
{
int key = opcode - aarch64_opcode_table;
int value;
switch (key)
{
case 3: value = 2; break; 
case 5: value = 4; break; 
case 8: value = 7; break; 
case 11: value = 10; break; 
case 13: value = 12; break; 
case 15: value = 14; break; 
case 18: value = 17; break; 
case 21: value = 20; break; 
case 23: value = 22; break; 
case 25: value = 26; break; 
case 26: value = 24; break; 
case 151: value = 150; break; 
case 153: value = 152; break; 
case 155: value = 154; break; 
case 237: value = 236; break; 
case 312: value = 311; break; 
case 383: value = 382; break; 
case 385: value = 384; break; 
case 407: value = 406; break; 
case 409: value = 408; break; 
case 530: value = 529; break; 
case 617: value = 616; break; 
case 616: value = 615; break; 
case 615: value = 618; break; 
case 618: value = 614; break; 
case 614: value = 613; break; 
case 613: value = 612; break; 
case 621: value = 622; break; 
case 622: value = 620; break; 
case 620: value = 619; break; 
case 627: value = 626; break; 
case 626: value = 629; break; 
case 629: value = 628; break; 
case 628: value = 625; break; 
case 625: value = 624; break; 
case 624: value = 623; break; 
case 659: value = 658; break; 
case 658: value = 657; break; 
case 662: value = 661; break; 
case 661: value = 660; break; 
case 664: value = 663; break; 
case 682: value = 683; break; 
case 708: value = 707; break; 
case 710: value = 709; break; 
case 712: value = 711; break; 
case 714: value = 713; break; 
case 725: value = 724; break; 
case 727: value = 726; break; 
case 729: value = 728; break; 
case 731: value = 730; break; 
case 734: value = 733; break; 
case 736: value = 735; break; 
case 747: value = 746; break; 
case 960: value = 959; break; 
case 962: value = 961; break; 
case 965: value = 964; break; 
case 970: value = 969; break; 
case 969: value = 968; break; 
case 972: value = 971; break; 
case 976: value = 975; break; 
case 1102: value = 1006; break; 
case 1103: value = 1007; break; 
case 1104: value = 1008; break; 
case 1105: value = 1010; break; 
case 1106: value = 1013; break; 
case 1107: value = 1016; break; 
case 1108: value = 1018; break; 
case 1109: value = 1019; break; 
case 1110: value = 1020; break; 
case 1111: value = 1022; break; 
case 1112: value = 1025; break; 
case 1113: value = 1028; break; 
case 1114: value = 1030; break; 
case 1115: value = 1031; break; 
case 1116: value = 1032; break; 
case 1117: value = 1034; break; 
case 1118: value = 1037; break; 
case 1119: value = 1040; break; 
case 1120: value = 1042; break; 
case 1121: value = 1043; break; 
case 1122: value = 1044; break; 
case 1123: value = 1046; break; 
case 1124: value = 1049; break; 
case 1125: value = 1052; break; 
case 1126: value = 1054; break; 
case 1127: value = 1055; break; 
case 1128: value = 1056; break; 
case 1129: value = 1058; break; 
case 1130: value = 1061; break; 
case 1131: value = 1064; break; 
case 1132: value = 1066; break; 
case 1133: value = 1067; break; 
case 1134: value = 1068; break; 
case 1135: value = 1070; break; 
case 1136: value = 1073; break; 
case 1137: value = 1076; break; 
case 1138: value = 1078; break; 
case 1139: value = 1079; break; 
case 1140: value = 1080; break; 
case 1141: value = 1082; break; 
case 1142: value = 1085; break; 
case 1143: value = 1088; break; 
case 1144: value = 1090; break; 
case 1145: value = 1091; break; 
case 1146: value = 1092; break; 
case 1147: value = 1094; break; 
case 1148: value = 1097; break; 
case 1149: value = 1100; break; 
case 1151: value = 1150; break; 
case 1153: value = 1152; break; 
case 1194: value = 1193; break; 
case 1193: value = 1192; break; 
case 1192: value = 1191; break; 
case 1191: value = 1190; break; 
case 1190: value = 1189; break; 
case 1189: value = 1188; break; 
case 1188: value = 1187; break; 
case 1187: value = 1172; break; 
case 1172: value = 1171; break; 
case 1171: value = 1170; break; 
case 1170: value = 1169; break; 
case 1169: value = 1168; break; 
case 1168: value = 1167; break; 
case 1167: value = 1166; break; 
case 1166: value = 1165; break; 
case 1165: value = 1164; break; 
case 1164: value = 1163; break; 
case 1163: value = 1162; break; 
case 1162: value = 1161; break; 
case 1161: value = 1160; break; 
case 1160: value = 1159; break; 
case 1159: value = 1158; break; 
case 1176: value = 1175; break; 
case 1175: value = 1174; break; 
case 1183: value = 1182; break; 
case 1182: value = 1181; break; 
case 1181: value = 1180; break; 
case 1180: value = 1179; break; 
case 1992: value = 1242; break; 
case 1225: value = 1244; break; 
case 1229: value = 1245; break; 
case 1993: value = 1280; break; 
case 1996: value = 1283; break; 
case 1994: value = 1285; break; 
case 1995: value = 1288; break; 
case 1222: value = 1310; break; 
case 1224: value = 1311; break; 
case 2003: value = 1227; break; 
case 1227: value = 1312; break; 
case 1217: value = 1324; break; 
case 1219: value = 1216; break; 
case 1216: value = 1325; break; 
case 2002: value = 1221; break; 
case 1221: value = 1326; break; 
case 1220: value = 1327; break; 
case 1997: value = 1329; break; 
case 1230: value = 1331; break; 
case 1231: value = 1332; break; 
case 1998: value = 1337; break; 
case 1999: value = 1338; break; 
case 2000: value = 1351; break; 
case 2001: value = 1353; break; 
case 1214: value = 1359; break; 
case 1213: value = 1382; break; 
case 1215: value = 1713; break; 
case 2004: value = 1714; break; 
case 1218: value = 1716; break; 
case 1228: value = 1717; break; 
case 1223: value = 1779; break; 
case 1226: value = 1780; break; 
default: return NULL;
}
return aarch64_opcode_table + value;
}
bfd_boolean
aarch64_extract_operand (const aarch64_operand *self,
aarch64_opnd_info *info,
aarch64_insn code, const aarch64_inst *inst,
aarch64_operand_error *errors)
{
int key = self - aarch64_operands;
switch (key)
{
case 1:
case 2:
case 3:
case 4:
case 5:
case 6:
case 7:
case 9:
case 10:
case 11:
case 15:
case 16:
case 17:
case 18:
case 20:
case 21:
case 22:
case 23:
case 24:
case 25:
case 26:
case 27:
case 28:
case 29:
case 153:
case 154:
case 155:
case 156:
case 157:
case 158:
case 159:
case 160:
case 161:
case 162:
case 175:
case 176:
case 177:
case 178:
case 179:
case 180:
case 181:
case 182:
case 183:
case 187:
case 190:
return aarch64_ext_regno (self, info, code, inst, errors);
case 8:
return aarch64_ext_regrt_sysins (self, info, code, inst, errors);
case 12:
return aarch64_ext_regno_pair (self, info, code, inst, errors);
case 13:
return aarch64_ext_reg_extended (self, info, code, inst, errors);
case 14:
return aarch64_ext_reg_shifted (self, info, code, inst, errors);
case 19:
return aarch64_ext_ft (self, info, code, inst, errors);
case 30:
case 31:
case 32:
case 33:
case 192:
return aarch64_ext_reglane (self, info, code, inst, errors);
case 34:
return aarch64_ext_reglist (self, info, code, inst, errors);
case 35:
return aarch64_ext_ldst_reglist (self, info, code, inst, errors);
case 36:
return aarch64_ext_ldst_reglist_r (self, info, code, inst, errors);
case 37:
return aarch64_ext_ldst_elemlist (self, info, code, inst, errors);
case 38:
case 39:
case 40:
case 41:
case 51:
case 52:
case 53:
case 54:
case 55:
case 56:
case 57:
case 58:
case 59:
case 60:
case 61:
case 62:
case 63:
case 64:
case 75:
case 76:
case 77:
case 78:
case 79:
case 150:
case 152:
case 167:
case 168:
case 169:
case 170:
case 171:
case 172:
case 173:
case 174:
return aarch64_ext_imm (self, info, code, inst, errors);
case 42:
case 43:
return aarch64_ext_advsimd_imm_shift (self, info, code, inst, errors);
case 44:
case 45:
case 46:
return aarch64_ext_advsimd_imm_modified (self, info, code, inst, errors);
case 47:
return aarch64_ext_shll_imm (self, info, code, inst, errors);
case 50:
case 141:
return aarch64_ext_fpimm (self, info, code, inst, errors);
case 65:
case 148:
return aarch64_ext_limm (self, info, code, inst, errors);
case 66:
return aarch64_ext_aimm (self, info, code, inst, errors);
case 67:
return aarch64_ext_imm_half (self, info, code, inst, errors);
case 68:
return aarch64_ext_fbits (self, info, code, inst, errors);
case 70:
case 71:
case 146:
return aarch64_ext_imm_rotate2 (self, info, code, inst, errors);
case 72:
case 145:
return aarch64_ext_imm_rotate1 (self, info, code, inst, errors);
case 73:
case 74:
return aarch64_ext_cond (self, info, code, inst, errors);
case 80:
case 87:
return aarch64_ext_addr_simple (self, info, code, inst, errors);
case 81:
return aarch64_ext_addr_regoff (self, info, code, inst, errors);
case 82:
case 83:
case 84:
return aarch64_ext_addr_simm (self, info, code, inst, errors);
case 85:
return aarch64_ext_addr_simm10 (self, info, code, inst, errors);
case 86:
return aarch64_ext_addr_uimm12 (self, info, code, inst, errors);
case 88:
return aarch64_ext_addr_offset (self, info, code, inst, errors);
case 89:
return aarch64_ext_simd_addr_post (self, info, code, inst, errors);
case 90:
return aarch64_ext_sysreg (self, info, code, inst, errors);
case 91:
return aarch64_ext_pstatefield (self, info, code, inst, errors);
case 92:
case 93:
case 94:
case 95:
return aarch64_ext_sysins_op (self, info, code, inst, errors);
case 96:
case 97:
return aarch64_ext_barrier (self, info, code, inst, errors);
case 98:
return aarch64_ext_prfop (self, info, code, inst, errors);
case 99:
return aarch64_ext_hint (self, info, code, inst, errors);
case 100:
return aarch64_ext_sve_addr_ri_s4 (self, info, code, inst, errors);
case 101:
case 102:
case 103:
case 104:
return aarch64_ext_sve_addr_ri_s4xvl (self, info, code, inst, errors);
case 105:
return aarch64_ext_sve_addr_ri_s6xvl (self, info, code, inst, errors);
case 106:
return aarch64_ext_sve_addr_ri_s9xvl (self, info, code, inst, errors);
case 107:
case 108:
case 109:
case 110:
return aarch64_ext_sve_addr_ri_u6 (self, info, code, inst, errors);
case 111:
case 112:
case 113:
case 114:
case 115:
case 116:
case 117:
case 118:
case 119:
case 120:
case 121:
case 122:
case 123:
return aarch64_ext_sve_addr_rr_lsl (self, info, code, inst, errors);
case 124:
case 125:
case 126:
case 127:
case 128:
case 129:
case 130:
case 131:
return aarch64_ext_sve_addr_rz_xtw (self, info, code, inst, errors);
case 132:
case 133:
case 134:
case 135:
return aarch64_ext_sve_addr_zi_u5 (self, info, code, inst, errors);
case 136:
return aarch64_ext_sve_addr_zz_lsl (self, info, code, inst, errors);
case 137:
return aarch64_ext_sve_addr_zz_sxtw (self, info, code, inst, errors);
case 138:
return aarch64_ext_sve_addr_zz_uxtw (self, info, code, inst, errors);
case 139:
return aarch64_ext_sve_aimm (self, info, code, inst, errors);
case 140:
return aarch64_ext_sve_asimm (self, info, code, inst, errors);
case 142:
return aarch64_ext_sve_float_half_one (self, info, code, inst, errors);
case 143:
return aarch64_ext_sve_float_half_two (self, info, code, inst, errors);
case 144:
return aarch64_ext_sve_float_zero_one (self, info, code, inst, errors);
case 147:
return aarch64_ext_inv_limm (self, info, code, inst, errors);
case 149:
return aarch64_ext_sve_limm_mov (self, info, code, inst, errors);
case 151:
return aarch64_ext_sve_scale (self, info, code, inst, errors);
case 163:
case 164:
return aarch64_ext_sve_shlimm (self, info, code, inst, errors);
case 165:
case 166:
return aarch64_ext_sve_shrimm (self, info, code, inst, errors);
case 184:
case 185:
case 186:
return aarch64_ext_sve_quad_index (self, info, code, inst, errors);
case 188:
return aarch64_ext_sve_index (self, info, code, inst, errors);
case 189:
case 191:
return aarch64_ext_sve_reglist (self, info, code, inst, errors);
default: assert (0); abort ();
}
}
