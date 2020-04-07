














#include <stdbool.h>

#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/arabic.h"




#define a_COMMA 0x060C
#define a_SEMICOLON 0x061B
#define a_QUESTION 0x061F
#define a_HAMZA 0x0621
#define a_ALEF_MADDA 0x0622
#define a_ALEF_HAMZA_ABOVE 0x0623
#define a_WAW_HAMZA 0x0624
#define a_ALEF_HAMZA_BELOW 0x0625
#define a_YEH_HAMZA 0x0626
#define a_ALEF 0x0627
#define a_BEH 0x0628
#define a_TEH_MARBUTA 0x0629
#define a_TEH 0x062a
#define a_THEH 0x062b
#define a_JEEM 0x062c
#define a_HAH 0x062d
#define a_KHAH 0x062e
#define a_DAL 0x062f
#define a_THAL 0x0630
#define a_REH 0x0631
#define a_ZAIN 0x0632
#define a_SEEN 0x0633
#define a_SHEEN 0x0634
#define a_SAD 0x0635
#define a_DAD 0x0636
#define a_TAH 0x0637
#define a_ZAH 0x0638
#define a_AIN 0x0639
#define a_GHAIN 0x063a
#define a_TATWEEL 0x0640
#define a_FEH 0x0641
#define a_QAF 0x0642
#define a_KAF 0x0643
#define a_LAM 0x0644
#define a_MEEM 0x0645
#define a_NOON 0x0646
#define a_HEH 0x0647
#define a_WAW 0x0648
#define a_ALEF_MAKSURA 0x0649
#define a_YEH 0x064a

#define a_FATHATAN 0x064b
#define a_DAMMATAN 0x064c
#define a_KASRATAN 0x064d
#define a_FATHA 0x064e
#define a_DAMMA 0x064f
#define a_KASRA 0x0650
#define a_SHADDA 0x0651
#define a_SUKUN 0x0652

#define a_MADDA_ABOVE 0x0653
#define a_HAMZA_ABOVE 0x0654
#define a_HAMZA_BELOW 0x0655

#define a_ZERO 0x0660
#define a_ONE 0x0661
#define a_TWO 0x0662
#define a_THREE 0x0663
#define a_FOUR 0x0664
#define a_FIVE 0x0665
#define a_SIX 0x0666
#define a_SEVEN 0x0667
#define a_EIGHT 0x0668
#define a_NINE 0x0669
#define a_PERCENT 0x066a
#define a_DECIMAL 0x066b
#define a_THOUSANDS 0x066c
#define a_STAR 0x066d
#define a_MINI_ALEF 0x0670








#define a_s_FATHATAN 0xfe70
#define a_m_TATWEEL_FATHATAN 0xfe71
#define a_s_DAMMATAN 0xfe72

#define a_s_KASRATAN 0xfe74

#define a_s_FATHA 0xfe76
#define a_m_FATHA 0xfe77
#define a_s_DAMMA 0xfe78
#define a_m_DAMMA 0xfe79
#define a_s_KASRA 0xfe7a
#define a_m_KASRA 0xfe7b
#define a_s_SHADDA 0xfe7c
#define a_m_SHADDA 0xfe7d
#define a_s_SUKUN 0xfe7e
#define a_m_SUKUN 0xfe7f

