struct {
const char *name;
ut16 val;
} msr_const[] = {
{ "MDCCSR_EL0", 0x9808 },
{ "DBGDTRRX_EL0", 0x9828 },
{ "MDRAR_EL1", 0x8080 },
{ "OSLSR_EL1", 0x808c },
{ "DBGAUTHSTATUS_EL1", 0x83f6 },
{ "PMCEID0_EL0", 0xdce6 },
{ "PMCEID1_EL0", 0xdce7 },
{ "MIDR_EL1", 0xc000 },
{ "CCSIDR_EL1", 0xc800 },
{ "CLIDR_EL1", 0xc801 },
{ "CTR_EL0", 0xd801 },
{ "MPIDR_EL1", 0xc005 },
{ "REVIDR_EL1", 0xc006 },
{ "AIDR_EL1", 0xc807 },
{ "DCZID_EL0", 0xd807 },
{ "ID_PFR0_EL1", 0xc008 },
{ "ID_PFR1_EL1", 0xc009 },
{ "ID_DFR0_EL1", 0xc00a },
{ "ID_AFR0_EL1", 0xc00b },
{ "ID_MMFR0_EL1", 0xc00c },
{ "ID_MMFR1_EL1", 0xc00d },
{ "ID_MMFR2_EL1", 0xc00e },
{ "ID_MMFR3_EL1", 0xc00f },
{ "ID_ISAR0_EL1", 0xc010 },
{ "ID_ISAR1_EL1", 0xc011 },
{ "ID_ISAR2_EL1", 0xc012 },
{ "ID_ISAR3_EL1", 0xc013 },
{ "ID_ISAR4_EL1", 0xc014 },
{ "ID_ISAR5_EL1", 0xc015 },
{ "ID_A64PFR0_EL1", 0xc020 },
{ "ID_A64PFR1_EL1", 0xc021 },
{ "ID_A64DFR0_EL1", 0xc028 },
{ "ID_A64DFR1_EL1", 0xc029 },
{ "ID_A64AFR0_EL1", 0xc02c },
{ "ID_A64AFR1_EL1", 0xc02d },
{ "ID_A64ISAR0_EL1", 0xc030 },
{ "ID_A64ISAR1_EL1", 0xc031 },
{ "ID_A64MMFR0_EL1", 0xc038 },
{ "ID_A64MMFR1_EL1", 0xc039 },
{ "ID_A64MMFR2_EL1", 0xc03a },
{ "MVFR0_EL1", 0xc018 },
{ "MVFR1_EL1", 0xc019 },
{ "MVFR2_EL1", 0xc01a },
{ "RVBAR_EL1", 0xc601 },
{ "RVBAR_EL2", 0xe601 },
{ "RVBAR_EL3", 0xf601 },
{ "ISR_EL1", 0xc608 },
{ "CNTPCT_EL0", 0xdf01 },
{ "CNTVCT_EL0", 0xdf02 },
{ "ID_MMFR4_EL1", 0xc016 },
{ "TRCSTATR", 0x8818 },
{ "TRCIDR8", 0x8806 },
{ "TRCIDR9", 0x880e },
{ "TRCIDR10", 0x8816 },
{ "TRCIDR11", 0x881e },
{ "TRCIDR12", 0x8826 },
{ "TRCIDR13", 0x882e },
{ "TRCIDR0", 0x8847 },
{ "TRCIDR1", 0x884f },
{ "TRCIDR2", 0x8857 },
{ "TRCIDR3", 0x885f },
{ "TRCIDR4", 0x8867 },
{ "TRCIDR5", 0x886f },
{ "TRCIDR6", 0x8877 },
{ "TRCIDR7", 0x887f },
{ "TRCOSLSR", 0x888c },
{ "TRCPDSR", 0x88ac },
{ "TRCDEVAFF0", 0x8bd6 },
{ "TRCDEVAFF1", 0x8bde },
{ "TRCLSR", 0x8bee },
{ "TRCAUTHSTATUS", 0x8bf6 },
{ "TRCDEVARCH", 0x8bfe },
{ "TRCDEVID", 0x8b97 },
{ "TRCDEVTYPE", 0x8b9f },
{ "TRCPIDR4", 0x8ba7 },
{ "TRCPIDR5", 0x8baf },
{ "TRCPIDR6", 0x8bb7 },
{ "TRCPIDR7", 0x8bbf },
{ "TRCPIDR0", 0x8bc7 },
{ "TRCPIDR1", 0x8bcf },
{ "TRCPIDR2", 0x8bd7 },
{ "TRCPIDR3", 0x8bdf },
{ "TRCCIDR0", 0x8be7 },
{ "TRCCIDR1", 0x8bef },
{ "TRCCIDR2", 0x8bf7 },
{ "TRCCIDR3", 0x8bff },
{ "ICC_IAR1_EL1", 0xc660 },
{ "ICC_IAR0_EL1", 0xc640 },
{ "ICC_HPPIR1_EL1", 0xc662 },
{ "ICC_HPPIR0_EL1", 0xc642 },
{ "ICC_RPR_EL1", 0xc65b },
{ "ICH_VTR_EL2", 0xe659 },
{ "ICH_EISR_EL2", 0xe65b },
{ "ICH_EISR_EL2", 0xe65b },
{ "DBGDTRTX_EL0", 0x9828 },
{ "OSLAR_EL1", 0x8084 },
{ "PMSWINC_EL0", 0xdce4 },
{ "TRCOSLAR", 0x8884 },
{ "TRCLAR", 0x8be6 },
{ "ICC_EOIR1_EL1", 0xc661 },
{ "ICC_EOIR0_EL1", 0xc641 },
{ "ICC_DIR_EL1", 0xc659 },
{ "ICC_SGI1R_EL1", 0xc65d },
{ "ICC_ASGI1R_EL1", 0xc65e },
{ "ICC_ASGI1R_EL1", 0xc65e },
{ "OSDTRRX_EL1", 0x8002 },
{ "OSDTRTX_EL1", 0x801a },
{ "TEECR32_EL1", 0x9000 },
{ "MDCCINT_EL1", 0x8010 },
{ "MDSCR_EL1", 0x8012 },
{ "DBGDTR_EL0", 0x9820 },
{ "OSECCR_EL1", 0x8032 },
{ "DBGVCR32_EL2", 0xa038 },
{ "DBGBVR0_EL1", 0x8004 },
{ "DBGBVR1_EL1", 0x800c },
{ "DBGBVR2_EL1", 0x8014 },
{ "DBGBVR3_EL1", 0x801c },
{ "DBGBVR4_EL1", 0x8024 },
{ "DBGBVR5_EL1", 0x802c },
{ "DBGBVR6_EL1", 0x8034 },
{ "DBGBVR7_EL1", 0x803c },
{ "DBGBVR8_EL1", 0x8044 },
{ "DBGBVR9_EL1", 0x804c },
{ "DBGBVR10_EL1", 0x8054 },
{ "DBGBVR11_EL1", 0x805c },
{ "DBGBVR12_EL1", 0x8064 },
{ "DBGBVR13_EL1", 0x806c },
{ "DBGBVR14_EL1", 0x8074 },
{ "DBGBVR15_EL1", 0x807c },
{ "DBGBCR0_EL1", 0x8005 },
{ "DBGBCR1_EL1", 0x800d },
{ "DBGBCR2_EL1", 0x8015 },
{ "DBGBCR3_EL1", 0x801d },
{ "DBGBCR4_EL1", 0x8025 },
{ "DBGBCR5_EL1", 0x802d },
{ "DBGBCR6_EL1", 0x8035 },
{ "DBGBCR7_EL1", 0x803d },
{ "DBGBCR8_EL1", 0x8045 },
{ "DBGBCR9_EL1", 0x804d },
{ "DBGBCR10_EL1", 0x8055 },
{ "DBGBCR11_EL1", 0x805d },
{ "DBGBCR12_EL1", 0x8065 },
{ "DBGBCR13_EL1", 0x806d },
{ "DBGBCR14_EL1", 0x8075 },
{ "DBGBCR15_EL1", 0x807d },
{ "DBGWVR0_EL1", 0x8006 },
{ "DBGWVR1_EL1", 0x800e },
{ "DBGWVR2_EL1", 0x8016 },
{ "DBGWVR3_EL1", 0x801e },
{ "DBGWVR4_EL1", 0x8026 },
{ "DBGWVR5_EL1", 0x802e },
{ "DBGWVR6_EL1", 0x8036 },
{ "DBGWVR7_EL1", 0x803e },
{ "DBGWVR8_EL1", 0x8046 },
{ "DBGWVR9_EL1", 0x804e },
{ "DBGWVR10_EL1", 0x8056 },
{ "DBGWVR11_EL1", 0x805e },
{ "DBGWVR12_EL1", 0x8066 },
{ "DBGWVR13_EL1", 0x806e },
{ "DBGWVR14_EL1", 0x8076 },
{ "DBGWVR15_EL1", 0x807e },
{ "DBGWCR0_EL1", 0x8007 },
{ "DBGWCR1_EL1", 0x800f },
{ "DBGWCR2_EL1", 0x8017 },
{ "DBGWCR3_EL1", 0x801f },
{ "DBGWCR4_EL1", 0x8027 },
{ "DBGWCR5_EL1", 0x802f },
{ "DBGWCR6_EL1", 0x8037 },
{ "DBGWCR7_EL1", 0x803f },
{ "DBGWCR8_EL1", 0x8047 },
{ "DBGWCR9_EL1", 0x804f },
{ "DBGWCR10_EL1", 0x8057 },
{ "DBGWCR11_EL1", 0x805f },
{ "DBGWCR12_EL1", 0x8067 },
{ "DBGWCR13_EL1", 0x806f },
{ "DBGWCR14_EL1", 0x8077 },
{ "DBGWCR15_EL1", 0x807f },
{ "TEEHBR32_EL1", 0x9080 },
{ "OSDLR_EL1", 0x809c },
{ "DBGPRCR_EL1", 0x80a4 },
{ "DBGCLAIMSET_EL1", 0x83c6 },
{ "DBGCLAIMCLR_EL1", 0x83ce },
{ "CSSELR_EL1", 0xd000 },
{ "VPIDR_EL2", 0xe000 },
{ "VMPIDR_EL2", 0xe005 },
{ "CPACR_EL1", 0xc082 },
{ "SCTLR_EL1", 0xc080 },
{ "SCTLR_EL2", 0xe080 },
{ "SCTLR_EL3", 0xf080 },
{ "ACTLR_EL1", 0xc081 },
{ "ACTLR_EL2", 0xe081 },
{ "ACTLR_EL3", 0xf081 },
{ "HCR_EL2", 0xe088 },
{ "SCR_EL3", 0xf088 },
{ "MDCR_EL2", 0xe089 },
{ "SDER32_EL3", 0xf089 },
{ "CPTR_EL2", 0xe08a },
{ "CPTR_EL3", 0xf08a },
{ "HSTR_EL2", 0xe08b },
{ "HACR_EL2", 0xe08f },
{ "MDCR_EL3", 0xf099 },
{ "TTBR0_EL1", 0xc100 },
{ "TTBR0_EL2", 0xe100 },
{ "TTBR0_EL3", 0xf100 },
{ "TTBR1_EL1", 0xc101 },
{ "TCR_EL1", 0xc102 },
{ "TCR_EL2", 0xe102 },
{ "TCR_EL3", 0xf102 },
{ "VTTBR_EL2", 0xe108 },
{ "VTCR_EL2", 0xe10a },
{ "DACR32_EL2", 0xe180 },
{ "SPSR_EL1", 0xc200 },
{ "SPSR_EL2", 0xe200 },
{ "SPSR_EL3", 0xf200 },
{ "ELR_EL1", 0xc201 },
{ "ELR_EL2", 0xe201 },
{ "ELR_EL3", 0xf201 },
{ "SP_EL0", 0xc208 },
{ "SP_EL1", 0xe208 },
{ "SP_EL2", 0xf208 },
{ "SPSel", 0xc210 },
{ "NZCV", 0xda10 },
{ "DAIF", 0xda11 },
{ "DAIFSet", 0x36 },
{ "DAIFClr", 0x37 },
{ "CurrentEL", 0xc212 },
{ "SPSR_irq", 0xe218 },
{ "SPSR_abt", 0xe219 },
{ "SPSR_und", 0xe21a },
{ "SPSR_fiq", 0xe21b },
{ "FPCR", 0xda20 },
{ "FPSR", 0xda21 },
{ "DSPSR_EL0", 0xda28 },
{ "DLR_EL0", 0xda29 },
{ "IFSR32_EL2", 0xe281 },
{ "AFSR0_EL1", 0xc288 },
{ "AFSR0_EL2", 0xe288 },
{ "AFSR0_EL3", 0xf288 },
{ "AFSR1_EL1", 0xc289 },
{ "AFSR1_EL2", 0xe289 },
{ "AFSR1_EL3", 0xf289 },
{ "ESR_EL1", 0xc290 },
{ "ESR_EL2", 0xe290 },
{ "ESR_EL3", 0xf290 },
{ "FPEXC32_EL2", 0xe298 },
{ "FAR_EL1", 0xc300 },
{ "FAR_EL2", 0xe300 },
{ "FAR_EL3", 0xf300 },
{ "HPFAR_EL2", 0xe304 },
{ "PAR_EL1", 0xc3a0 },
{ "PMCR_EL0", 0xdce0 },
{ "PMCNTENSET_EL0", 0xdce1 },
{ "PMCNTENCLR_EL0", 0xdce2 },
{ "PMOVSCLR_EL0", 0xdce3 },
{ "PMSELR_EL0", 0xdce5 },
{ "PMCCNTR_EL0", 0xdce8 },
{ "PMXEVTYPER_EL0", 0xdce9 },
{ "PMXEVCNTR_EL0", 0xdcea },
{ "PMUSERENR_EL0", 0xdcf0 },
{ "PMINTENSET_EL1", 0xc4f1 },
{ "PMINTENCLR_EL1", 0xc4f2 },
{ "PMOVSSET_EL0", 0xdcf3 },
{ "MAIR_EL1", 0xc510 },
{ "MAIR_EL2", 0xe510 },
{ "MAIR_EL3", 0xf510 },
{ "AMAIR_EL1", 0xc518 },
{ "AMAIR_EL2", 0xe518 },
{ "AMAIR_EL3", 0xf518 },
{ "VBAR_EL1", 0xc600 },
{ "VBAR_EL2", 0xe600 },
{ "VBAR_EL3", 0xf600 },
{ "RMR_EL1", 0xc602 },
{ "RMR_EL2", 0xe602 },
{ "RMR_EL3", 0xf602 },
{ "CONTEXTIDR_EL1", 0xc681 },
{ "TPIDR_EL0", 0xde82 },
{ "TPIDR_EL2", 0xe682 },
{ "TPIDR_EL3", 0xf682 },
{ "TPIDRRO_EL0", 0xde83 },
{ "TPIDR_EL1", 0xc684 },
{ "CNTFRQ_EL0", 0xdf00 },
{ "CNTVOFF_EL2", 0xe703 },
{ "CNTKCTL_EL1", 0xc708 },
{ "CNTHCTL_EL2", 0xe708 },
{ "CNTP_TVAL_EL0", 0xdf10 },
{ "CNTHP_TVAL_EL2", 0xe710 },
{ "CNTPS_TVAL_EL1", 0xff10 },
{ "CNTP_CTL_EL0", 0xdf11 },
{ "CNTHP_CTL_EL2", 0xe711 },
{ "CNTPS_CTL_EL1", 0xff11 },
{ "CNTP_CVAL_EL0", 0xdf12 },
{ "CNTHP_CVAL_EL2", 0xe712 },
{ "CNTPS_CVAL_EL1", 0xff12 },
{ "CNTV_TVAL_EL0", 0xdf18 },
{ "CNTV_CTL_EL0", 0xdf19 },
{ "CNTV_CVAL_EL0", 0xdf1a },
{ "PMEVCNTR0_EL0", 0xdf40 },
{ "PMEVCNTR1_EL0", 0xdf41 },
{ "PMEVCNTR2_EL0", 0xdf42 },
{ "PMEVCNTR3_EL0", 0xdf43 },
{ "PMEVCNTR4_EL0", 0xdf44 },
{ "PMEVCNTR5_EL0", 0xdf45 },
{ "PMEVCNTR6_EL0", 0xdf46 },
{ "PMEVCNTR7_EL0", 0xdf47 },
{ "PMEVCNTR8_EL0", 0xdf48 },
{ "PMEVCNTR9_EL0", 0xdf49 },
{ "PMEVCNTR10_EL0", 0xdf4a },
{ "PMEVCNTR11_EL0", 0xdf4b },
{ "PMEVCNTR12_EL0", 0xdf4c },
{ "PMEVCNTR13_EL0", 0xdf4d },
{ "PMEVCNTR14_EL0", 0xdf4e },
{ "PMEVCNTR15_EL0", 0xdf4f },
{ "PMEVCNTR16_EL0", 0xdf50 },
{ "PMEVCNTR17_EL0", 0xdf51 },
{ "PMEVCNTR18_EL0", 0xdf52 },
{ "PMEVCNTR19_EL0", 0xdf53 },
{ "PMEVCNTR20_EL0", 0xdf54 },
{ "PMEVCNTR21_EL0", 0xdf55 },
{ "PMEVCNTR22_EL0", 0xdf56 },
{ "PMEVCNTR23_EL0", 0xdf57 },
{ "PMEVCNTR24_EL0", 0xdf58 },
{ "PMEVCNTR25_EL0", 0xdf59 },
{ "PMEVCNTR26_EL0", 0xdf5a },
{ "PMEVCNTR27_EL0", 0xdf5b },
{ "PMEVCNTR28_EL0", 0xdf5c },
{ "PMEVCNTR29_EL0", 0xdf5d },
{ "PMEVCNTR30_EL0", 0xdf5e },
{ "PMCCFILTR_EL0", 0xdf7f },
{ "PMEVTYPER0_EL0", 0xdf60 },
{ "PMEVTYPER1_EL0", 0xdf61 },
{ "PMEVTYPER2_EL0", 0xdf62 },
{ "PMEVTYPER3_EL0", 0xdf63 },
{ "PMEVTYPER4_EL0", 0xdf64 },
{ "PMEVTYPER5_EL0", 0xdf65 },
{ "PMEVTYPER6_EL0", 0xdf66 },
{ "PMEVTYPER7_EL0", 0xdf67 },
{ "PMEVTYPER8_EL0", 0xdf68 },
{ "PMEVTYPER9_EL0", 0xdf69 },
{ "PMEVTYPER10_EL0", 0xdf6a },
{ "PMEVTYPER11_EL0", 0xdf6b },
{ "PMEVTYPER12_EL0", 0xdf6c },
{ "PMEVTYPER13_EL0", 0xdf6d },
{ "PMEVTYPER14_EL0", 0xdf6e },
{ "PMEVTYPER15_EL0", 0xdf6f },
{ "PMEVTYPER16_EL0", 0xdf70 },
{ "PMEVTYPER17_EL0", 0xdf71 },
{ "PMEVTYPER18_EL0", 0xdf72 },
{ "PMEVTYPER19_EL0", 0xdf73 },
{ "PMEVTYPER20_EL0", 0xdf74 },
{ "PMEVTYPER21_EL0", 0xdf75 },
{ "PMEVTYPER22_EL0", 0xdf76 },
{ "PMEVTYPER23_EL0", 0xdf77 },
{ "PMEVTYPER24_EL0", 0xdf78 },
{ "PMEVTYPER25_EL0", 0xdf79 },
{ "PMEVTYPER26_EL0", 0xdf7a },
{ "PMEVTYPER27_EL0", 0xdf7b },
{ "PMEVTYPER28_EL0", 0xdf7c },
{ "PMEVTYPER29_EL0", 0xdf7d },
{ "PMEVTYPER30_EL0", 0xdf7e },
{ "TRCPRGCTLR", 0x8808 },
{ "TRCPROCSELR", 0x8810 },
{ "TRCCONFIGR", 0x8820 },
{ "TRCAUXCTLR", 0x8830 },
{ "TRCEVENTCTL0R", 0x8840 },
{ "TRCEVENTCTL1R", 0x8848 },
{ "TRCSTALLCTLR", 0x8858 },
{ "TRCTSCTLR", 0x8860 },
{ "TRCSYNCPR", 0x8868 },
{ "TRCCCCTLR", 0x8870 },
{ "TRCBBCTLR", 0x8878 },
{ "TRCTRACEIDR", 0x8801 },
{ "TRCQCTLR", 0x8809 },
{ "TRCVICTLR", 0x8802 },
{ "TRCVIIECTLR", 0x880a },
{ "TRCVISSCTLR", 0x8812 },
{ "TRCVIPCSSCTLR", 0x881a },
{ "TRCVDCTLR", 0x8842 },
{ "TRCVDSACCTLR", 0x884a },
{ "TRCVDARCCTLR", 0x8852 },
{ "TRCSEQEVR0", 0x8804 },
{ "TRCSEQEVR1", 0x880c },
{ "TRCSEQEVR2", 0x8814 },
{ "TRCSEQRSTEVR", 0x8834 },
{ "TRCSEQSTR", 0x883c },
{ "TRCEXTINSELR", 0x8844 },
{ "TRCCNTRLDVR0", 0x8805 },
{ "TRCCNTRLDVR1", 0x880d },
{ "TRCCNTRLDVR2", 0x8815 },
{ "TRCCNTRLDVR3", 0x881d },
{ "TRCCNTCTLR0", 0x8825 },
{ "TRCCNTCTLR1", 0x882d },
{ "TRCCNTCTLR2", 0x8835 },
{ "TRCCNTCTLR3", 0x883d },
{ "TRCCNTVR0", 0x8845 },
{ "TRCCNTVR1", 0x884d },
{ "TRCCNTVR2", 0x8855 },
{ "TRCCNTVR3", 0x885d },
{ "TRCIMSPEC0", 0x8807 },
{ "TRCIMSPEC1", 0x880f },
{ "TRCIMSPEC2", 0x8817 },
{ "TRCIMSPEC3", 0x881f },
{ "TRCIMSPEC4", 0x8827 },
{ "TRCIMSPEC5", 0x882f },
{ "TRCIMSPEC6", 0x8837 },
{ "TRCIMSPEC7", 0x883f },
{ "TRCRSCTLR2", 0x8890 },
{ "TRCRSCTLR3", 0x8898 },
{ "TRCRSCTLR4", 0x88a0 },
{ "TRCRSCTLR5", 0x88a8 },
{ "TRCRSCTLR6", 0x88b0 },
{ "TRCRSCTLR7", 0x88b8 },
{ "TRCRSCTLR8", 0x88c0 },
{ "TRCRSCTLR9", 0x88c8 },
{ "TRCRSCTLR10", 0x88d0 },
{ "TRCRSCTLR11", 0x88d8 },
{ "TRCRSCTLR12", 0x88e0 },
{ "TRCRSCTLR13", 0x88e8 },
{ "TRCRSCTLR14", 0x88f0 },
{ "TRCRSCTLR15", 0x88f8 },
{ "TRCRSCTLR16", 0x8881 },
{ "TRCRSCTLR17", 0x8889 },
{ "TRCRSCTLR18", 0x8891 },
{ "TRCRSCTLR19", 0x8899 },
{ "TRCRSCTLR20", 0x88a1 },
{ "TRCRSCTLR21", 0x88a9 },
{ "TRCRSCTLR22", 0x88b1 },
{ "TRCRSCTLR23", 0x88b9 },
{ "TRCRSCTLR24", 0x88c1 },
{ "TRCRSCTLR25", 0x88c9 },
{ "TRCRSCTLR26", 0x88d1 },
{ "TRCRSCTLR27", 0x88d9 },
{ "TRCRSCTLR28", 0x88e1 },
{ "TRCRSCTLR29", 0x88e9 },
{ "TRCRSCTLR30", 0x88f1 },
{ "TRCRSCTLR31", 0x88f9 },
{ "TRCSSCCR0", 0x8882 },
{ "TRCSSCCR1", 0x888a },
{ "TRCSSCCR2", 0x8892 },
{ "TRCSSCCR3", 0x889a },
{ "TRCSSCCR4", 0x88a2 },
{ "TRCSSCCR5", 0x88aa },
{ "TRCSSCCR6", 0x88b2 },
{ "TRCSSCCR7", 0x88ba },
{ "TRCSSCSR0", 0x88c2 },
{ "TRCSSCSR1", 0x88ca },
{ "TRCSSCSR2", 0x88d2 },
{ "TRCSSCSR3", 0x88da },
{ "TRCSSCSR4", 0x88e2 },
{ "TRCSSCSR5", 0x88ea },
{ "TRCSSCSR6", 0x88f2 },
{ "TRCSSCSR7", 0x88fa },
{ "TRCSSPCICR0", 0x8883 },
{ "TRCSSPCICR1", 0x888b },
{ "TRCSSPCICR2", 0x8893 },
{ "TRCSSPCICR3", 0x889b },
{ "TRCSSPCICR4", 0x88a3 },
{ "TRCSSPCICR5", 0x88ab },
{ "TRCSSPCICR6", 0x88b3 },
{ "TRCSSPCICR7", 0x88bb },
{ "TRCPDCR", 0x88a4 },
{ "TRCACVR0", 0x8900 },
{ "TRCACVR1", 0x8910 },
{ "TRCACVR2", 0x8920 },
{ "TRCACVR3", 0x8930 },
{ "TRCACVR4", 0x8940 },
{ "TRCACVR5", 0x8950 },
{ "TRCACVR6", 0x8960 },
{ "TRCACVR7", 0x8970 },
{ "TRCACVR8", 0x8901 },
{ "TRCACVR9", 0x8911 },
{ "TRCACVR10", 0x8921 },
{ "TRCACVR11", 0x8931 },
{ "TRCACVR12", 0x8941 },
{ "TRCACVR13", 0x8951 },
{ "TRCACVR14", 0x8961 },
{ "TRCACVR15", 0x8971 },
{ "TRCACATR0", 0x8902 },
{ "TRCACATR1", 0x8912 },
{ "TRCACATR2", 0x8922 },
{ "TRCACATR3", 0x8932 },
{ "TRCACATR4", 0x8942 },
{ "TRCACATR5", 0x8952 },
{ "TRCACATR6", 0x8962 },
{ "TRCACATR7", 0x8972 },
{ "TRCACATR8", 0x8903 },
{ "TRCACATR9", 0x8913 },
{ "TRCACATR10", 0x8923 },
{ "TRCACATR11", 0x8933 },
{ "TRCACATR12", 0x8943 },
{ "TRCACATR13", 0x8953 },
{ "TRCACATR14", 0x8963 },
{ "TRCACATR15", 0x8973 },
{ "TRCDVCVR0", 0x8904 },
{ "TRCDVCVR1", 0x8924 },
{ "TRCDVCVR2", 0x8944 },
{ "TRCDVCVR3", 0x8964 },
{ "TRCDVCVR4", 0x8905 },
{ "TRCDVCVR5", 0x8925 },
{ "TRCDVCVR6", 0x8945 },
{ "TRCDVCVR7", 0x8965 },
{ "TRCDVCMR0", 0x8906 },
{ "TRCDVCMR1", 0x8926 },
{ "TRCDVCMR2", 0x8946 },
{ "TRCDVCMR3", 0x8966 },
{ "TRCDVCMR4", 0x8907 },
{ "TRCDVCMR5", 0x8927 },
{ "TRCDVCMR6", 0x8947 },
{ "TRCDVCMR7", 0x8967 },
{ "TRCCIDCVR0", 0x8980 },
{ "TRCCIDCVR1", 0x8990 },
{ "TRCCIDCVR2", 0x89a0 },
{ "TRCCIDCVR3", 0x89b0 },
{ "TRCCIDCVR4", 0x89c0 },
{ "TRCCIDCVR5", 0x89d0 },
{ "TRCCIDCVR6", 0x89e0 },
{ "TRCCIDCVR7", 0x89f0 },
{ "TRCVMIDCVR0", 0x8981 },
{ "TRCVMIDCVR1", 0x8991 },
{ "TRCVMIDCVR2", 0x89a1 },
{ "TRCVMIDCVR3", 0x89b1 },
{ "TRCVMIDCVR4", 0x89c1 },
{ "TRCVMIDCVR5", 0x89d1 },
{ "TRCVMIDCVR6", 0x89e1 },
{ "TRCVMIDCVR7", 0x89f1 },
{ "TRCCIDCCTLR0", 0x8982 },
{ "TRCCIDCCTLR1", 0x898a },
{ "TRCVMIDCCTLR0", 0x8992 },
{ "TRCVMIDCCTLR1", 0x899a },
{ "TRCITCTRL", 0x8b84 },
{ "TRCCLAIMSET", 0x8bc6 },
{ "TRCCLAIMCLR", 0x8bce },
{ "TRCCLAIMCLR", 0x8bce },
{ "TRCCLAIMCLR", 0x8bce },
{ "ICC_BPR1_EL1", 0xc663 },
{ "ICC_BPR0_EL1", 0xc643 },
{ "ICC_PMR_EL1", 0xc230 },
{ "ICC_CTLR_EL1", 0xc664 },
{ "ICC_CTLR_EL3", 0xf664 },
{ "ICC_SRE_EL1", 0xc665 },
{ "ICC_SRE_EL2", 0xe64d },
{ "ICC_SRE_EL3", 0xf665 },
{ "ICC_IGRPEN0_EL1", 0xc666 },
{ "ICC_IGRPEN1_EL1", 0xc667 },
{ "ICC_IGRPEN1_EL3", 0xf667 },
{ "ICC_SEIEN_EL1", 0xc668 },
{ "ICC_AP0R0_EL1", 0xc644 },
{ "ICC_AP0R1_EL1", 0xc645 },
{ "ICC_AP0R2_EL1", 0xc646 },
{ "ICC_AP0R3_EL1", 0xc647 },
{ "ICC_AP1R0_EL1", 0xc648 },
{ "ICC_AP1R1_EL1", 0xc649 },
{ "ICC_AP1R2_EL1", 0xc64a },
{ "ICC_AP1R3_EL1", 0xc64b },
{ "ICH_AP0R0_EL2", 0xe640 },
{ "ICH_AP0R1_EL2", 0xe641 },
{ "ICH_AP0R2_EL2", 0xe642 },
{ "ICH_AP0R3_EL2", 0xe643 },
{ "ICH_AP1R0_EL2", 0xe648 },
{ "ICH_AP1R1_EL2", 0xe649 },
{ "ICH_AP1R2_EL2", 0xe64a },
{ "ICH_AP1R3_EL2", 0xe64b },
{ "ICH_HCR_EL2", 0xe658 },
{ "ICH_MISR_EL2", 0xe65a },
{ "ICH_VMCR_EL2", 0xe65f },
{ "ICH_VSEIR_EL2", 0xe64c },
{ "ICH_LR0_EL2", 0xe660 },
{ "ICH_LR1_EL2", 0xe661 },
{ "ICH_LR2_EL2", 0xe662 },
{ "ICH_LR3_EL2", 0xe663 },
{ "ICH_LR4_EL2", 0xe664 },
{ "ICH_LR5_EL2", 0xe665 },
{ "ICH_LR6_EL2", 0xe666 },
{ "ICH_LR7_EL2", 0xe667 },
{ "ICH_LR8_EL2", 0xe668 },
{ "ICH_LR9_EL2", 0xe669 },
{ "ICH_LR10_EL2", 0xe66a },
{ "ICH_LR11_EL2", 0xe66b },
{ "ICH_LR12_EL2", 0xe66c },
{ "ICH_LR13_EL2", 0xe66d },
{ "ICH_LR14_EL2", 0xe66e },
{ "ICH_LR15_EL2", 0xe66f },
{ "PAN", 0xc213 },
{ "LORSA_EL1", 0xc520 },
{ "LOREA_EL1", 0xc521 },
{ "LORN_EL1", 0xc522 },
{ "LORC_EL1", 0xc523 },
{ "LORID_EL1", 0xc527 },
{ "TTBR1_EL2", 0xe101 },
{ "CONTEXTIDR_EL2", 0xe681 },
{ "CNTHV_TVAL_EL2", 0xe718 },
{ "CNTHV_CVAL_EL2", 0xe71a },
{ "CNTHV_CTL_EL2", 0xe719 },
{ "SCTLR_EL12", 0xe880 },
{ "CPACR_EL12", 0xe882 },
{ "TTBR0_EL12", 0xe900 },
{ "TTBR1_EL12", 0xe901 },
{ "TCR_EL12", 0xe902 },
{ "AFSR0_EL12", 0xea88 },
{ "AFSR1_EL12", 0xea89 },
{ "ESR_EL12", 0xea90 },
{ "FAR_EL12", 0xeb00 },
{ "MAIR_EL12", 0xed10 },
{ "AMAIR_EL12", 0xed18 },
{ "VBAR_EL12", 0xee00 },
{ "CONTEXTIDR_EL12", 0xee81 },
{ "CNTKCTL_EL12", 0xef08 },
{ "CNTP_TVAL_EL02", 0xef10 },
{ "CNTP_CTL_EL02", 0xef11 },
{ "CNTP_CVAL_EL02", 0xef12 },
{ "CNTV_TVAL_EL02", 0xef18 },
{ "CNTV_CTL_EL02", 0xef19 },
{ "CNTV_CVAL_EL02", 0xef1a },
{ "SPSR_EL12", 0xea00 },
{ "ELR_EL12", 0xea01 },
{ "UAO", 0xc214 },
{ "PMBLIMITR_EL1", 0xc4d0 },
{ "PMBPTR_EL1", 0xc4d1 },
{ "PMBSR_EL1", 0xc4d3 },
{ "PMBIDR_EL1", 0xc4d7 },
{ "PMSCR_EL2", 0xe4c8 },
{ "PMSCR_EL12", 0xecc8 },
{ "PMSCR_EL1", 0xc4c8 },
{ "PMSICR_EL1", 0xc4ca },
{ "PMSIRR_EL1", 0xc4cb },
{ "PMSFCR_EL1", 0xc4cc },
{ "PMSEVFR_EL1", 0xc4cd },
{ "PMSLATFR_EL1", 0xc4ce },
{ "PMSIDR_EL1", 0xc4cf },
{ "CPM_IOACC_CTL_EL3", 0xff90 },
{ NULL, 0x0000 }
};
