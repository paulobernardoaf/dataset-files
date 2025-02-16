return strdup (
"=PC eip\n"
"=SP esp\n"
"=BP ebp\n"
"=A0 eax\n"
"=A1 ebx\n"
"=A2 ecx\n"
"=A3 edx\n"
"=SN oeax\n"
"gpr eax .32 0 0\n"
"gpr ecx .32 4 0\n"
"gpr edx .32 8 0\n"
"gpr ebx .32 12 0\n"
"gpr esp .32 16 0\n"
"gpr ebp .32 20 0\n"
"gpr esi .32 24 0\n"
"gpr edi .32 28 0\n"
"gpr eip .32 32 0\n"
"gpr eflags .32 36 0\n"
"seg cs .32 40 0\n"
"seg ss .32 44 0\n"
"seg ds .32 48 0\n"
"seg es .32 52 0\n"
"seg fs .32 56 0\n"
"seg gs .32 60 0\n"
"fpu st0 .80 64 0\n"
"fpu st1 .80 74 0\n"
"fpu st2 .80 84 0\n"
"fpu st3 .80 94 0\n"
"fpu st4 .80 104 0\n"
"fpu st5 .80 114 0\n"
"fpu st6 .80 124 0\n"
"fpu st7 .80 134 0\n"
"gpr fctrl .32 144 0\n"
"gpr fstat .32 148 0\n"
"gpr ftag .32 152 0\n"
"gpr fiseg .32 156 0\n"
"gpr fioff .32 160 0\n"
"gpr foseg .32 164 0\n"
"gpr fooff .32 168 0\n"
"gpr fop .32 172 0\n"
"fpu xmm0 .128 176 0\n"
"fpu xmm1 .128 192 0\n"
"fpu xmm2 .128 208 0\n"
"fpu xmm3 .128 224 0\n"
"fpu xmm4 .128 240 0\n"
"fpu xmm5 .128 256 0\n"
"fpu xmm6 .128 272 0\n"
"fpu xmm7 .128 288 0\n"
"gpr mxcsr .32 304 0\n"
);