#define a_s_HAMZA 0xfe80
#define a_s_ALEF_MADDA 0xfe81
#define a_f_ALEF_MADDA 0xfe82
#define a_s_ALEF_HAMZA_ABOVE 0xfe83
#define a_f_ALEF_HAMZA_ABOVE 0xfe84
#define a_s_WAW_HAMZA 0xfe85
#define a_f_WAW_HAMZA 0xfe86
#define a_s_ALEF_HAMZA_BELOW 0xfe87
#define a_f_ALEF_HAMZA_BELOW 0xfe88
#define a_s_YEH_HAMZA 0xfe89
#define a_f_YEH_HAMZA 0xfe8a
#define a_i_YEH_HAMZA 0xfe8b
#define a_m_YEH_HAMZA 0xfe8c
#define a_s_ALEF 0xfe8d
#define a_f_ALEF 0xfe8e
#define a_s_BEH 0xfe8f
#define a_f_BEH 0xfe90
#define a_i_BEH 0xfe91
#define a_m_BEH 0xfe92
#define a_s_TEH_MARBUTA 0xfe93
#define a_f_TEH_MARBUTA 0xfe94
#define a_s_TEH 0xfe95
#define a_f_TEH 0xfe96
#define a_i_TEH 0xfe97
#define a_m_TEH 0xfe98
#define a_s_THEH 0xfe99
#define a_f_THEH 0xfe9a
#define a_i_THEH 0xfe9b
#define a_m_THEH 0xfe9c
#define a_s_JEEM 0xfe9d
#define a_f_JEEM 0xfe9e
#define a_i_JEEM 0xfe9f
#define a_m_JEEM 0xfea0
#define a_s_HAH 0xfea1
#define a_f_HAH 0xfea2
#define a_i_HAH 0xfea3
#define a_m_HAH 0xfea4
#define a_s_KHAH 0xfea5
#define a_f_KHAH 0xfea6
#define a_i_KHAH 0xfea7
#define a_m_KHAH 0xfea8
#define a_s_DAL 0xfea9
#define a_f_DAL 0xfeaa
#define a_s_THAL 0xfeab
#define a_f_THAL 0xfeac
#define a_s_REH 0xfead
#define a_f_REH 0xfeae
#define a_s_ZAIN 0xfeaf
#define a_f_ZAIN 0xfeb0
#define a_s_SEEN 0xfeb1
#define a_f_SEEN 0xfeb2
#define a_i_SEEN 0xfeb3
#define a_m_SEEN 0xfeb4
#define a_s_SHEEN 0xfeb5
#define a_f_SHEEN 0xfeb6
#define a_i_SHEEN 0xfeb7
#define a_m_SHEEN 0xfeb8
#define a_s_SAD 0xfeb9
#define a_f_SAD 0xfeba
#define a_i_SAD 0xfebb
#define a_m_SAD 0xfebc
#define a_s_DAD 0xfebd
#define a_f_DAD 0xfebe
#define a_i_DAD 0xfebf
#define a_m_DAD 0xfec0
#define a_s_TAH 0xfec1
#define a_f_TAH 0xfec2
#define a_i_TAH 0xfec3
#define a_m_TAH 0xfec4
#define a_s_ZAH 0xfec5
#define a_f_ZAH 0xfec6
#define a_i_ZAH 0xfec7
#define a_m_ZAH 0xfec8
#define a_s_AIN 0xfec9
#define a_f_AIN 0xfeca
#define a_i_AIN 0xfecb
#define a_m_AIN 0xfecc
#define a_s_GHAIN 0xfecd
#define a_f_GHAIN 0xfece
#define a_i_GHAIN 0xfecf
#define a_m_GHAIN 0xfed0
#define a_s_FEH 0xfed1
#define a_f_FEH 0xfed2
#define a_i_FEH 0xfed3
#define a_m_FEH 0xfed4
#define a_s_QAF 0xfed5
#define a_f_QAF 0xfed6
#define a_i_QAF 0xfed7
#define a_m_QAF 0xfed8
#define a_s_KAF 0xfed9
#define a_f_KAF 0xfeda
#define a_i_KAF 0xfedb
#define a_m_KAF 0xfedc
#define a_s_LAM 0xfedd
#define a_f_LAM 0xfede
#define a_i_LAM 0xfedf
#define a_m_LAM 0xfee0
#define a_s_MEEM 0xfee1
#define a_f_MEEM 0xfee2
#define a_i_MEEM 0xfee3
#define a_m_MEEM 0xfee4
#define a_s_NOON 0xfee5
#define a_f_NOON 0xfee6
#define a_i_NOON 0xfee7
#define a_m_NOON 0xfee8
#define a_s_HEH 0xfee9
#define a_f_HEH 0xfeea
#define a_i_HEH 0xfeeb
#define a_m_HEH 0xfeec
#define a_s_WAW 0xfeed
#define a_f_WAW 0xfeee
#define a_s_ALEF_MAKSURA 0xfeef
#define a_f_ALEF_MAKSURA 0xfef0
#define a_s_YEH 0xfef1
#define a_f_YEH 0xfef2
#define a_i_YEH 0xfef3
#define a_m_YEH 0xfef4
#define a_s_LAM_ALEF_MADDA_ABOVE 0xfef5
#define a_f_LAM_ALEF_MADDA_ABOVE 0xfef6
#define a_s_LAM_ALEF_HAMZA_ABOVE 0xfef7
#define a_f_LAM_ALEF_HAMZA_ABOVE 0xfef8
#define a_s_LAM_ALEF_HAMZA_BELOW 0xfef9
#define a_f_LAM_ALEF_HAMZA_BELOW 0xfefa
#define a_s_LAM_ALEF 0xfefb
#define a_f_LAM_ALEF 0xfefc

#define a_BYTE_ORDER_MARK 0xfeff


#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "arabic.c.generated.h"
#endif


static bool A_is_a(int cur_c)
{
switch (cur_c) {
case a_HAMZA:
case a_ALEF_MADDA:
case a_ALEF_HAMZA_ABOVE:
case a_WAW_HAMZA:
case a_ALEF_HAMZA_BELOW:
case a_YEH_HAMZA:
case a_ALEF:
case a_BEH:
case a_TEH_MARBUTA:
case a_TEH:
case a_THEH:
case a_JEEM:
case a_HAH:
case a_KHAH:
case a_DAL:
case a_THAL:
case a_REH:
case a_ZAIN:
case a_SEEN:
case a_SHEEN:
case a_SAD:
case a_DAD:
case a_TAH:
case a_ZAH:
case a_AIN:
case a_GHAIN:
case a_TATWEEL:
case a_FEH:
case a_QAF:
case a_KAF:
case a_LAM:
case a_MEEM:
case a_NOON:
case a_HEH:
case a_WAW:
case a_ALEF_MAKSURA:
case a_YEH:
return true;
}

return false;
}


static bool A_is_s(int cur_c)
{
switch (cur_c) {
case a_s_HAMZA:
case a_s_ALEF_MADDA:
case a_s_ALEF_HAMZA_ABOVE:
case a_s_WAW_HAMZA:
case a_s_ALEF_HAMZA_BELOW:
case a_s_YEH_HAMZA:
case a_s_ALEF:
case a_s_BEH:
case a_s_TEH_MARBUTA:
case a_s_TEH:
case a_s_THEH:
case a_s_JEEM:
case a_s_HAH:
case a_s_KHAH:
case a_s_DAL:
case a_s_THAL:
case a_s_REH:
case a_s_ZAIN:
case a_s_SEEN:
case a_s_SHEEN:
case a_s_SAD:
case a_s_DAD:
case a_s_TAH:
case a_s_ZAH:
case a_s_AIN:
case a_s_GHAIN:
case a_s_FEH:
case a_s_QAF:
case a_s_KAF:
case a_s_LAM:
case a_s_MEEM:
case a_s_NOON:
case a_s_HEH:
case a_s_WAW:
case a_s_ALEF_MAKSURA:
case a_s_YEH:
return true;
}

return false;
}


static bool A_is_f(int cur_c)
{
switch (cur_c) {
case a_f_ALEF_MADDA:
case a_f_ALEF_HAMZA_ABOVE:
case a_f_WAW_HAMZA:
case a_f_ALEF_HAMZA_BELOW:
case a_f_YEH_HAMZA:
case a_f_ALEF:
case a_f_BEH:
case a_f_TEH_MARBUTA:
case a_f_TEH:
case a_f_THEH:
case a_f_JEEM:
case a_f_HAH:
case a_f_KHAH:
case a_f_DAL:
case a_f_THAL:
case a_f_REH:
case a_f_ZAIN:
case a_f_SEEN:
case a_f_SHEEN:
case a_f_SAD:
case a_f_DAD:
case a_f_TAH:
case a_f_ZAH:
case a_f_AIN:
case a_f_GHAIN:
case a_f_FEH:
case a_f_QAF:
case a_f_KAF:
case a_f_LAM:
case a_f_MEEM:
case a_f_NOON:
case a_f_HEH:
case a_f_WAW:
case a_f_ALEF_MAKSURA:
case a_f_YEH:
case a_f_LAM_ALEF_MADDA_ABOVE:
case a_f_LAM_ALEF_HAMZA_ABOVE:
case a_f_LAM_ALEF_HAMZA_BELOW:
case a_f_LAM_ALEF:
return true;
}
return false;
}


static int chg_c_a2s(int cur_c)
{
switch (cur_c) {
case a_HAMZA: return a_s_HAMZA;
case a_ALEF_MADDA: return a_s_ALEF_MADDA;
case a_ALEF_HAMZA_ABOVE: return a_s_ALEF_HAMZA_ABOVE;
case a_WAW_HAMZA: return a_s_WAW_HAMZA;
case a_ALEF_HAMZA_BELOW: return a_s_ALEF_HAMZA_BELOW;
case a_YEH_HAMZA: return a_s_YEH_HAMZA;
case a_ALEF: return a_s_ALEF;
case a_TEH_MARBUTA: return a_s_TEH_MARBUTA;
case a_DAL: return a_s_DAL;
case a_THAL: return a_s_THAL;
case a_REH: return a_s_REH;
case a_ZAIN: return a_s_ZAIN;
case a_TATWEEL: return cur_c; 
case a_WAW: return a_s_WAW;
case a_ALEF_MAKSURA: return a_s_ALEF_MAKSURA;
case a_BEH: return a_s_BEH;
case a_TEH: return a_s_TEH;
case a_THEH: return a_s_THEH;
case a_JEEM: return a_s_JEEM;
case a_HAH: return a_s_HAH;
case a_KHAH: return a_s_KHAH;
case a_SEEN: return a_s_SEEN;
case a_SHEEN: return a_s_SHEEN;
case a_SAD: return a_s_SAD;
case a_DAD: return a_s_DAD;
case a_TAH: return a_s_TAH;
case a_ZAH: return a_s_ZAH;
case a_AIN: return a_s_AIN;
case a_GHAIN: return a_s_GHAIN;
case a_FEH: return a_s_FEH;
case a_QAF: return a_s_QAF;
case a_KAF: return a_s_KAF;
case a_LAM: return a_s_LAM;
case a_MEEM: return a_s_MEEM;
case a_NOON: return a_s_NOON;
case a_HEH: return a_s_HEH;
case a_YEH: return a_s_YEH;
}
return 0;
}


static int chg_c_a2i(int cur_c)
{
switch (cur_c) {
case a_YEH_HAMZA: return a_i_YEH_HAMZA;
case a_HAMZA: return a_s_HAMZA; 
case a_ALEF_MADDA: return a_s_ALEF_MADDA; 
case a_ALEF_HAMZA_ABOVE: return a_s_ALEF_HAMZA_ABOVE; 
case a_WAW_HAMZA: return a_s_WAW_HAMZA; 
case a_ALEF_HAMZA_BELOW: return a_s_ALEF_HAMZA_BELOW; 
case a_ALEF: return a_s_ALEF; 
case a_TEH_MARBUTA: return a_s_TEH_MARBUTA; 
case a_DAL: return a_s_DAL; 
case a_THAL: return a_s_THAL; 
case a_REH: return a_s_REH; 
case a_ZAIN: return a_s_ZAIN; 
case a_TATWEEL: return cur_c; 
case a_WAW: return a_s_WAW; 
case a_ALEF_MAKSURA: return a_s_ALEF_MAKSURA; 
case a_BEH: return a_i_BEH;
case a_TEH: return a_i_TEH;
case a_THEH: return a_i_THEH;
case a_JEEM: return a_i_JEEM;
case a_HAH: return a_i_HAH;
case a_KHAH: return a_i_KHAH;
case a_SEEN: return a_i_SEEN;
case a_SHEEN: return a_i_SHEEN;
case a_SAD: return a_i_SAD;
case a_DAD: return a_i_DAD;
case a_TAH: return a_i_TAH;
case a_ZAH: return a_i_ZAH;
case a_AIN: return a_i_AIN;
case a_GHAIN: return a_i_GHAIN;
case a_FEH: return a_i_FEH;
case a_QAF: return a_i_QAF;
case a_KAF: return a_i_KAF;
case a_LAM: return a_i_LAM;
case a_MEEM: return a_i_MEEM;
case a_NOON: return a_i_NOON;
case a_HEH: return a_i_HEH;
case a_YEH: return a_i_YEH;
}
return 0;
}


static int chg_c_a2m(int cur_c)
{
switch (cur_c) {
case a_HAMZA: return a_s_HAMZA; 
case a_ALEF_MADDA: return a_f_ALEF_MADDA; 
case a_ALEF_HAMZA_ABOVE: return a_f_ALEF_HAMZA_ABOVE; 
case a_WAW_HAMZA: return a_f_WAW_HAMZA; 
case a_ALEF_HAMZA_BELOW: return a_f_ALEF_HAMZA_BELOW; 
case a_YEH_HAMZA: return a_m_YEH_HAMZA;
case a_ALEF: return a_f_ALEF; 
case a_BEH: return a_m_BEH;
case a_TEH_MARBUTA: return a_f_TEH_MARBUTA; 
case a_TEH: return a_m_TEH;
case a_THEH: return a_m_THEH;
case a_JEEM: return a_m_JEEM;
case a_HAH: return a_m_HAH;
case a_KHAH: return a_m_KHAH;
case a_DAL: return a_f_DAL; 
case a_THAL: return a_f_THAL; 
case a_REH: return a_f_REH; 
case a_ZAIN: return a_f_ZAIN; 
case a_SEEN: return a_m_SEEN;
case a_SHEEN: return a_m_SHEEN;
case a_SAD: return a_m_SAD;
case a_DAD: return a_m_DAD;
case a_TAH: return a_m_TAH;
case a_ZAH: return a_m_ZAH;
case a_AIN: return a_m_AIN;
case a_GHAIN: return a_m_GHAIN;
case a_TATWEEL: return cur_c; 
case a_FEH: return a_m_FEH;
case a_QAF: return a_m_QAF;
case a_KAF: return a_m_KAF;
case a_LAM: return a_m_LAM;
case a_MEEM: return a_m_MEEM;
case a_NOON: return a_m_NOON;
case a_HEH: return a_m_HEH;
case a_WAW: return a_f_WAW; 
case a_ALEF_MAKSURA: return a_f_ALEF_MAKSURA; 
case a_YEH: return a_m_YEH;
}
return 0;
}


static int chg_c_a2f(int cur_c)
{









switch (cur_c) {
case a_HAMZA: return a_s_HAMZA; 
case a_ALEF_MADDA: return a_f_ALEF_MADDA;
case a_ALEF_HAMZA_ABOVE: return a_f_ALEF_HAMZA_ABOVE;
case a_WAW_HAMZA: return a_f_WAW_HAMZA;
case a_ALEF_HAMZA_BELOW: return a_f_ALEF_HAMZA_BELOW;
case a_YEH_HAMZA: return a_f_YEH_HAMZA;
case a_ALEF: return a_f_ALEF;
case a_BEH: return a_f_BEH;
case a_TEH_MARBUTA: return a_f_TEH_MARBUTA;
case a_TEH: return a_f_TEH;
case a_THEH: return a_f_THEH;
case a_JEEM: return a_f_JEEM;
case a_HAH: return a_f_HAH;
case a_KHAH: return a_f_KHAH;
case a_DAL: return a_f_DAL;
case a_THAL: return a_f_THAL;
case a_REH: return a_f_REH;
case a_ZAIN: return a_f_ZAIN;
case a_SEEN: return a_f_SEEN;
case a_SHEEN: return a_f_SHEEN;
case a_SAD: return a_f_SAD;
case a_DAD: return a_f_DAD;
case a_TAH: return a_f_TAH;
case a_ZAH: return a_f_ZAH;
case a_AIN: return a_f_AIN;
case a_GHAIN: return a_f_GHAIN;
case a_TATWEEL: return cur_c; 
case a_FEH: return a_f_FEH;
case a_QAF: return a_f_QAF;
case a_KAF: return a_f_KAF;
case a_LAM: return a_f_LAM;
case a_MEEM: return a_f_MEEM;
case a_NOON: return a_f_NOON;
case a_HEH: return a_f_HEH;
case a_WAW: return a_f_WAW;
case a_ALEF_MAKSURA: return a_f_ALEF_MAKSURA;
case a_YEH: return a_f_YEH;
}
return 0;
}




#if 0
static int chg_c_i2m(int cur_c)
{
switch (cur_c) {
case a_i_YEH_HAMZA: return a_m_YEH_HAMZA;
case a_i_BEH: return a_m_BEH;
case a_i_TEH: return a_m_TEH;
case a_i_THEH: return a_m_THEH;
case a_i_JEEM: return a_m_JEEM;
case a_i_HAH: return a_m_HAH;
case a_i_KHAH: return a_m_KHAH;
case a_i_SEEN: return a_m_SEEN;
case a_i_SHEEN: return a_m_SHEEN;
case a_i_SAD: return a_m_SAD;
case a_i_DAD: return a_m_DAD;
case a_i_TAH: return a_m_TAH;
case a_i_ZAH: return a_m_ZAH;
case a_i_AIN: return a_m_AIN;
case a_i_GHAIN: return a_m_GHAIN;
case a_i_FEH: return a_m_FEH;
case a_i_QAF: return a_m_QAF;
case a_i_KAF: return a_m_KAF;
case a_i_LAM: return a_m_LAM;
case a_i_MEEM: return a_m_MEEM;
case a_i_NOON: return a_m_NOON;
case a_i_HEH: return a_m_HEH;
case a_i_YEH: return a_m_YEH;
}
return 0;
}
#endif


static int chg_c_f2m(int cur_c)
{
switch (cur_c) {




case a_f_YEH_HAMZA: return a_m_YEH_HAMZA;
case a_f_WAW_HAMZA: 
case a_f_ALEF:
case a_f_TEH_MARBUTA:
case a_f_DAL:
case a_f_THAL:
case a_f_REH:
case a_f_ZAIN:
case a_f_WAW:
case a_f_ALEF_MAKSURA:
return cur_c;
case a_f_BEH: return a_m_BEH;
case a_f_TEH: return a_m_TEH;
case a_f_THEH: return a_m_THEH;
case a_f_JEEM: return a_m_JEEM;
case a_f_HAH: return a_m_HAH;
case a_f_KHAH: return a_m_KHAH;
case a_f_SEEN: return a_m_SEEN;
case a_f_SHEEN: return a_m_SHEEN;
case a_f_SAD: return a_m_SAD;
case a_f_DAD: return a_m_DAD;
case a_f_TAH: return a_m_TAH;
case a_f_ZAH: return a_m_ZAH;
case a_f_AIN: return a_m_AIN;
case a_f_GHAIN: return a_m_GHAIN;
case a_f_FEH: return a_m_FEH;
case a_f_QAF: return a_m_QAF;
case a_f_KAF: return a_m_KAF;
case a_f_LAM: return a_m_LAM;
case a_f_MEEM: return a_m_MEEM;
case a_f_NOON: return a_m_NOON;
case a_f_HEH: return a_m_HEH;
case a_f_YEH: return a_m_YEH;





}
return 0;
}


static int chg_c_laa2i(int hid_c)
{
switch (hid_c) {
case a_ALEF_MADDA: return a_s_LAM_ALEF_MADDA_ABOVE;
case a_ALEF_HAMZA_ABOVE: return a_s_LAM_ALEF_HAMZA_ABOVE;
case a_ALEF_HAMZA_BELOW: return a_s_LAM_ALEF_HAMZA_BELOW;
case a_ALEF: return a_s_LAM_ALEF;
}
return 0;
}


static int chg_c_laa2f(int hid_c)
{
switch (hid_c) {
case a_ALEF_MADDA: return a_f_LAM_ALEF_MADDA_ABOVE;
case a_ALEF_HAMZA_ABOVE: return a_f_LAM_ALEF_HAMZA_ABOVE;
case a_ALEF_HAMZA_BELOW: return a_f_LAM_ALEF_HAMZA_BELOW;
case a_ALEF: return a_f_LAM_ALEF;
}
return 0;
}


static int half_shape(int c)
{
if (A_is_a(c)) {
return chg_c_a2i(c);
}

if (A_is_valid(c) && A_is_f(c)) {
return chg_c_f2m(c);
}
return 0;
}








int arabic_shape(int c, int *ccp, int *c1p, int prev_c, int prev_c1,
int next_c)
{

if (!A_is_ok(c)) {
return c;
}


int shape_c = half_shape(prev_c);


int curr_c = c;

int curr_laa = A_firstc_laa(c, *c1p);
int prev_laa = A_firstc_laa(prev_c, prev_c1);

if (curr_laa) {
if (A_is_valid(prev_c) && !A_is_f(shape_c) && !A_is_s(shape_c)
&& !prev_laa) {
curr_c = chg_c_laa2f(curr_laa);
} else {
curr_c = chg_c_laa2i(curr_laa);
}


*c1p = 0;
} else if (!A_is_valid(prev_c) && A_is_valid(next_c)) {
curr_c = chg_c_a2i(c);
} else if (!shape_c || A_is_f(shape_c) || A_is_s(shape_c) || prev_laa) {
curr_c = A_is_valid(next_c) ? chg_c_a2i(c) : chg_c_a2s(c);
} else if (A_is_valid(next_c)) {
#if 0
curr_c = A_is_iso(c) ? chg_c_a2m(c) : chg_c_i2m(c);
#else
curr_c = A_is_iso(c) ? chg_c_a2m(c) : 0;
#endif
} else if (A_is_valid(prev_c)) {
curr_c = chg_c_a2f(c);
} else {
curr_c = chg_c_a2s(c);
}



if (curr_c == NUL) {
curr_c = c;
}

if ((curr_c != c) && (ccp != NULL)) {
char_u buf[MB_MAXBYTES + 1];


utf_char2bytes(curr_c, buf);
*ccp = buf[0];
}


return curr_c;
}






bool arabic_combine(int one, int two)
{
if (one == a_LAM) {
return arabic_maycombine(two);
}
return false;
}



bool arabic_maycombine(int two)
{
if (p_arshape && !p_tbidi) {
return two == a_ALEF_MADDA
|| two == a_ALEF_HAMZA_ABOVE
|| two == a_ALEF_HAMZA_BELOW
|| two == a_ALEF;
}
return false;
}




static int A_firstc_laa(int c, int c1)
{
if ((c1 != NUL) && (c == a_LAM) && !A_is_harakat(c1)) {
return c1;
}
return 0;
}



static bool A_is_harakat(int c)
{
return c >= a_FATHATAN && c <= a_SUKUN;
}



static bool A_is_iso(int c)
{
return ((c >= a_HAMZA && c <= a_GHAIN)
|| (c >= a_TATWEEL && c <= a_HAMZA_BELOW)
|| c == a_MINI_ALEF);
}



static bool A_is_formb(int c)
{
return ((c >= a_s_FATHATAN && c <= a_s_DAMMATAN)
|| c == a_s_KASRATAN
|| (c >= a_s_FATHA && c <= a_f_LAM_ALEF)
|| c == a_BYTE_ORDER_MARK);
}


static bool A_is_ok(int c)
{
return A_is_iso(c) || A_is_formb(c);
}



static bool A_is_valid(int c)
{
return A_is_ok(c) && !A_is_special(c);
}



static bool A_is_special(int c)
{
return c == a_HAMZA || c == a_s_HAMZA;
}
