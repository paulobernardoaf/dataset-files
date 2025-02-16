{ "Cd32 = Rs32" , "0110 0010 001sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0, NULL},
{ "Pd4 = Ps4" , "0000ddss000000000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(C2_pxfer_map)}, 
{ "Pd4 = Rs32" , "1000 0101 01-sssss 00------ ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = all8 ( Ps4 )" , "0110 1011 101---ss 00------ ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_CRSLOT23|A_NOTE_CRSLOT23, NULL},
{ "Pd4 = and ( Ps4 , Pt4 )" , "0110 1011 000---ss 00----tt ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_CRSLOT23|A_NOTE_CRSLOT23, NULL},
{ "Pd4 = and ( Pt4 , ! Ps4 )" , "0110 1011 011---ss 00----tt ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_CRSLOT23|A_NOTE_CRSLOT23, NULL},
{ "Pd4 = any8 ( Ps4 )" , "0110 1011 100---ss 00------ ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_CRSLOT23|A_NOTE_CRSLOT23, NULL},
{ "Pd4 = bitsclr ( Rs32 , #u6 )" , "1000 0101 10-sssss 00iiiiii ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Pd4 = bitsclr ( Rs32 , Rt32 )" , "1100 0111 10-sssss 00-ttttt ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Pd4 = bitsset ( Rs32 , Rt32 )" , "1100 0111 01-sssss 00-ttttt ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Pd4 = cmp.eq ( Rs32 , #s10 )" , "0111 0101 00isssss 00iiiiii iii---dd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Pd4 = cmp.eq ( Rs32 , Rt32 )" , "1111 0010 -00sssss 00-ttttt ------dd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Pd4 = cmp.eq ( Rss32 , Rtt32 )" , "1101 -010 1--sssss 00-ttttt 00----dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = cmp.ge ( Rs32 , #s8 )" , "0000ddsssssiiiiiiii0000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(C2_cmpgei)}, 
{ "Pd4 = cmp.geu ( Rs32 , #u8 )" , "0000ddsssssiiiiiiii0000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(C2_cmpgeui)}, 
{ "Pd4 = cmp.gt ( Rs32 , #s10 )" , "0111 0101 01isssss 00iiiiii iii---dd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Pd4 = cmp.gt ( Rs32 , Rt32 )" , "1111 0010 -10sssss 00-ttttt ------dd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Pd4 = cmp.gt ( Rss32 , Rtt32 )" , "1101 -010 1--sssss 00-ttttt 01----dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = cmp.gtu ( Rs32 , #u9 )" , "0111 0101 100sssss 00iiiiii iii---dd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Pd4 = cmp.gtu ( Rs32 , Rt32 )" , "1111 0010 -11sssss 00-ttttt ------dd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Pd4 = cmp.gtu ( Rss32 , Rtt32 )" , "1101 -010 1--sssss 00-ttttt 10----dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = cmp.lt ( Rs32 , Rt32 )" , "0000ddsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(C2_cmplt)}, 
{ "Pd4 = cmp.ltu ( Rs32 , Rt32 )" , "0000ddsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(C2_cmpltu)}, 
{ "Pd4 = not ( Ps4 )" , "0110 1011 110---ss 00------ ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_CRSLOT23|A_NOTE_CRSLOT23, NULL},
{ "Pd4 = or ( Ps4 , Pt4 )" , "0110 1011 001---ss 00----tt ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_CRSLOT23|A_NOTE_CRSLOT23, NULL},
{ "Pd4 = or ( Pt4 , ! Ps4 )" , "0110 1011 111---ss 00----tt ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2|A_CRSLOT23|A_NOTE_CRSLOT23|A_ARCHV2, NULL},
{ "Pd4 = tstbit ( Rs32 , #u5 )" , "1000 0101 00-sssss 000iiiii ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = tstbit ( Rs32 , Rt32 )" , "1100 0111 00-sssss 00-ttttt ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Pd4 = vcmpb.eq ( Rss32 , Rtt32 )" , "1101 -010 0--sssss 00-ttttt 110---dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = vcmpb.gtu ( Rss32 , Rtt32 )" , "1101 -010 0--sssss 00-ttttt 111---dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = vcmph.eq ( Rss32 , Rtt32 )" , "1101 -010 0--sssss 00-ttttt 011---dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = vcmph.gt ( Rss32 , Rtt32 )" , "1101 -010 0--sssss 00-ttttt 100---dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = vcmph.gtu ( Rss32 , Rtt32 )" , "1101 -010 0--sssss 00-ttttt 101---dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = vcmpw.eq ( Rss32 , Rtt32 )" , "1101 -010 0--sssss 00-ttttt 000---dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = vcmpw.gt ( Rss32 , Rtt32 )" , "1101 -010 0--sssss 00-ttttt 001---dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = vcmpw.gtu ( Rss32 , Rtt32 )" , "1101 -010 0--sssss 00-ttttt 010---dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Pd4 = xor ( Ps4 , Pt4 )" , "0110 1011 010---ss 00----tt ------dd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_CRSLOT23|A_NOTE_CRSLOT23, NULL},
{ "Rd32 = #s16" , "0111 1000 ii-iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = + mpyi ( Rs32 , #u8 )" , "111000000--sssss 00-iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = - mpyi ( Rs32 , #u8 )" , "111000001--sssss 00-iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = Cs32" , "0110 1010 000sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0, NULL},
{ "Rd32 = Ps4" , "1000 1001 01- ---ss 00------ --- ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = Rs32" , "0111 0000 011sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = Ss64" , "0110 1110 1-ssssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV, NULL},
{ "Rd32 = abs ( Rs32 ) :sat" , "1000 1100 10-sssss 00------ 1 01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = abs ( Rs32 )" , "1000 1100 10-sssss 00------ 1 00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = add ( Rs32 , #s16 )" , "1011 iiii iiisssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = add ( Rs32 , Rt32 ) :sat" , "1101 -101 100sssss 00-ttttt 0--ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = add ( Rs32 , Rt32 )" , "1111 0011 0-0sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = add ( Rt32.h , Rs32.h ) :<<16" , "1101 -101 010sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = add ( Rt32.h , Rs32.h ) :sat :<<16" , "1101 -101 010sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = add ( Rt32.h , Rs32.l ) :<<16" , "1101 -101 010sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = add ( Rt32.h , Rs32.l ) :sat :<<16" , "1101 -101 010sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = add ( Rt32.l , Rs32.h ) :<<16" , "1101 -101 010sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = add ( Rt32.l , Rs32.h ) :sat :<<16" , "1101 -101 010sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = add ( Rt32.l , Rs32.h ) :sat" , "1101 -101 000sssss 00-ttttt 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = add ( Rt32.l , Rs32.h )" , "1101 -101 000sssss 00-ttttt 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = add ( Rt32.l , Rs32.l ) :<<16" , "1101 -101 010sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = add ( Rt32.l , Rs32.l ) :sat :<<16" , "1101 -101 010sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = add ( Rt32.l , Rs32.l ) :sat" , "1101 -101 000sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = add ( Rt32.l , Rs32.l )" , "1101 -101 000sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = addasl ( Rt32 , Rs32 , #u3 )" , "1100 0100 --- sssss 00-ttttt iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = and ( Rs32 , #s10 )" , "0111 0110 00isssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = and ( Rs32 , Rt32 )" , "1111 0001 -00sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = asl ( Rs32 , #u5 ) :sat" , "1000 1100 01-sssss 000iiiii 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = asl ( Rs32 , #u5 )" , "1000 1100 00-sssss 000iiiii 0 10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = asl ( Rs32 , Rt32 ) :sat" , "1100 0110 00-sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_BIDIRSHIFTL|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = asl ( Rs32 , Rt32 )" , "1100 0110 01-sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rd32 = aslh ( Rs32 )" , "0111 0000 000sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = asr ( Rs32 , #u5 ) :rnd" , "1000 1100 01-sssss 000iiiii 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = asr ( Rs32 , #u5 )" , "1000 1100 00-sssss 000iiiii 0 00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = asr ( Rs32 , Rt32 ) :sat" , "1100 0110 00-sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_BIDIRSHIFTR|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = asr ( Rs32 , Rt32 )" , "1100 0110 01-sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rd32 = asrh ( Rs32 )" , "0111 0000 001sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = asrrnd ( Rs32 , #u5 )" , "0000dddddsssssiiiii0000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_asr_i_r_rnd_goodsyntax)}, 
{ "Rd32 = brev ( Rs32 )" , "1000 1100 01-sssss 00------ 1 10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = cl0 ( Rs32 )" , "1000 1100 00-sssss 00------ 1 01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = cl0 ( Rss32 )" , "1000 1000 01-sssss 00------ 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = cl1 ( Rs32 )" , "1000 1100 00-sssss 00------ 1 10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = cl1 ( Rss32 )" , "1000 1000 01-sssss 00------ 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = clb ( Rs32 )" , "1000 1100 00-sssss 00------ 1 00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = clb ( Rss32 )" , "1000 1000 01-sssss 00------ 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = clrbit ( Rs32 , #u5 )" , "1000 1100 11-sssss 000iiiii 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = clrbit ( Rs32 , Rt32 )" , "1100 0110 10-sssss 00-ttttt 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rd32 = cmpy ( Rs32 , Rt32 ) :<<1 :rnd :sat" , "11101101101sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = cmpy ( Rs32 , Rt32 ) :rnd :sat" , "11101101001sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = cmpy ( Rs32 , Rt32 * ) :<<1 :rnd :sat" , "11101101111sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = cmpy ( Rs32 , Rt32 * ) :rnd :sat" , "11101101011sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = combine ( Rt32.h , Rs32.h )" , "1111 0011 100sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = combine ( Rt32.h , Rs32.l )" , "1111 0011 101sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = combine ( Rt32.l , Rs32.h )" , "1111 0011 110sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = combine ( Rt32.l , Rs32.l )" , "1111 0011 111sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = ct0 ( Rs32 )" , "1000 1100 01-sssss 00------ 1 00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = ct1 ( Rs32 )" , "1000 1100 01-sssss 00------ 1 01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = dctagr ( Rs32 )" , "1010 010 00 01sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1, NULL},
{ "Rd32 = extractu ( Rs32 , #u5 , #U5 )" , "1000 1101 0IIsssss 000iiiii IIIddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = extractu ( Rs32 , Rtt32 )" , "1100 1001 00-sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rd32 = getimask ( Rs32 )" , "0110 0110 000sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_ARCHV3, NULL},
{ "Rd32 = iassignr ( Rs32 )" , "0110 0110 011sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "Rd32 = ictagr ( Rs32 )" , "0101 0101 111sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x4, 0, 0|A_ICOP|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "Rd32 = lsl ( Rs32 , Rt32 )" , "1100 0110 01-sssss 00-ttttt 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rd32 = lsr ( Rs32 , #u5 )" , "1000 1100 00-sssss 000iiiii 0 01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = lsr ( Rs32 , Rt32 )" , "1100 0110 01-sssss 00-ttttt 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rd32 = max ( Rs32 , Rt32 )" , "1101 -101 110sssss 00-ttttt 0--ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = maxu ( Rs32 , Rt32 )" , "1101 -101 110sssss 00-ttttt 1--ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rd32 = memb ( gp + #u16:0 )" , "0100 1ii1 000 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memb ( #g16:0 )" , "0100 1ii1 000 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memb ( Rs32 )" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_loadrb_zomap)}, 
{ "Rd32 = memb ( Rs32 + #s11:0 )" , "1001 0 ii 1 000 sssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memb ( Rx32 ++ #s4:0 )" , "1001 1 01 1 000 xxxxx 0000---i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memb ( Rx32 ++ #s4:0 :circ ( Mu2 ) )" , "1001 1 00 1 000 xxxxx 00u0--0i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memb ( Rx32 ++ Mu2 )" , "1001 1 10 1 000 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memb ( Rx32 ++ Mu2 :brev )" , "1001 1 11 1 000 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memb ( Rx32 ++I :circ ( Mu2 ) )" , "1001 1 00 1 000 xxxxx 00u0--1- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memh ( gp + #u16:1 )" , "0100 1ii1 010 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memh ( #g16:1 )" , "0100 1ii1 010 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memh ( Rs32 )" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_loadrh_zomap)}, 
{ "Rd32 = memh ( Rs32 + #s11:1 )" , "1001 0 ii 1 010 sssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memh ( Rx32 ++ #s4:1 )" , "1001 1 01 1 010 xxxxx 0000---i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memh ( Rx32 ++ #s4:1 :circ ( Mu2 ) )" , "1001 1 00 1 010 xxxxx 00u0--0i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memh ( Rx32 ++ Mu2 )" , "1001 1 10 1 010 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memh ( Rx32 ++ Mu2 :brev )" , "1001 1 11 1 010 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memh ( Rx32 ++I :circ ( Mu2 ) )" , "1001 1 00 1 010 xxxxx 00u0--1- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memub ( gp + #u16:0 )" , "0100 1ii1 001 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memub ( #g16:0 )" , "0100 1ii1 001 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memub ( Rs32 )" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_loadrub_zomap)}, 
{ "Rd32 = memub ( Rs32 + #s11:0 )" , "1001 0 ii 1 001 sssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memub ( Rx32 ++ #s4:0 )" , "1001 1 01 1 001 xxxxx 0000---i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memub ( Rx32 ++ #s4:0 :circ ( Mu2 ) )" , "1001 1 00 1 001 xxxxx 00u0--0i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memub ( Rx32 ++ Mu2 )" , "1001 1 10 1 001 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memub ( Rx32 ++ Mu2 :brev )" , "1001 1 11 1 001 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memub ( Rx32 ++I :circ ( Mu2 ) )" , "1001 1 00 1 001 xxxxx 00u0--1- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_1B|A_LOAD|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memubh ( Rs32 )" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_loadbzw2_zomap)}, 
{ "Rd32 = memubh ( Rs32 + #s11:1 )" , "1001 0 ii 0 011 sssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memubh ( Rx32 ++ #s4:1 )" , "1001 1 01 0 011 xxxxx 0000---i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memubh ( Rx32 ++ #s4:1 :circ ( Mu2 ) )" , "1001 1 00 0 011 xxxxx 00u0--0i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memubh ( Rx32 ++ Mu2 )" , "1001 1 10 0 011 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memubh ( Rx32 ++ Mu2 :brev )" , "1001 1 11 0 011 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memubh ( Rx32 ++I :circ ( Mu2 ) )" , "1001 1 00 0 011 xxxxx 00u0--1- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memuh ( gp + #u16:1 )" , "0100 1ii1 011 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memuh ( #g16:1 )" , "0100 1ii1 011 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memuh ( Rs32 )" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_loadruh_zomap)}, 
{ "Rd32 = memuh ( Rs32 + #s11:1 )" , "1001 0 ii 1 011 sssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memuh ( Rx32 ++ #s4:1 )" , "1001 1 01 1 011 xxxxx 0000---i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memuh ( Rx32 ++ #s4:1 :circ ( Mu2 ) )" , "1001 1 00 1 011 xxxxx 00u0--0i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memuh ( Rx32 ++ Mu2 )" , "1001 1 10 1 011 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memuh ( Rx32 ++ Mu2 :brev )" , "1001 1 11 1 011 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memuh ( Rx32 ++I :circ ( Mu2 ) )" , "1001 1 00 1 011 xxxxx 00u0--1- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_2B|A_LOAD|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memw ( gp + #u16:2 )" , "0100 1ii1 100 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memw ( #g16:2 )" , "0100 1ii1 100 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memw ( Rs32 )" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_loadri_zomap)}, 
{ "Rd32 = memw ( Rs32 + #s11:2 )" , "1001 0 ii 1 100 sssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memw ( Rx32 ++ #s4:2 )" , "1001 1 01 1 100 xxxxx 0000---i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memw ( Rx32 ++ #s4:2 :circ ( Mu2 ) )" , "1001 1 00 1 100 xxxxx 00u0--0i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memw ( Rx32 ++ Mu2 )" , "1001 1 10 1 100 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memw ( Rx32 ++ Mu2 :brev )" , "1001 1 11 1 100 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = memw ( Rx32 ++I :circ ( Mu2 ) )" , "1001 1 00 1 100 xxxxx 00u0--1- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rd32 = memw_locked ( Rs32 )" , "1001 001 0 000 sssss 000----- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_4B|A_LOAD|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1|A_RESTRICT_PACKET_AXOK|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rd32 = min ( Rt32 , Rs32 )" , "1101 -101 101sssss 00-ttttt 0--ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = minu ( Rt32 , Rs32 )" , "1101 -101 101sssss 00-ttttt 1--ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.h ) :<<1 :rnd :sat" , "11101100101sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.h ) :<<1 :rnd" , "11101100101sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.h ) :<<1 :sat" , "11101100100sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.h ) :<<1" , "11101100100sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.h ) :rnd :sat" , "11101100001sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.h ) :rnd" , "11101100001sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.h ) :sat" , "11101100000sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.h )" , "11101100000sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.l ) :<<1 :rnd :sat" , "11101100101sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.l ) :<<1 :rnd" , "11101100101sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.l ) :<<1 :sat" , "11101100100sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.l ) :<<1" , "11101100100sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.l ) :rnd :sat" , "11101100001sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.l ) :rnd" , "11101100001sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.l ) :sat" , "11101100000sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.h , Rt32.l )" , "11101100000sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.h ) :<<1 :rnd :sat" , "11101100101sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.h ) :<<1 :rnd" , "11101100101sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.h ) :<<1 :sat" , "11101100100sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.h ) :<<1" , "11101100100sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.h ) :rnd :sat" , "11101100001sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.h ) :rnd" , "11101100001sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.h ) :sat" , "11101100000sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.h )" , "11101100000sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.l ) :<<1 :rnd :sat" , "11101100101sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.l ) :<<1 :rnd" , "11101100101sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.l ) :<<1 :sat" , "11101100100sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.l ) :<<1" , "11101100100sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.l ) :rnd :sat" , "11101100001sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.l ) :rnd" , "11101100001sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.l ) :sat" , "11101100000sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32.l , Rt32.l )" , "11101100000sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = mpy ( Rs32 , Rt32.h ) :<<1 :rnd :sat" , "11101101101sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32 , Rt32.l ) :<<1 :rnd :sat" , "11101101111sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = mpy ( Rs32 , Rt32 ) :rnd" , "11101101001sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_IT_MPY|A_IT_MPY_32, NULL},
{ "Rd32 = mpy ( Rs32 , Rt32 )" , "11101101000sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_IT_MPY|A_IT_MPY_32, NULL},
{ "Rd32 = mpyi ( Rs32 , #m9 )" , "0000dddddsssssiiiiiiiii000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(M2_mpysmi)}, 
{ "Rd32 = mpyi ( Rs32 , Rt32 )" , "11101101000sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_IT_MPY|A_IT_MPY_32, NULL},
{ "Rd32 = mpyu ( Rs32.h , Rt32.h ) :<<1" , "11101100110sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rd32 = mpyu ( Rs32.h , Rt32.h )" , "11101100010sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rd32 = mpyu ( Rs32.h , Rt32.l ) :<<1" , "11101100110sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rd32 = mpyu ( Rs32.h , Rt32.l )" , "11101100010sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rd32 = mpyu ( Rs32.l , Rt32.h ) :<<1" , "11101100110sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rd32 = mpyu ( Rs32.l , Rt32.h )" , "11101100010sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rd32 = mpyu ( Rs32.l , Rt32.l ) :<<1" , "11101100110sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rd32 = mpyu ( Rs32.l , Rt32.l )" , "11101100010sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rd32 = mpyu ( Rs32 , Rt32 )" , "11101101010sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_IT_MPY|A_IT_MPY_32|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rd32 = mpyui ( Rs32 , Rt32 )" , "0000dddddsssssttttt0000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(M2_mpyui)}, 
{ "Rd32 = mux ( Pu4 , #s8 , #S8 )" , "0111 101u uIIIIIII 00Iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = mux ( Pu4 , #s8 , Rs32 )" , "0111 0011 1uusssss 000iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = mux ( Pu4 , Rs32 , #s8 )" , "0111 0011 0uusssss 000iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = mux ( Pu4 , Rs32 , Rt32 )" , "1111 0100 ---sssss 00-ttttt -uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = neg ( Rs32 ) :sat" , "1000 1100 10-sssss 00------ 1 10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = neg ( Rs32 )" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_neg)}, 
{ "Rd32 = normamt ( Rs32 )" , "1000 1100 00-sssss 00------ 1 11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = not ( Rs32 )" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_not)}, 
{ "Rd32 = or ( Rs32 , #s10 )" , "0111 0110 10isssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = or ( Rs32 , Rt32 )" , "1111 0001 -01sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = parity ( Rss32 , Rtt32 )" , "1101 -000 ---sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = sat ( Rss32 )" , "1000 1000 11-sssss 00------ 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = satb ( Rs32 )" , "1000 1100 11-sssss 00------ 1 11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = sath ( Rs32 )" , "1000 1100 11-sssss 00------ 1 00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = satub ( Rs32 )" , "1000 1100 11-sssss 00------ 1 10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rd32 = satuh ( Rs32 )" , "1000 1100 11-sssss 00------ 1 01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rd32 = setbit ( Rs32 , #u5 )" , "1000 1100 11-sssss 000iiiii 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = setbit ( Rs32 , Rt32 )" , "1100 0110 10-sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rd32 = sub ( #s10 , Rs32 )" , "0111 0110 01isssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = sub ( Rt32.h , Rs32.h ) :<<16" , "1101 -101 011sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = sub ( Rt32.h , Rs32.h ) :sat :<<16" , "1101 -101 011sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = sub ( Rt32.h , Rs32.l ) :<<16" , "1101 -101 011sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = sub ( Rt32.h , Rs32.l ) :sat :<<16" , "1101 -101 011sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = sub ( Rt32.l , Rs32.h ) :<<16" , "1101 -101 011sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = sub ( Rt32.l , Rs32.h ) :sat :<<16" , "1101 -101 011sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = sub ( Rt32.l , Rs32.h ) :sat" , "1101 -101 001sssss 00-ttttt 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = sub ( Rt32.l , Rs32.h )" , "1101 -101 001sssss 00-ttttt 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = sub ( Rt32.l , Rs32.l ) :<<16" , "1101 -101 011sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = sub ( Rt32.l , Rs32.l ) :sat :<<16" , "1101 -101 011sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = sub ( Rt32.l , Rs32.l ) :sat" , "1101 -101 001sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = sub ( Rt32.l , Rs32.l )" , "1101 -101 001sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = sub ( Rt32 , Rs32 ) :sat" , "1101 -101 100sssss 00-ttttt 1--ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = sub ( Rt32 , Rs32 )" , "1111 0011 0-1sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = swiz ( Rs32 )" , "1000 1100 10-sssss 00------ 1 11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = sxtb ( Rs32 )" , "0111 0000 101sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = sxth ( Rs32 )" , "0111 0000 111sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = togglebit ( Rs32 , #u5 )" , "1000 1100 11-sssss 000iiiii 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = togglebit ( Rs32 , Rt32 )" , "1100 0110 10-sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rd32 = vaddh ( Rs32 , Rt32 ) :sat" , "1111 0110 001sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vaddh ( Rs32 , Rt32 )" , "1111 0110 000sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = vadduh ( Rs32 , Rt32 ) :sat" , "1111 0110 011sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rd32 = vasrw ( Rss32 , #u5 )" , "1000 1000 11-sssss 000iiiii 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = vasrw ( Rss32 , Rt32 )" , "1100 0101 ---sssss 00-ttttt 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2|A_BIDIRSHIFTR, NULL},
{ "Rd32 = vavgh ( Rs32 , Rt32 ) :rnd" , "1111 0111 -01sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = vavgh ( Rs32 , Rt32 )" , "1111 0111 -00sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = vdmpy ( Rss32 , Rtt32 ) :<<1 :rnd :sat" , "111010011--sssss 00-ttttt -00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vdmpy ( Rss32 , Rtt32 ) :rnd :sat" , "111010010--sssss 00-ttttt -00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vitpack ( Ps4 , Pt4 )" , "1000 1001 00- ---ss 00----tt --- ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = vmpyh ( Rs32 , Rt32 ) :<<1 :rnd :sat" , "11101101101sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vmpyh ( Rs32 , Rt32 ) :rnd :sat" , "11101101001sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vnavgh ( Rt32 , Rs32 )" , "1111 0111 -11sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = vradduh ( Rss32 , Rtt32 )" , "111010010--sssss 00-ttttt -01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV3, NULL},
{ "Rd32 = vrcmpys ( Rss32 , Rt32 ) :<<1 :rnd :sat" , "0000dddddsssssttttt0000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(M2_vrcmpys_s1rp)}, 
{ "Rd32 = vrcmpys ( Rss32 , Rtt32 ) :<<1 :rnd :sat :raw :hi" , "111010011-1sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV3|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vrcmpys ( Rss32 , Rtt32 ) :<<1 :rnd :sat :raw :lo" , "111010011-1sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV3|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vrndwh ( Rss32 ) :sat" , "1000 1000 10-sssss 00------ 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vrndwh ( Rss32 )" , "1000 1000 10-sssss 00------ 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = vsathb ( Rs32 )" , "1000 1100 10-sssss 00------ 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vsathb ( Rss32 )" , "1000 1000 00-sssss 00------ 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vsathub ( Rs32 )" , "1000 1100 10-sssss 00------ 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rd32 = vsathub ( Rss32 )" , "1000 1000 00-sssss 00------ 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rd32 = vsatwh ( Rss32 )" , "1000 1000 00-sssss 00------ 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vsatwuh ( Rss32 )" , "1000 1000 00-sssss 00------ 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rd32 = vsplatb ( Rs32 )" , "1000 1100 01-sssss 00------ 1 11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = vsubh ( Rt32 , Rs32 ) :sat" , "1111 0110 101sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rd32 = vsubh ( Rt32 , Rs32 )" , "1111 0110 100sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = vsubuh ( Rt32 , Rs32 ) :sat" , "1111 0110 111sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rd32 = vtrunehb ( Rss32 )" , "1000 1000 10-sssss 00------ 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rd32 = vtrunohb ( Rss32 )" , "1000 1000 10-sssss 00------ 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rd32 = xor ( Rs32 , Rt32 )" , "1111 0001 -11sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rd32 = zxtb ( Rs32 )" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_zxtb)}, 
{ "Rd32 = zxth ( Rs32 )" , "0111 0000 110sssss 00------ ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rdd32 = #s8" , "0000dddddiiiiiiii000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrpi)}, 
{ "Rdd32 = Rss32" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrp)}, 
{ "Rdd32 = abs ( Rss32 )" , "1000 0000 10-sssss 00------ 1 10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = add ( Rs32 , Rtt32 )" , "0000dddddsssssttttt0000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_addsp)}, 
{ "Rdd32 = add ( Rss32 , Rtt32 ) :raw :hi" , "1101 -011 011sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV3, NULL},
{ "Rdd32 = add ( Rss32 , Rtt32 ) :raw :lo" , "1101 -011 011sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV3, NULL},
{ "Rdd32 = add ( Rss32 , Rtt32 ) :sat" , "1101 -011 011sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV3|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = add ( Rss32 , Rtt32 )" , "1101 -011 000sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = and ( Rss32 , Rtt32 )" , "1101 -011 111sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = asl ( Rss32 , #u6 )" , "1000 0000 00-sssss 00iiiiii 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = asl ( Rss32 , Rt32 )" , "1100 0011 10-sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rdd32 = asr ( Rss32 , #u6 )" , "1000 0000 00-sssss 00iiiiii 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = asr ( Rss32 , Rt32 )" , "1100 0011 10-sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rdd32 = cmpy ( Rs32 , Rt32 ) :<<1 :sat" , "11100101100sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = cmpy ( Rs32 , Rt32 ) :sat" , "11100101000sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = cmpy ( Rs32 , Rt32 * ) :<<1 :sat" , "11100101110sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = cmpy ( Rs32 , Rt32 * ) :sat" , "11100101010sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = cmpyi ( Rs32 , Rt32 )" , "11100101000sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = cmpyr ( Rs32 , Rt32 )" , "11100101000sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = combine ( #s8 , #S8 )" , "0111 1100 -IIIIIII 00Iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = combine ( Rs32 , Rt32 )" , "1111 0101 ---sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rdd32 = decbin ( Rss32 , Rtt32 )" , "1100 0001 11-sssss 00-ttttt 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_P0, 0|A_ARCHV3|A_NOTE_LATEPRED|A_RESTRICT_LATEPRED|A_IMPLICIT_WRITES_P0, NULL},
{ "Rdd32 = deinterleave ( Rss32 )" , "1000 0000 11-sssss 00------ 1 00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = extractu ( Rss32 , #u6 , #U6 )" , "1000 0001 IIIsssss 00iiiiii IIIddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = extractu ( Rss32 , Rtt32 )" , "1100 0001 00-sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rdd32 = interleave ( Rss32 )" , "1000 0000 11-sssss 00------ 1 01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = lfs ( Rss32 , Rtt32 )" , "1100 0001 10-sssss 00-ttttt 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = lsl ( Rss32 , Rt32 )" , "1100 0011 10-sssss 00-ttttt 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rdd32 = lsr ( Rss32 , #u6 )" , "1000 0000 00-sssss 00iiiiii 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = lsr ( Rss32 , Rt32 )" , "1100 0011 10-sssss 00-ttttt 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rdd32 = mask ( Pt4 )" , "1000 0110 --- ----- 00----tt --- ddddd", HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = max ( Rss32 , Rtt32 )" , "1101 -011 110sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV3, NULL},
{ "Rdd32 = maxu ( Rss32 , Rtt32 )" , "1101 -011 110sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED|A_ARCHV3, NULL},
{ "Rdd32 = memd ( gp + #u16:3 )" , "0100 1ii1 110 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_8B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rdd32 = memd ( #g16:3 )" , "0100 1ii1 110 iiiii 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_8B|A_LOAD|A_ARCHV2|A_IMPLICIT_READS_GP|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rdd32 = memd ( Rs32 )" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_loadrd_zomap)}, 
{ "Rdd32 = memd ( Rs32 + #s11:3 )" , "1001 0 ii 1 110 sssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rdd32 = memd ( Rx32 ++ #s4:3 )" , "1001 1 01 1 110 xxxxx 0000---i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rdd32 = memd ( Rx32 ++ #s4:3 :circ ( Mu2 ) )" , "1001 1 00 1 110 xxxxx 00u0--0i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rdd32 = memd ( Rx32 ++ Mu2 )" , "1001 1 10 1 110 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rdd32 = memd ( Rx32 ++ Mu2 :brev )" , "1001 1 11 1 110 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_8B|A_LOAD|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rdd32 = memd ( Rx32 ++I :circ ( Mu2 ) )" , "1001 1 00 1 110 xxxxx 00u0--1- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_8B|A_LOAD|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rdd32 = memubh ( Rs32 )" , "0000dddddsssss000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_loadbzw4_zomap)}, 
{ "Rdd32 = memubh ( Rs32 + #s11:2 )" , "1001 0 ii 0 101 sssss 00iiiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rdd32 = memubh ( Rx32 ++ #s4:2 )" , "1001 1 01 0 101 xxxxx 0000---i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rdd32 = memubh ( Rx32 ++ #s4:2 :circ ( Mu2 ) )" , "1001 1 00 0 101 xxxxx 00u0--0i iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rdd32 = memubh ( Rx32 ++ Mu2 )" , "1001 1 10 0 101 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rdd32 = memubh ( Rx32 ++ Mu2 :brev )" , "1001 1 11 0 101 xxxxx 00u0---- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "Rdd32 = memubh ( Rx32 ++I :circ ( Mu2 ) )" , "1001 1 00 0 101 xxxxx 00u0--1- ---ddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_MEMSIZE_4B|A_LOAD|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "Rdd32 = min ( Rtt32 , Rss32 )" , "1101 -011 101sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV3, NULL},
{ "Rdd32 = minu ( Rtt32 , Rss32 )" , "1101 -011 101sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED|A_ARCHV3, NULL},
{ "Rdd32 = mpy ( Rs32.h , Rt32.h ) :<<1 :rnd" , "11100100101sssss 00-ttttt -11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.h , Rt32.h ) :<<1" , "11100100100sssss 00-ttttt -11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.h , Rt32.h ) :rnd" , "11100100001sssss 00-ttttt -11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.h , Rt32.h )" , "11100100000sssss 00-ttttt -11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.h , Rt32.l ) :<<1 :rnd" , "11100100101sssss 00-ttttt -10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.h , Rt32.l ) :<<1" , "11100100100sssss 00-ttttt -10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.h , Rt32.l ) :rnd" , "11100100001sssss 00-ttttt -10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.h , Rt32.l )" , "11100100000sssss 00-ttttt -10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.l , Rt32.h ) :<<1 :rnd" , "11100100101sssss 00-ttttt -01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.l , Rt32.h ) :<<1" , "11100100100sssss 00-ttttt -01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.l , Rt32.h ) :rnd" , "11100100001sssss 00-ttttt -01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.l , Rt32.h )" , "11100100000sssss 00-ttttt -01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.l , Rt32.l ) :<<1 :rnd" , "11100100101sssss 00-ttttt -00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.l , Rt32.l ) :<<1" , "11100100100sssss 00-ttttt -00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.l , Rt32.l ) :rnd" , "11100100001sssss 00-ttttt -00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32.l , Rt32.l )" , "11100100000sssss 00-ttttt -00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = mpy ( Rs32 , Rt32 )" , "11100101000sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_IT_MPY|A_IT_MPY_32, NULL},
{ "Rdd32 = mpyu ( Rs32.h , Rt32.h ) :<<1" , "11100100110sssss 00-ttttt -11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rdd32 = mpyu ( Rs32.h , Rt32.h )" , "11100100010sssss 00-ttttt -11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rdd32 = mpyu ( Rs32.h , Rt32.l ) :<<1" , "11100100110sssss 00-ttttt -10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rdd32 = mpyu ( Rs32.h , Rt32.l )" , "11100100010sssss 00-ttttt -10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rdd32 = mpyu ( Rs32.l , Rt32.h ) :<<1" , "11100100110sssss 00-ttttt -01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rdd32 = mpyu ( Rs32.l , Rt32.h )" , "11100100010sssss 00-ttttt -01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rdd32 = mpyu ( Rs32.l , Rt32.l ) :<<1" , "11100100110sssss 00-ttttt -00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rdd32 = mpyu ( Rs32.l , Rt32.l )" , "11100100010sssss 00-ttttt -00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rdd32 = mpyu ( Rs32 , Rt32 )" , "11100101010sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_IT_MPY|A_IT_MPY_32|A_INTRINSIC_RETURNS_UNSIGNED, NULL},
{ "Rdd32 = neg ( Rss32 )" , "1000 0000 10-sssss 00------ 1 01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = not ( Rss32 )" , "1000 0000 10-sssss 00------ 1 00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = or ( Rss32 , Rtt32 )" , "1101 -011 111sssss 00-ttttt 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = packhl ( Rs32 , Rt32 )" , "1101 -100 ---sssss 00-ttttt ---ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = shuffeb ( Rss32 , Rtt32 )" , "1100 0001 00-sssss 00-ttttt 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = shuffeh ( Rss32 , Rtt32 )" , "1100 0001 00-sssss 00-ttttt 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = shuffob ( Rtt32 , Rss32 )" , "1100 0001 00-sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = shuffoh ( Rtt32 , Rss32 )" , "1100 0001 10-sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = sub ( Rtt32 , Rss32 )" , "1101 -011 001sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = sxtw ( Rs32 )" , "1000 0100 01-sssss 00------ 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vabsdiffh ( Rtt32 , Rss32 )" , "11101000011sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = vabsdiffw ( Rtt32 , Rss32 )" , "11101000001sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = vabsh ( Rss32 ) :sat" , "1000 0000 01-sssss 00------ 1 01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vabsh ( Rss32 )" , "1000 0000 01-sssss 00------ 1 00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vabsw ( Rss32 ) :sat" , "1000 0000 01-sssss 00------ 1 11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vabsw ( Rss32 )" , "1000 0000 01-sssss 00------ 1 10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vaddb ( Rss32 , Rtt32 )" , "1101 -011 000sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vaddh ( Rss32 , Rtt32 ) :sat" , "1101 -011 000sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vaddh ( Rss32 , Rtt32 )" , "1101 -011 000sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vaddub ( Rss32 , Rtt32 ) :sat" , "1101 -011 000sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rdd32 = vaddub ( Rss32 , Rtt32 )" , "1101 -011 000sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vadduh ( Rss32 , Rtt32 ) :sat" , "1101 -011 000sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rdd32 = vaddw ( Rss32 , Rtt32 ) :sat" , "1101 -011 000sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vaddw ( Rss32 , Rtt32 )" , "1101 -011 000sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = valignb ( Rtt32 , Rss32 , #u3 )" , "1100 0000 0--sssss 00-ttttt iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = valignb ( Rtt32 , Rss32 , Pu4 )" , "1100 0010 0--sssss 00-ttttt -uuddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vaslh ( Rss32 , #u4 )" , "1000 0000 10-sssss 0000iiii 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vaslh ( Rss32 , Rt32 )" , "1100 0011 01-sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_NOTE_OOBVSHIFT|A_BIDIRSHIFTL, NULL},
{ "Rdd32 = vaslw ( Rss32 , #u5 )" , "1000 0000 01-sssss 000iiiii 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vaslw ( Rss32 , Rt32 )" , "1100 0011 00-sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_NOTE_OOBVSHIFT|A_BIDIRSHIFTL, NULL},
{ "Rdd32 = vasrh ( Rss32 , #u4 )" , "1000 0000 10-sssss 0000iiii 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vasrh ( Rss32 , Rt32 )" , "1100 0011 01-sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_NOTE_OOBVSHIFT|A_BIDIRSHIFTR, NULL},
{ "Rdd32 = vasrw ( Rss32 , #u5 )" , "1000 0000 01-sssss 000iiiii 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vasrw ( Rss32 , Rt32 )" , "1100 0011 00-sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_NOTE_OOBVSHIFT|A_BIDIRSHIFTR, NULL},
{ "Rdd32 = vavgh ( Rss32 , Rtt32 ) :crnd" , "1101 -011 010sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = vavgh ( Rss32 , Rtt32 ) :rnd" , "1101 -011 010sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vavgh ( Rss32 , Rtt32 )" , "1101 -011 010sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vavgub ( Rss32 , Rtt32 ) :rnd" , "1101 -011 010sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vavgub ( Rss32 , Rtt32 )" , "1101 -011 010sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vavguh ( Rss32 , Rtt32 ) :rnd" , "1101 -011 010sssss 00-ttttt 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vavguh ( Rss32 , Rtt32 )" , "1101 -011 010sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vavguw ( Rss32 , Rtt32 ) :rnd" , "1101 -011 011sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vavguw ( Rss32 , Rtt32 )" , "1101 -011 011sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vavgw ( Rss32 , Rtt32 ) :crnd" , "1101 -011 011sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = vavgw ( Rss32 , Rtt32 ) :rnd" , "1101 -011 011sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vavgw ( Rss32 , Rtt32 )" , "1101 -011 011sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vcmpyi ( Rss32 , Rtt32 ) :<<1 :sat" , "11101000110sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vcmpyi ( Rss32 , Rtt32 ) :sat" , "11101000010sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vcmpyr ( Rss32 , Rtt32 ) :<<1 :sat" , "11101000101sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vcmpyr ( Rss32 , Rtt32 ) :sat" , "11101000001sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vconj ( Rss32 ) :sat" , "1000 0000 10-sssss 00------ 1 11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vcrotate ( Rss32 , Rt32 )" , "1100 0011 11-sssss 00-ttttt 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vdmpy ( Rss32 , Rtt32 ) :<<1 :sat" , "11101000100sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vdmpy ( Rss32 , Rtt32 ) :sat" , "11101000000sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vlslh ( Rss32 , Rt32 )" , "1100 0011 01-sssss 00-ttttt 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_NOTE_OOBVSHIFT|A_BIDIRSHIFTL, NULL},
{ "Rdd32 = vlslw ( Rss32 , Rt32 )" , "1100 0011 00-sssss 00-ttttt 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_NOTE_OOBVSHIFT|A_BIDIRSHIFTL, NULL},
{ "Rdd32 = vlsrh ( Rss32 , #u4 )" , "1000 0000 10-sssss 0000iiii 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vlsrh ( Rss32 , Rt32 )" , "1100 0011 01-sssss 00-ttttt 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_NOTE_OOBVSHIFT|A_BIDIRSHIFTR, NULL},
{ "Rdd32 = vlsrw ( Rss32 , #u5 )" , "1000 0000 01-sssss 000iiiii 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vlsrw ( Rss32 , Rt32 )" , "1100 0011 00-sssss 00-ttttt 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_NOTE_OOBVSHIFT|A_BIDIRSHIFTR, NULL},
{ "Rdd32 = vmaxh ( Rss32 , Rtt32 )" , "1101 -011 110sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vmaxub ( Rss32 , Rtt32 )" , "1101 -011 110sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vmaxuh ( Rss32 , Rtt32 )" , "1101 -011 110sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vmaxuw ( Rss32 , Rtt32 )" , "1101 -011 101sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vmaxw ( Rss32 , Rtt32 )" , "1101 -011 110sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vminh ( Rtt32 , Rss32 )" , "1101 -011 101sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vminub ( Rtt32 , Rss32 )" , "1101 -011 101sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vminuh ( Rtt32 , Rss32 )" , "1101 -011 101sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vminuw ( Rtt32 , Rss32 )" , "1101 -011 101sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vminw ( Rtt32 , Rss32 )" , "1101 -011 101sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vmpyeh ( Rss32 , Rtt32 ) :<<1 :sat" , "11101000100sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpyeh ( Rss32 , Rtt32 ) :sat" , "11101000000sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpyh ( Rs32 , Rt32 ) :<<1 :sat" , "11100101100sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpyh ( Rs32 , Rt32 ) :sat" , "11100101000sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpyweh ( Rss32 , Rtt32 ) :<<1 :rnd :sat" , "11101000101sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpyweh ( Rss32 , Rtt32 ) :<<1 :sat" , "11101000100sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpyweh ( Rss32 , Rtt32 ) :rnd :sat" , "11101000001sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpyweh ( Rss32 , Rtt32 ) :sat" , "11101000000sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpyweuh ( Rss32 , Rtt32 ) :<<1 :rnd :sat" , "11101000111sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpyweuh ( Rss32 , Rtt32 ) :<<1 :sat" , "11101000110sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpyweuh ( Rss32 , Rtt32 ) :rnd :sat" , "11101000011sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpyweuh ( Rss32 , Rtt32 ) :sat" , "11101000010sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpywoh ( Rss32 , Rtt32 ) :<<1 :rnd :sat" , "11101000101sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpywoh ( Rss32 , Rtt32 ) :<<1 :sat" , "11101000100sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpywoh ( Rss32 , Rtt32 ) :rnd :sat" , "11101000001sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpywoh ( Rss32 , Rtt32 ) :sat" , "11101000000sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpywouh ( Rss32 , Rtt32 ) :<<1 :rnd :sat" , "11101000111sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpywouh ( Rss32 , Rtt32 ) :<<1 :sat" , "11101000110sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpywouh ( Rss32 , Rtt32 ) :rnd :sat" , "11101000011sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmpywouh ( Rss32 , Rtt32 ) :sat" , "11101000010sssss 00-ttttt 111ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vmux ( Pu4 , Rss32 , Rtt32 )" , "1101 -001 ---sssss 00-ttttt -uuddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vnavgh ( Rtt32 , Rss32 ) :crnd :sat" , "1101 -011 100sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vnavgh ( Rtt32 , Rss32 ) :rnd :sat" , "1101 -011 100sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vnavgh ( Rtt32 , Rss32 )" , "1101 -011 100sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vnavgw ( Rtt32 , Rss32 ) :crnd :sat" , "1101 -011 100sssss 00-ttttt 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vnavgw ( Rtt32 , Rss32 ) :rnd :sat" , "1101 -011 100sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vnavgw ( Rtt32 , Rss32 )" , "1101 -011 100sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = vraddub ( Rss32 , Rtt32 )" , "11101000010sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vrcmpyi ( Rss32 , Rtt32 )" , "11101000000sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vrcmpyi ( Rss32 , Rtt32 * )" , "11101000010sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = vrcmpyr ( Rss32 , Rtt32 )" , "11101000000sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vrcmpyr ( Rss32 , Rtt32 * )" , "11101000011sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = vrcmpys ( Rss32 , Rt32 ) :<<1 :sat" , "0000dddddsssssttttt0000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(M2_vrcmpys_s1)}, 
{ "Rdd32 = vrcmpys ( Rss32 , Rtt32 ) :<<1 :sat :raw :hi" , "11101000101sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV3|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vrcmpys ( Rss32 , Rtt32 ) :<<1 :sat :raw :lo" , "11101000111sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV3|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vrmpyh ( Rss32 , Rtt32 )" , "11101000000sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vrsadub ( Rss32 , Rtt32 )" , "11101000010sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vsathb ( Rss32 )" , "1000 0000 00-sssss 00------ 1 11ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vsathub ( Rss32 )" , "1000 0000 00-sssss 00------ 1 00ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rdd32 = vsatwh ( Rss32 )" , "1000 0000 00-sssss 00------ 1 10ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vsatwuh ( Rss32 )" , "1000 0000 00-sssss 00------ 1 01ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rdd32 = vsplath ( Rs32 )" , "1000 0100 01-sssss 00------ 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vspliceb ( Rss32 , Rtt32 , #u3 )" , "1100 0000 1--sssss 00-ttttt iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vspliceb ( Rss32 , Rtt32 , Pu4 )" , "1100 0010 1--sssss 00-ttttt -uuddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vsubb ( Rtt32 , Rss32 )" , "1101 -011 001sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vsubh ( Rtt32 , Rss32 ) :sat" , "1101 -011 001sssss 00-ttttt 011ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vsubh ( Rtt32 , Rss32 )" , "1101 -011 001sssss 00-ttttt 010ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vsubub ( Rtt32 , Rss32 ) :sat" , "1101 -011 001sssss 00-ttttt 001ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rdd32 = vsubub ( Rtt32 , Rss32 )" , "1101 -011 001sssss 00-ttttt 000ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vsubuh ( Rtt32 , Rss32 ) :sat" , "1101 -011 001sssss 00-ttttt 100ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE|A_USATURATE, NULL},
{ "Rdd32 = vsubw ( Rtt32 , Rss32 ) :sat" , "1101 -011 001sssss 00-ttttt 110ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rdd32 = vsubw ( Rtt32 , Rss32 )" , "1101 -011 001sssss 00-ttttt 101ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vsxtbh ( Rs32 )" , "1000 0100 00-sssss 00------ 00-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = vsxthw ( Rs32 )" , "1000 0100 00-sssss 00------ 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vtrunewh ( Rss32 , Rtt32 )" , "1100 0001 10-sssss 00-ttttt 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = vtrunowh ( Rss32 , Rtt32 )" , "1100 0001 10-sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rdd32 = vzxtbh ( Rs32 )" , "1000 0100 00-sssss 00------ 01-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = vzxthw ( Rs32 )" , "1000 0100 00-sssss 00------ 11-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rdd32 = xor ( Rss32 , Rtt32 )" , "1101 -011 111sssss 00-ttttt 10-ddddd" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32.h = #u16" , "0111 0010 ii1xxxxx 00iiiiii iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rx32.l = #u16" , "0111 0001 ii1xxxxx 00iiiiii iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, NULL},
{ "Rx32 &= asl ( Rs32 , #u5 )" , "1000 1110 01-sssss 000iiiii 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 &= asl ( Rs32 , Rt32 )" , "1100 1100 01-sssss 00-ttttt 10-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rx32 &= asr ( Rs32 , #u5 )" , "1000 1110 01-sssss 000iiiii 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 &= asr ( Rs32 , Rt32 )" , "1100 1100 01-sssss 00-ttttt 00-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rx32 &= lsl ( Rs32 , Rt32 )" , "1100 1100 01-sssss 00-ttttt 11-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rx32 &= lsr ( Rs32 , #u5 )" , "1000 1110 01-sssss 000iiiii 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 &= lsr ( Rs32 , Rt32 )" , "1100 1100 01-sssss 00-ttttt 01-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rx32 += add ( Rs32 , #s8 )" , "111000100--sssss 00-iiiii iiixxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rx32 += add ( Rs32 , Rt32 )" , "111011110--sssss 00-ttttt -01xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rx32 += asl ( Rs32 , #u5 )" , "1000 1110 00-sssss 000iiiii 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += asl ( Rs32 , Rt32 )" , "1100 1100 11-sssss 00-ttttt 10-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rx32 += asr ( Rs32 , #u5 )" , "1000 1110 00-sssss 000iiiii 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += asr ( Rs32 , Rt32 )" , "1100 1100 11-sssss 00-ttttt 00-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rx32 += lsl ( Rs32 , Rt32 )" , "1100 1100 11-sssss 00-ttttt 11-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rx32 += lsr ( Rs32 , #u5 )" , "1000 1110 00-sssss 000iiiii 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += lsr ( Rs32 , Rt32 )" , "1100 1100 11-sssss 00-ttttt 01-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rx32 += mpy ( Rs32.h , Rt32.h ) :<<1 :sat" , "11101110100sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 += mpy ( Rs32.h , Rt32.h ) :<<1" , "11101110100sssss 00-ttttt 011xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpy ( Rs32.h , Rt32.h ) :sat" , "11101110000sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 += mpy ( Rs32.h , Rt32.h )" , "11101110000sssss 00-ttttt 011xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpy ( Rs32.h , Rt32.l ) :<<1 :sat" , "11101110100sssss 00-ttttt 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 += mpy ( Rs32.h , Rt32.l ) :<<1" , "11101110100sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpy ( Rs32.h , Rt32.l ) :sat" , "11101110000sssss 00-ttttt 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 += mpy ( Rs32.h , Rt32.l )" , "11101110000sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpy ( Rs32.l , Rt32.h ) :<<1 :sat" , "11101110100sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 += mpy ( Rs32.l , Rt32.h ) :<<1" , "11101110100sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpy ( Rs32.l , Rt32.h ) :sat" , "11101110000sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 += mpy ( Rs32.l , Rt32.h )" , "11101110000sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpy ( Rs32.l , Rt32.l ) :<<1 :sat" , "11101110100sssss 00-ttttt 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 += mpy ( Rs32.l , Rt32.l ) :<<1" , "11101110100sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpy ( Rs32.l , Rt32.l ) :sat" , "11101110000sssss 00-ttttt 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 += mpy ( Rs32.l , Rt32.l )" , "11101110000sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpyi ( Rs32 , #u8 )" , "111000010--sssss 00-iiiii iiixxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rx32 += mpyi ( Rs32 , Rt32 )" , "111011110--sssss 00-ttttt -00xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_IT_MPY|A_IT_MPY_32|A_ARCHV2, NULL},
{ "Rx32 += mpyu ( Rs32.h , Rt32.h ) :<<1" , "11101110110sssss 00-ttttt 011xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpyu ( Rs32.h , Rt32.h )" , "11101110010sssss 00-ttttt 011xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpyu ( Rs32.h , Rt32.l ) :<<1" , "11101110110sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpyu ( Rs32.h , Rt32.l )" , "11101110010sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpyu ( Rs32.l , Rt32.h ) :<<1" , "11101110110sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpyu ( Rs32.l , Rt32.h )" , "11101110010sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpyu ( Rs32.l , Rt32.l ) :<<1" , "11101110110sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += mpyu ( Rs32.l , Rt32.l )" , "11101110010sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 += sub ( Rt32 , Rs32 )" , "111011110--sssss 00-ttttt -11xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rx32 -= add ( Rs32 , #s8 )" , "111000101--sssss 00-iiiii iiixxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rx32 -= add ( Rs32 , Rt32 )" , "111011111--sssss 00-ttttt -01xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rx32 -= asl ( Rs32 , #u5 )" , "1000 1110 00-sssss 000iiiii 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= asl ( Rs32 , Rt32 )" , "1100 1100 10-sssss 00-ttttt 10-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rx32 -= asr ( Rs32 , #u5 )" , "1000 1110 00-sssss 000iiiii 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= asr ( Rs32 , Rt32 )" , "1100 1100 10-sssss 00-ttttt 00-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rx32 -= lsl ( Rs32 , Rt32 )" , "1100 1100 10-sssss 00-ttttt 11-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rx32 -= lsr ( Rs32 , #u5 )" , "1000 1110 00-sssss 000iiiii 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= lsr ( Rs32 , Rt32 )" , "1100 1100 10-sssss 00-ttttt 01-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rx32 -= mpy ( Rs32.h , Rt32.h ) :<<1 :sat" , "11101110101sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 -= mpy ( Rs32.h , Rt32.h ) :<<1" , "11101110101sssss 00-ttttt 011xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpy ( Rs32.h , Rt32.h ) :sat" , "11101110001sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 -= mpy ( Rs32.h , Rt32.h )" , "11101110001sssss 00-ttttt 011xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpy ( Rs32.h , Rt32.l ) :<<1 :sat" , "11101110101sssss 00-ttttt 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 -= mpy ( Rs32.h , Rt32.l ) :<<1" , "11101110101sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpy ( Rs32.h , Rt32.l ) :sat" , "11101110001sssss 00-ttttt 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 -= mpy ( Rs32.h , Rt32.l )" , "11101110001sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpy ( Rs32.l , Rt32.h ) :<<1 :sat" , "11101110101sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 -= mpy ( Rs32.l , Rt32.h ) :<<1" , "11101110101sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpy ( Rs32.l , Rt32.h ) :sat" , "11101110001sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 -= mpy ( Rs32.l , Rt32.h )" , "11101110001sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpy ( Rs32.l , Rt32.l ) :<<1 :sat" , "11101110101sssss 00-ttttt 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 -= mpy ( Rs32.l , Rt32.l ) :<<1" , "11101110101sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpy ( Rs32.l , Rt32.l ) :sat" , "11101110001sssss 00-ttttt 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rx32 -= mpy ( Rs32.l , Rt32.l )" , "11101110001sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpyi ( Rs32 , #u8 )" , "111000011--sssss 00-iiiii iiixxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rx32 -= mpyu ( Rs32.h , Rt32.h ) :<<1" , "11101110111sssss 00-ttttt 011xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpyu ( Rs32.h , Rt32.h )" , "11101110011sssss 00-ttttt 011xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpyu ( Rs32.h , Rt32.l ) :<<1" , "11101110111sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpyu ( Rs32.h , Rt32.l )" , "11101110011sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpyu ( Rs32.l , Rt32.h ) :<<1" , "11101110111sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpyu ( Rs32.l , Rt32.h )" , "11101110011sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpyu ( Rs32.l , Rt32.l ) :<<1" , "11101110111sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 -= mpyu ( Rs32.l , Rt32.l )" , "11101110011sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 = insert ( Rs32 , #u5 , #U5 )" , "1000 1111 0IIsssss 000iiiii IIIxxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 = insert ( Rs32 , Rtt32 )" , "1100 1000 ---sssss 00-ttttt ---xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 = tableidxb ( Rs32 , #u4 , #S6 ) :raw" , "1000 0111 00isssss 00IIIIII iiixxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2|A_BIDIRSHIFTR, NULL},
{ "Rx32 = tableidxb ( Rs32 , #u4 , #U5 )" , "0000IIIIIxxxxxsssssiiii000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_tableidxb_goodsyntax)}, 
{ "Rx32 = tableidxd ( Rs32 , #u4 , #S6 ) :raw" , "1000 0111 11isssss 00IIIIII iiixxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2|A_BIDIRSHIFTR, NULL},
{ "Rx32 = tableidxd ( Rs32 , #u4 , #U5 )" , "0000IIIIIxxxxxsssssiiii000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_tableidxd_goodsyntax)}, 
{ "Rx32 = tableidxh ( Rs32 , #u4 , #S6 ) :raw" , "1000 0111 01isssss 00IIIIII iiixxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2|A_BIDIRSHIFTR, NULL},
{ "Rx32 = tableidxh ( Rs32 , #u4 , #U5 )" , "0000IIIIIxxxxxsssssiiii000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_tableidxh_goodsyntax)}, 
{ "Rx32 = tableidxw ( Rs32 , #u4 , #S6 ) :raw" , "1000 0111 10isssss 00IIIIII iiixxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2|A_BIDIRSHIFTR, NULL},
{ "Rx32 = tableidxw ( Rs32 , #u4 , #U5 )" , "0000IIIIIxxxxxsssssiiii000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_tableidxw_goodsyntax)}, 
{ "Rx32 ^= asl ( Rs32 , #u5 )" , "1000 1110 10-sssss 000iiiii 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rx32 ^= lsr ( Rs32 , #u5 )" , "1000 1110 10-sssss 000iiiii 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rx32 ^= xor ( Rs32 , Rt32 )" , "111011111--sssss 00-ttttt -11xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rx32 |= asl ( Rs32 , #u5 )" , "1000 1110 01-sssss 000iiiii 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 |= asl ( Rs32 , Rt32 )" , "1100 1100 00-sssss 00-ttttt 10-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rx32 |= asr ( Rs32 , #u5 )" , "1000 1110 01-sssss 000iiiii 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 |= asr ( Rs32 , Rt32 )" , "1100 1100 00-sssss 00-ttttt 00-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rx32 |= lsl ( Rs32 , Rt32 )" , "1100 1100 00-sssss 00-ttttt 11-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rx32 |= lsr ( Rs32 , #u5 )" , "1000 1110 01-sssss 000iiiii 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rx32 |= lsr ( Rs32 , Rt32 )" , "1100 1100 00-sssss 00-ttttt 01-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rxx32 &= asl ( Rss32 , #u6 )" , "1000 0010 01-sssss 00iiiiii 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 &= asl ( Rss32 , Rt32 )" , "1100 1011 01-sssss 00-ttttt 10-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rxx32 &= asr ( Rss32 , #u6 )" , "1000 0010 01-sssss 00iiiiii 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 &= asr ( Rss32 , Rt32 )" , "1100 1011 01-sssss 00-ttttt 00-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rxx32 &= lsl ( Rss32 , Rt32 )" , "1100 1011 01-sssss 00-ttttt 11-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rxx32 &= lsr ( Rss32 , #u6 )" , "1000 0010 01-sssss 00iiiiii 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 &= lsr ( Rss32 , Rt32 )" , "1100 1011 01-sssss 00-ttttt 01-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rxx32 += asl ( Rss32 , #u6 )" , "1000 0010 00-sssss 00iiiiii 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += asl ( Rss32 , Rt32 )" , "1100 1011 11-sssss 00-ttttt 10-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rxx32 += asr ( Rss32 , #u6 )" , "1000 0010 00-sssss 00iiiiii 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += asr ( Rss32 , Rt32 )" , "1100 1011 11-sssss 00-ttttt 00-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rxx32 += cmpy ( Rs32 , Rt32 ) :<<1 :sat" , "11100111100sssss 00-ttttt 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += cmpy ( Rs32 , Rt32 ) :sat" , "11100111000sssss 00-ttttt 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += cmpy ( Rs32 , Rt32 * ) :<<1 :sat" , "11100111110sssss 00-ttttt 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += cmpy ( Rs32 , Rt32 * ) :sat" , "11100111010sssss 00-ttttt 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += cmpyi ( Rs32 , Rt32 )" , "11100111000sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += cmpyr ( Rs32 , Rt32 )" , "11100111000sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += lsl ( Rss32 , Rt32 )" , "1100 1011 11-sssss 00-ttttt 11-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rxx32 += lsr ( Rss32 , #u6 )" , "1000 0010 00-sssss 00iiiiii 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += lsr ( Rss32 , Rt32 )" , "1100 1011 11-sssss 00-ttttt 01-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rxx32 += mpy ( Rs32.h , Rt32.h ) :<<1" , "11100110100sssss 00-ttttt -11xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpy ( Rs32.h , Rt32.h )" , "11100110000sssss 00-ttttt -11xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpy ( Rs32.h , Rt32.l ) :<<1" , "11100110100sssss 00-ttttt -10xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpy ( Rs32.h , Rt32.l )" , "11100110000sssss 00-ttttt -10xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpy ( Rs32.l , Rt32.h ) :<<1" , "11100110100sssss 00-ttttt -01xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpy ( Rs32.l , Rt32.h )" , "11100110000sssss 00-ttttt -01xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpy ( Rs32.l , Rt32.l ) :<<1" , "11100110100sssss 00-ttttt -00xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpy ( Rs32.l , Rt32.l )" , "11100110000sssss 00-ttttt -00xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpy ( Rs32 , Rt32 )" , "11100111000sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_IT_MPY|A_IT_MPY_32, NULL},
{ "Rxx32 += mpyu ( Rs32.h , Rt32.h ) :<<1" , "11100110110sssss 00-ttttt -11xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpyu ( Rs32.h , Rt32.h )" , "11100110010sssss 00-ttttt -11xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpyu ( Rs32.h , Rt32.l ) :<<1" , "11100110110sssss 00-ttttt -10xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpyu ( Rs32.h , Rt32.l )" , "11100110010sssss 00-ttttt -10xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpyu ( Rs32.l , Rt32.h ) :<<1" , "11100110110sssss 00-ttttt -01xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpyu ( Rs32.l , Rt32.h )" , "11100110010sssss 00-ttttt -01xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpyu ( Rs32.l , Rt32.l ) :<<1" , "11100110110sssss 00-ttttt -00xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpyu ( Rs32.l , Rt32.l )" , "11100110010sssss 00-ttttt -00xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += mpyu ( Rs32 , Rt32 )" , "11100111010sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_IT_MPY|A_IT_MPY_32, NULL},
{ "Rxx32 += vcmpyi ( Rss32 , Rtt32 ) :sat" , "11101010010sssss 00-ttttt 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vcmpyr ( Rss32 , Rtt32 ) :sat" , "11101010001sssss 00-ttttt 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vdmpy ( Rss32 , Rtt32 ) :<<1 :sat" , "11101010100sssss 00-ttttt 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vdmpy ( Rss32 , Rtt32 ) :sat" , "11101010000sssss 00-ttttt 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyeh ( Rss32 , Rtt32 ) :<<1 :sat" , "11101010100sssss 00-ttttt 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyeh ( Rss32 , Rtt32 ) :sat" , "11101010000sssss 00-ttttt 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyeh ( Rss32 , Rtt32 )" , "11101010001sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rxx32 += vmpyh ( Rs32 , Rt32 ) :<<1 :sat" , "11100111100sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyh ( Rs32 , Rt32 ) :sat" , "11100111000sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyh ( Rs32 , Rt32 )" , "11100111001sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rxx32 += vmpyweh ( Rss32 , Rtt32 ) :<<1 :rnd :sat" , "11101010101sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyweh ( Rss32 , Rtt32 ) :<<1 :sat" , "11101010100sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyweh ( Rss32 , Rtt32 ) :rnd :sat" , "11101010001sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyweh ( Rss32 , Rtt32 ) :sat" , "11101010000sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyweuh ( Rss32 , Rtt32 ) :<<1 :rnd :sat" , "11101010111sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyweuh ( Rss32 , Rtt32 ) :<<1 :sat" , "11101010110sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyweuh ( Rss32 , Rtt32 ) :rnd :sat" , "11101010011sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpyweuh ( Rss32 , Rtt32 ) :sat" , "11101010010sssss 00-ttttt 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpywoh ( Rss32 , Rtt32 ) :<<1 :rnd :sat" , "11101010101sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpywoh ( Rss32 , Rtt32 ) :<<1 :sat" , "11101010100sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpywoh ( Rss32 , Rtt32 ) :rnd :sat" , "11101010001sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpywoh ( Rss32 , Rtt32 ) :sat" , "11101010000sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpywouh ( Rss32 , Rtt32 ) :<<1 :rnd :sat" , "11101010111sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpywouh ( Rss32 , Rtt32 ) :<<1 :sat" , "11101010110sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpywouh ( Rss32 , Rtt32 ) :rnd :sat" , "11101010011sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vmpywouh ( Rss32 , Rtt32 ) :sat" , "11101010010sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vraddub ( Rss32 , Rtt32 )" , "11101010010sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += vrcmpyi ( Rss32 , Rtt32 )" , "11101010000sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += vrcmpyi ( Rss32 , Rtt32 * )" , "11101010010sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rxx32 += vrcmpyr ( Rss32 , Rtt32 )" , "11101010000sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += vrcmpyr ( Rss32 , Rtt32 * )" , "11101010011sssss 00-ttttt 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rxx32 += vrcmpys ( Rss32 , Rt32 ) :<<1 :sat" , "0000xxxxxsssssttttt0000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(M2_vrcmpys_acc_s1)}, 
{ "Rxx32 += vrcmpys ( Rss32 , Rtt32 ) :<<1 :sat :raw :hi" , "11101010101sssss 00-ttttt 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV3|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vrcmpys ( Rss32 , Rtt32 ) :<<1 :sat :raw :lo" , "11101010111sssss 00-ttttt 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV3|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 += vrmpyh ( Rss32 , Rtt32 )" , "11101010000sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 += vrsadub ( Rss32 , Rtt32 )" , "11101010010sssss 00-ttttt 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= asl ( Rss32 , #u6 )" , "1000 0010 00-sssss 00iiiiii 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= asl ( Rss32 , Rt32 )" , "1100 1011 10-sssss 00-ttttt 10-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rxx32 -= asr ( Rss32 , #u6 )" , "1000 0010 00-sssss 00iiiiii 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= asr ( Rss32 , Rt32 )" , "1100 1011 10-sssss 00-ttttt 00-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rxx32 -= cmpy ( Rs32 , Rt32 ) :<<1 :sat" , "11100111100sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 -= cmpy ( Rs32 , Rt32 ) :sat" , "11100111000sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 -= cmpy ( Rs32 , Rt32 * ) :<<1 :sat" , "11100111110sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 -= cmpy ( Rs32 , Rt32 * ) :sat" , "11100111010sssss 00-ttttt 111xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_SR_OVF, 0|A_ARCHV2|A_SATURATE|A_IMPLICIT_WRITES_SRBIT|A_NOTE_SR_OVF_WHEN_SATURATING|A_RESTRICT_NOSRMOVE, NULL},
{ "Rxx32 -= lsl ( Rss32 , Rt32 )" , "1100 1011 10-sssss 00-ttttt 11-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rxx32 -= lsr ( Rss32 , #u6 )" , "1000 0010 00-sssss 00iiiiii 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= lsr ( Rss32 , Rt32 )" , "1100 1011 10-sssss 00-ttttt 01-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rxx32 -= mpy ( Rs32.h , Rt32.h ) :<<1" , "11100110101sssss 00-ttttt -11xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpy ( Rs32.h , Rt32.h )" , "11100110001sssss 00-ttttt -11xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpy ( Rs32.h , Rt32.l ) :<<1" , "11100110101sssss 00-ttttt -10xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpy ( Rs32.h , Rt32.l )" , "11100110001sssss 00-ttttt -10xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpy ( Rs32.l , Rt32.h ) :<<1" , "11100110101sssss 00-ttttt -01xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpy ( Rs32.l , Rt32.h )" , "11100110001sssss 00-ttttt -01xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpy ( Rs32.l , Rt32.l ) :<<1" , "11100110101sssss 00-ttttt -00xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpy ( Rs32.l , Rt32.l )" , "11100110001sssss 00-ttttt -00xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpy ( Rs32 , Rt32 )" , "11100111001sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_IT_MPY|A_IT_MPY_32, NULL},
{ "Rxx32 -= mpyu ( Rs32.h , Rt32.h ) :<<1" , "11100110111sssss 00-ttttt -11xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpyu ( Rs32.h , Rt32.h )" , "11100110011sssss 00-ttttt -11xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpyu ( Rs32.h , Rt32.l ) :<<1" , "11100110111sssss 00-ttttt -10xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpyu ( Rs32.h , Rt32.l )" , "11100110011sssss 00-ttttt -10xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpyu ( Rs32.l , Rt32.h ) :<<1" , "11100110111sssss 00-ttttt -01xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpyu ( Rs32.l , Rt32.h )" , "11100110011sssss 00-ttttt -01xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpyu ( Rs32.l , Rt32.l ) :<<1" , "11100110111sssss 00-ttttt -00xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpyu ( Rs32.l , Rt32.l )" , "11100110011sssss 00-ttttt -00xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 -= mpyu ( Rs32 , Rt32 )" , "11100111011sssss 00-ttttt 000xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_IT_MPY|A_IT_MPY_32, NULL},
{ "Rxx32 = insert ( Rss32 , #u6 , #U6 )" , "1000 0011 IIIsssss 00iiiiii IIIxxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 = insert ( Rss32 , Rtt32 )" , "1100 1010 ---sssss 00-ttttt ---xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 ^= asl ( Rss32 , #u6 )" , "1000 0010 10-sssss 00iiiiii 010xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rxx32 ^= lsr ( Rss32 , #u6 )" , "1000 0010 10-sssss 00iiiiii 001xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_ARCHV2, NULL},
{ "Rxx32 |= asl ( Rss32 , #u6 )" , "1000 0010 01-sssss 00iiiiii 110xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 |= asl ( Rss32 , Rt32 )" , "1100 1011 00-sssss 00-ttttt 10-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rxx32 |= asr ( Rss32 , #u6 )" , "1000 0010 01-sssss 00iiiiii 100xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 |= asr ( Rss32 , Rt32 )" , "1100 1011 00-sssss 00-ttttt 00-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Rxx32 |= lsl ( Rss32 , Rt32 )" , "1100 1011 00-sssss 00-ttttt 11-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTL, NULL},
{ "Rxx32 |= lsr ( Rss32 , #u6 )" , "1000 0010 01-sssss 00iiiiii 101xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0, NULL},
{ "Rxx32 |= lsr ( Rss32 , Rt32 )" , "1100 1011 00-sssss 00-ttttt 01-xxxxx" , HEXAGON_MACH, NULL, NULL, 0xc, 0, 0|A_BIDIRSHIFTR, NULL},
{ "Sd64 = Rs32" , "0110 0111 00-sssss 00------ --dddddd" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_RESTRICT_PACKET_AXOK|A_RESTRICT_PACKET_SOMEREGS_OK|A_NOTE_SPECIALGROUPING, NULL},
{ "allocframe ( #u11:3 )" , "1010 000 01 0011101 00000iii iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0|IMPLICIT_SP|IMPLICIT_FP, 0|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_READS_SP|A_IMPLICIT_WRITES_SP|A_IMPLICIT_READS_LR|A_IMPLICIT_WRITES_FP|A_IMPLICIT_READS_FP, NULL},
{ "barrier" , "1010 100 00 00----- 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "brkpt" , "0110 1100 001----- 00------ 000-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "call #r22:2" , "0101 101i iiiiiiii 00iiiiii iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC|IMPLICIT_LR, 0|A_CALL|A_DIRECT|A_NOTE_PACKET_PC|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_IMPLICIT_WRITES_PC|A_COF|A_IMPLICIT_WRITES_LR|A_IMPLICIT_READS_PC, NULL},
{ "callr Rs32" , "0101 0000 101sssss 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC|IMPLICIT_LR, 0|A_CALL|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_IMPLICIT_WRITES_PC|A_COF|A_IMPLICIT_WRITES_LR, NULL},
{ "ciad ( Rs32 )" , "0110 0100 000sssss 00------ 011-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_EXCEPTION_SWI, NULL},
{ "crswap ( Rx32 , sgp )" , "0110 0101 000xxxxx 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV, NULL},
{ "cswi ( Rs32 )" , "0110 0100 000sssss 00------ 001-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "dccleana ( Rs32 )" , "1010 000 00 00sssss 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "dccleanidx ( Rs32 )" , "1010 001 00 01sssss 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "dccleaninva ( Rs32 )" , "1010 000 00 10sssss 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "dccleaninvidx ( Rs32 )" , "1010 001 00 11sssss 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "dcfetch ( Rs32 )" , "0000sssss00000000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(Y2_dcfetch)}, 
{ "dcfetch ( Rs32 + #u11:3 )" , "1001 010 0 000 sssss 000--iii iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_RESTRICT_PREFERSLOT0|A_ARCHV3|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "dcinva ( Rs32 )" , "1010 000 00 01sssss 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "dcinvidx ( Rs32 )" , "1010 001 00 10sssss 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "dckill" , "1010 001 00 00----- 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "dctagw ( Rs32 )" , "1010 010 00 00sssss 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1, NULL},
{ "dczeroa ( Rs32 )" , "1010 000 01 10sssss 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_STORE|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "deallocframe" , "1001 000 0 000 11110 000----- ---11110" , HEXAGON_MACH, NULL, NULL, 0x3, 0|IMPLICIT_LR|IMPLICIT_SP|IMPLICIT_FP, 0|A_MEMSIZE_8B|A_LOAD|A_IMPLICIT_WRITES_LR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_WRITES_SP|A_IMPLICIT_WRITES_FP|A_IMPLICIT_READS_FP, NULL},
{ "iassignw ( Rs32 )" , "0110 0100 000sssss 00------ 010-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "icinva ( Rs32 )" , "0101 0110 110sssss 00000--- --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0, 0|A_ICOP|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "icinvidx ( Rs32 )" , "0101 0110 110sssss 00001--- --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0, 0|A_ICOP|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "ickill" , "0101 0110 110----- 00010--- --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0, 0|A_ICOP|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "ictagw ( Rs32 )" , "0101 0110 110sssss 00011--- --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0, 0|A_ICOP|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "if ! Pt4 .new Rd32 = memb ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrbfnew_zomap_alt)}, 
{ "if ! Pt4 .new Rd32 = memb ( Rs32 + #u6:0 )" , "0100 0111 000 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pt4 .new Rd32 = memh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrhfnew_zomap_alt)}, 
{ "if ! Pt4 .new Rd32 = memh ( Rs32 + #u6:1 )" , "0100 0111 010 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pt4 .new Rd32 = memub ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrubfnew_zomap_alt)}, 
{ "if ! Pt4 .new Rd32 = memub ( Rs32 + #u6:0 )" , "0100 0111 001 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pt4 .new Rd32 = memuh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadruhfnew_zomap_alt)}, 
{ "if ! Pt4 .new Rd32 = memuh ( Rs32 + #u6:1 )" , "0100 0111 011 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pt4 .new Rd32 = memw ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrifnew_zomap_alt)}, 
{ "if ! Pt4 .new Rd32 = memw ( Rs32 + #u6:2 )" , "0100 0111 100 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pt4 .new Rdd32 = memd ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrdfnew_zomap_alt)}, 
{ "if ! Pt4 .new Rdd32 = memd ( Rs32 + #u6:3 )" , "0100 0111 110 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pt4 Rd32 = memb ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrbf_zomap_alt)}, 
{ "if ! Pt4 Rd32 = memb ( Rs32 + #u6:0 )" , "0100 0101 000 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pt4 Rd32 = memb ( Rx32 ++ #s4:0 )" , "1001 1 01 1 000 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pt4 Rd32 = memh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrhf_zomap_alt)}, 
{ "if ! Pt4 Rd32 = memh ( Rs32 + #u6:1 )" , "0100 0101 010 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pt4 Rd32 = memh ( Rx32 ++ #s4:1 )" , "1001 1 01 1 010 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pt4 Rd32 = memub ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrubf_zomap_alt)}, 
{ "if ! Pt4 Rd32 = memub ( Rs32 + #u6:0 )" , "0100 0101 001 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pt4 Rd32 = memub ( Rx32 ++ #s4:0 )" , "1001 1 01 1 001 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pt4 Rd32 = memuh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadruhf_zomap_alt)}, 
{ "if ! Pt4 Rd32 = memuh ( Rs32 + #u6:1 )" , "0100 0101 011 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pt4 Rd32 = memuh ( Rx32 ++ #s4:1 )" , "1001 1 01 1 011 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pt4 Rd32 = memw ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrif_zomap_alt)}, 
{ "if ! Pt4 Rd32 = memw ( Rs32 + #u6:2 )" , "0100 0101 100 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pt4 Rd32 = memw ( Rx32 ++ #s4:2 )" , "1001 1 01 1 100 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pt4 Rdd32 = memd ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrdf_zomap_alt)}, 
{ "if ! Pt4 Rdd32 = memd ( Rs32 + #u6:3 )" , "0100 0101 110 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pt4 Rdd32 = memd ( Rx32 ++ #s4:3 )" , "1001 1 01 1 110 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 .new Rd32 = #s12" , "0111 1110 1uu-iiii 001iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 .new Rd32 = Rs32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrfnew_alt)}, 
{ "if ! Pu4 .new Rd32 = add ( Rs32 , #s8 )" , "0111 0100 1uusssss 001iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 .new Rd32 = add ( Rs32 , Rt32 )" , "1111 1011 0-0sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 .new Rd32 = and ( Rs32 , Rt32 )" , "1111 1001 -00sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 .new Rd32 = or ( Rs32 , Rt32 )" , "1111 1001 -01sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 .new Rd32 = sub ( Rt32 , Rs32 )" , "1111 1011 0-1sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 .new Rd32 = xor ( Rs32 , Rt32 )" , "1111 1001 -11sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 .new Rdd32 = Rss32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrpfnew_alt)}, 
{ "if ! Pu4 .new Rdd32 = combine ( Rs32 , Rt32 )" , "1111 1101 ---sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 .new jump:nt #r15:2" , "0101 1100 ii1iiiii 00i01-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_ARCHV2|A_IMPLICIT_READS_PC|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 .new jump:t #r15:2" , "0101 1100 ii1iiiii 00i11-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_ARCHV2|A_IMPLICIT_READS_PC|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 .new jumpr:nt Rs32" , "0101 0011 011sssss 00-01-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_ARCHV3|A_RESTRICT_NOLOOPSETUP|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 .new jumpr:t Rs32" , "0101 0011 011sssss 00-11-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_ARCHV3|A_RESTRICT_NOLOOPSETUP|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ! Pu4 Rd32 = #s12" , "0111 1110 1uu-iiii 000iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 Rd32 = Rs32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrf_alt)}, 
{ "if ! Pu4 Rd32 = add ( Rs32 , #s8 )" , "0111 0100 1uusssss 000iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 Rd32 = add ( Rs32 , Rt32 )" , "1111 1011 0-0sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 Rd32 = and ( Rs32 , Rt32 )" , "1111 1001 -00sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 Rd32 = or ( Rs32 , Rt32 )" , "1111 1001 -01sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 Rd32 = sub ( Rt32 , Rs32 )" , "1111 1011 0-1sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 Rd32 = xor ( Rs32 , Rt32 )" , "1111 1001 -11sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 Rdd32 = Rss32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrpf_alt)}, 
{ "if ! Pu4 Rdd32 = combine ( Rs32 , Rt32 )" , "1111 1101 ---sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 call #r15:2" , "0101 1101 ii1iiiii 00i00-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC|IMPLICIT_LR, 0|A_CALL|A_DIRECT|A_NOTE_PACKET_PC|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|A_IMPLICIT_WRITES_LR|A_IMPLICIT_READS_PC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 callr Rs32" , "0101 0001 001sssss 00----uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC|IMPLICIT_LR, 0|A_CALL|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|A_IMPLICIT_WRITES_LR|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 jump #r15:2" , "0101 1100 ii1iiiii 00i00-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_RELAX_COF_1ST|A_RELAX_COF_2ND|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pu4 jumpr Rs32" , "0101 0011 011sssss 00-00-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pv4 .new memb ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerbfnew_zomap_alt)}, 
{ "if ! Pv4 .new memd ( Rs32 ) = Rtt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerdfnew_zomap_alt)}, 
{ "if ! Pv4 .new memh ( Rs32 ) = Rt32.h" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerffnew_zomap_alt)}, 
{ "if ! Pv4 .new memh ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerhfnew_zomap_alt)}, 
{ "if ! Pv4 .new memw ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerifnew_zomap_alt)}, 
{ "if ! Pv4 memb ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerbf_zomap_alt)}, 
{ "if ! Pv4 memb ( Rs32 + #u6:0 ) = Rt32" , "0100 0100 000 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pv4 memb ( Rx32 ++ #s4:0 ) = Rt32" , "1010 1 01 1 000 xxxxx 001ttttt -iiii1vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pv4 memd ( Rs32 ) = Rtt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerdf_zomap_alt)}, 
{ "if ! Pv4 memd ( Rs32 + #u6:3 ) = Rtt32" , "0100 0100 110 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pv4 memd ( Rx32 ++ #s4:3 ) = Rtt32" , "1010 1 01 1 110 xxxxx 001ttttt -iiii1vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pv4 memh ( Rs32 ) = Rt32.h" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerff_zomap_alt)}, 
{ "if ! Pv4 memh ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerhf_zomap_alt)}, 
{ "if ! Pv4 memh ( Rs32 + #u6:1 ) = Rt32.h" , "0100 0100 011 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pv4 memh ( Rs32 + #u6:1 ) = Rt32" , "0100 0100 010 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pv4 memh ( Rx32 ++ #s4:1 ) = Rt32.h" , "1010 1 01 1 011 xxxxx 001ttttt -iiii1vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pv4 memh ( Rx32 ++ #s4:1 ) = Rt32" , "1010 1 01 1 010 xxxxx 001ttttt -iiii1vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pv4 memw ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerif_zomap_alt)}, 
{ "if ! Pv4 memw ( Rs32 + #u6:2 ) = Rt32" , "0100 0100 100 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ! Pv4 memw ( Rx32 ++ #s4:2 ) = Rt32" , "1010 1 01 1 100 xxxxx 001ttttt -iiii1vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rd32 = memb ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrbf_zomap)}, 
{ "if ( ! Pt4 ) Rd32 = memb ( Rs32 + #u6:0 )" , "0100 0101 000 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rd32 = memb ( Rx32 ++ #s4:0 )" , "1001 1 01 1 000 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rd32 = memh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrhf_zomap)}, 
{ "if ( ! Pt4 ) Rd32 = memh ( Rs32 + #u6:1 )" , "0100 0101 010 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rd32 = memh ( Rx32 ++ #s4:1 )" , "1001 1 01 1 010 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rd32 = memub ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrubf_zomap)}, 
{ "if ( ! Pt4 ) Rd32 = memub ( Rs32 + #u6:0 )" , "0100 0101 001 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rd32 = memub ( Rx32 ++ #s4:0 )" , "1001 1 01 1 001 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rd32 = memuh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadruhf_zomap)}, 
{ "if ( ! Pt4 ) Rd32 = memuh ( Rs32 + #u6:1 )" , "0100 0101 011 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rd32 = memuh ( Rx32 ++ #s4:1 )" , "1001 1 01 1 011 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rd32 = memw ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrif_zomap)}, 
{ "if ( ! Pt4 ) Rd32 = memw ( Rs32 + #u6:2 )" , "0100 0101 100 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rd32 = memw ( Rx32 ++ #s4:2 )" , "1001 1 01 1 100 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rdd32 = memd ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrdf_zomap)}, 
{ "if ( ! Pt4 ) Rdd32 = memd ( Rs32 + #u6:3 )" , "0100 0101 110 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 ) Rdd32 = memd ( Rx32 ++ #s4:3 )" , "1001 1 01 1 110 xxxxx 001-1tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pt4 .new ) Rd32 = memb ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrbfnew_zomap)}, 
{ "if ( ! Pt4 .new ) Rd32 = memb ( Rs32 + #u6:0 )" , "0100 0111 000 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pt4 .new ) Rd32 = memh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrhfnew_zomap)}, 
{ "if ( ! Pt4 .new ) Rd32 = memh ( Rs32 + #u6:1 )" , "0100 0111 010 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pt4 .new ) Rd32 = memub ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrubfnew_zomap)}, 
{ "if ( ! Pt4 .new ) Rd32 = memub ( Rs32 + #u6:0 )" , "0100 0111 001 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pt4 .new ) Rd32 = memuh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadruhfnew_zomap)}, 
{ "if ( ! Pt4 .new ) Rd32 = memuh ( Rs32 + #u6:1 )" , "0100 0111 011 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pt4 .new ) Rd32 = memw ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrifnew_zomap)}, 
{ "if ( ! Pt4 .new ) Rd32 = memw ( Rs32 + #u6:2 )" , "0100 0111 100 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pt4 .new ) Rdd32 = memd ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrdfnew_zomap)}, 
{ "if ( ! Pt4 .new ) Rdd32 = memd ( Rs32 + #u6:3 )" , "0100 0111 110 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 ) Rd32 = #s12" , "0111 1110 1uu-iiii 000iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 ) Rd32 = Rs32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrf)}, 
{ "if ( ! Pu4 ) Rd32 = add ( Rs32 , #s8 )" , "0111 0100 1uusssss 000iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 ) Rd32 = add ( Rs32 , Rt32 )" , "1111 1011 0-0sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 ) Rd32 = and ( Rs32 , Rt32 )" , "1111 1001 -00sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 ) Rd32 = or ( Rs32 , Rt32 )" , "1111 1001 -01sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 ) Rd32 = sub ( Rt32 , Rs32 )" , "1111 1011 0-1sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 ) Rd32 = xor ( Rs32 , Rt32 )" , "1111 1001 -11sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 ) Rdd32 = Rss32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrpf)}, 
{ "if ( ! Pu4 ) Rdd32 = combine ( Rs32 , Rt32 )" , "1111 1101 ---sssss 000ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 ) call #r15:2" , "0101 1101 ii1iiiii 00i00-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC|IMPLICIT_LR, 0|A_CALL|A_DIRECT|A_NOTE_PACKET_PC|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|A_IMPLICIT_WRITES_LR|A_IMPLICIT_READS_PC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 ) callr Rs32" , "0101 0001 001sssss 00----uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC|IMPLICIT_LR, 0|A_CALL|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|A_IMPLICIT_WRITES_LR|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 ) jump #r15:2" , "0101 1100 ii1iiiii 00i00-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_RELAX_COF_1ST|A_RELAX_COF_2ND|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 ) jumpr Rs32" , "0101 0011 011sssss 00-00-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pu4 .new ) Rd32 = #s12" , "0111 1110 1uu-iiii 001iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 .new ) Rd32 = Rs32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrfnew)}, 
{ "if ( ! Pu4 .new ) Rd32 = add ( Rs32 , #s8 )" , "0111 0100 1uusssss 001iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 .new ) Rd32 = add ( Rs32 , Rt32 )" , "1111 1011 0-0sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 .new ) Rd32 = and ( Rs32 , Rt32 )" , "1111 1001 -00sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 .new ) Rd32 = or ( Rs32 , Rt32 )" , "1111 1001 -01sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 .new ) Rd32 = sub ( Rt32 , Rs32 )" , "1111 1011 0-1sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 .new ) Rd32 = xor ( Rs32 , Rt32 )" , "1111 1001 -11sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 .new ) Rdd32 = Rss32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrpfnew)}, 
{ "if ( ! Pu4 .new ) Rdd32 = combine ( Rs32 , Rt32 )" , "1111 1101 ---sssss 001ttttt 1uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 .new ) jump:nt #r15:2" , "0101 1100 ii1iiiii 00i01-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_ARCHV2|A_IMPLICIT_READS_PC|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 .new ) jump:t #r15:2" , "0101 1100 ii1iiiii 00i11-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_ARCHV2|A_IMPLICIT_READS_PC|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 .new ) jumpr:nt Rs32" , "0101 0011 011sssss 00-01-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_ARCHV3|A_RESTRICT_NOLOOPSETUP|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pu4 .new ) jumpr:t Rs32" , "0101 0011 011sssss 00-11-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_ARCHV3|A_RESTRICT_NOLOOPSETUP|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED |CONDITION_DOTNEW, NULL},
{ "if ( ! Pv4 ) memb ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerbf_zomap)}, 
{ "if ( ! Pv4 ) memb ( Rs32 + #u6:0 ) = Rt32" , "0100 0100 000 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pv4 ) memb ( Rx32 ++ #s4:0 ) = Rt32" , "1010 1 01 1 000 xxxxx 001ttttt -iiii1vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pv4 ) memd ( Rs32 ) = Rtt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerdf_zomap)}, 
{ "if ( ! Pv4 ) memd ( Rs32 + #u6:3 ) = Rtt32" , "0100 0100 110 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pv4 ) memd ( Rx32 ++ #s4:3 ) = Rtt32" , "1010 1 01 1 110 xxxxx 001ttttt -iiii1vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pv4 ) memh ( Rs32 ) = Rt32.h" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerff_zomap)}, 
{ "if ( ! Pv4 ) memh ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerhf_zomap)}, 
{ "if ( ! Pv4 ) memh ( Rs32 + #u6:1 ) = Rt32.h" , "0100 0100 011 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pv4 ) memh ( Rs32 + #u6:1 ) = Rt32" , "0100 0100 010 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pv4 ) memh ( Rx32 ++ #s4:1 ) = Rt32.h" , "1010 1 01 1 011 xxxxx 001ttttt -iiii1vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pv4 ) memh ( Rx32 ++ #s4:1 ) = Rt32" , "1010 1 01 1 010 xxxxx 001ttttt -iiii1vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pv4 ) memw ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerif_zomap)}, 
{ "if ( ! Pv4 ) memw ( Rs32 + #u6:2 ) = Rt32" , "0100 0100 100 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pv4 ) memw ( Rx32 ++ #s4:2 ) = Rt32" , "1010 1 01 1 100 xxxxx 001ttttt -iiii1vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_SENSE_INVERTED , NULL},
{ "if ( ! Pv4 .new ) memb ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerbfnew_zomap)}, 
{ "if ( ! Pv4 .new ) memd ( Rs32 ) = Rtt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerdfnew_zomap)}, 
{ "if ( ! Pv4 .new ) memh ( Rs32 ) = Rt32.h" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerffnew_zomap)}, 
{ "if ( ! Pv4 .new ) memh ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerhfnew_zomap)}, 
{ "if ( ! Pv4 .new ) memw ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerifnew_zomap)}, 
{ "if ( Pt4 ) Rd32 = memb ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrbt_zomap)}, 
{ "if ( Pt4 ) Rd32 = memb ( Rs32 + #u6:0 )" , "0100 0001 000 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 ) Rd32 = memb ( Rx32 ++ #s4:0 )" , "1001 1 01 1 000 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 ) Rd32 = memh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrht_zomap)}, 
{ "if ( Pt4 ) Rd32 = memh ( Rs32 + #u6:1 )" , "0100 0001 010 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 ) Rd32 = memh ( Rx32 ++ #s4:1 )" , "1001 1 01 1 010 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 ) Rd32 = memub ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrubt_zomap)}, 
{ "if ( Pt4 ) Rd32 = memub ( Rs32 + #u6:0 )" , "0100 0001 001 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 ) Rd32 = memub ( Rx32 ++ #s4:0 )" , "1001 1 01 1 001 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 ) Rd32 = memuh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadruht_zomap)}, 
{ "if ( Pt4 ) Rd32 = memuh ( Rs32 + #u6:1 )" , "0100 0001 011 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 ) Rd32 = memuh ( Rx32 ++ #s4:1 )" , "1001 1 01 1 011 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 ) Rd32 = memw ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrit_zomap)}, 
{ "if ( Pt4 ) Rd32 = memw ( Rs32 + #u6:2 )" , "0100 0001 100 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 ) Rd32 = memw ( Rx32 ++ #s4:2 )" , "1001 1 01 1 100 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 ) Rdd32 = memd ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrdt_zomap)}, 
{ "if ( Pt4 ) Rdd32 = memd ( Rs32 + #u6:3 )" , "0100 0001 110 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 ) Rdd32 = memd ( Rx32 ++ #s4:3 )" , "1001 1 01 1 110 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pt4 .new ) Rd32 = memb ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrbtnew_zomap)}, 
{ "if ( Pt4 .new ) Rd32 = memb ( Rs32 + #u6:0 )" , "0100 0011 000 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pt4 .new ) Rd32 = memh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrhtnew_zomap)}, 
{ "if ( Pt4 .new ) Rd32 = memh ( Rs32 + #u6:1 )" , "0100 0011 010 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pt4 .new ) Rd32 = memub ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrubtnew_zomap)}, 
{ "if ( Pt4 .new ) Rd32 = memub ( Rs32 + #u6:0 )" , "0100 0011 001 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pt4 .new ) Rd32 = memuh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadruhtnew_zomap)}, 
{ "if ( Pt4 .new ) Rd32 = memuh ( Rs32 + #u6:1 )" , "0100 0011 011 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pt4 .new ) Rd32 = memw ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadritnew_zomap)}, 
{ "if ( Pt4 .new ) Rd32 = memw ( Rs32 + #u6:2 )" , "0100 0011 100 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pt4 .new ) Rdd32 = memd ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrdtnew_zomap)}, 
{ "if ( Pt4 .new ) Rdd32 = memd ( Rs32 + #u6:3 )" , "0100 0011 110 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 ) Rd32 = #s12" , "0111 1110 0uu-iiii 000iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 ) Rd32 = Rs32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrt)}, 
{ "if ( Pu4 ) Rd32 = add ( Rs32 , #s8 )" , "0111 0100 0uusssss 000iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 ) Rd32 = add ( Rs32 , Rt32 )" , "1111 1011 0-0sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 ) Rd32 = and ( Rs32 , Rt32 )" , "1111 1001 -00sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 ) Rd32 = or ( Rs32 , Rt32 )" , "1111 1001 -01sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 ) Rd32 = sub ( Rt32 , Rs32 )" , "1111 1011 0-1sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 ) Rd32 = xor ( Rs32 , Rt32 )" , "1111 1001 -11sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 ) Rdd32 = Rss32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrpt)}, 
{ "if ( Pu4 ) Rdd32 = combine ( Rs32 , Rt32 )" , "1111 1101 ---sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 ) call #r15:2" , "0101 1101 ii0iiiii 00i00-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC|IMPLICIT_LR, 0|A_CALL|A_DIRECT|A_NOTE_PACKET_PC|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|A_IMPLICIT_WRITES_LR|A_IMPLICIT_READS_PC|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 ) callr Rs32" , "0101 0001 000sssss 00----uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC|IMPLICIT_LR, 0|A_CALL|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|A_IMPLICIT_WRITES_LR|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 ) jump #r15:2" , "0101 1100 ii0iiiii 00i00-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_RELAX_COF_1ST|A_RELAX_COF_2ND|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 ) jumpr Rs32" , "0101 0011 010sssss 00-00-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if ( Pu4 .new ) Rd32 = #s12" , "0111 1110 0uu-iiii 001iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 .new ) Rd32 = Rs32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrtnew)}, 
{ "if ( Pu4 .new ) Rd32 = add ( Rs32 , #s8 )" , "0111 0100 0uusssss 001iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 .new ) Rd32 = add ( Rs32 , Rt32 )" , "1111 1011 0-0sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 .new ) Rd32 = and ( Rs32 , Rt32 )" , "1111 1001 -00sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 .new ) Rd32 = or ( Rs32 , Rt32 )" , "1111 1001 -01sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 .new ) Rd32 = sub ( Rt32 , Rs32 )" , "1111 1011 0-1sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 .new ) Rd32 = xor ( Rs32 , Rt32 )" , "1111 1001 -11sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 .new ) Rdd32 = Rss32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrptnew)}, 
{ "if ( Pu4 .new ) Rdd32 = combine ( Rs32 , Rt32 )" , "1111 1101 ---sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 .new ) jump:nt #r15:2" , "0101 1100 ii0iiiii 00i01-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_ARCHV2|A_IMPLICIT_READS_PC|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 .new ) jump:t #r15:2" , "0101 1100 ii0iiiii 00i11-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_ARCHV2|A_IMPLICIT_READS_PC|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 .new ) jumpr:nt Rs32" , "0101 0011 010sssss 00-01-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_ARCHV3|A_RESTRICT_NOLOOPSETUP|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pu4 .new ) jumpr:t Rs32" , "0101 0011 010sssss 00-11-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_ARCHV3|A_RESTRICT_NOLOOPSETUP|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if ( Pv4 ) memb ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerbt_zomap)}, 
{ "if ( Pv4 ) memb ( Rs32 + #u6:0 ) = Rt32" , "0100 0000 000 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pv4 ) memb ( Rx32 ++ #s4:0 ) = Rt32" , "1010 1 01 1 000 xxxxx 001ttttt -iiii0vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pv4 ) memd ( Rs32 ) = Rtt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerdt_zomap)}, 
{ "if ( Pv4 ) memd ( Rs32 + #u6:3 ) = Rtt32" , "0100 0000 110 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pv4 ) memd ( Rx32 ++ #s4:3 ) = Rtt32" , "1010 1 01 1 110 xxxxx 001ttttt -iiii0vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pv4 ) memh ( Rs32 ) = Rt32.h" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerft_zomap)}, 
{ "if ( Pv4 ) memh ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerht_zomap)}, 
{ "if ( Pv4 ) memh ( Rs32 + #u6:1 ) = Rt32.h" , "0100 0000 011 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pv4 ) memh ( Rs32 + #u6:1 ) = Rt32" , "0100 0000 010 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pv4 ) memh ( Rx32 ++ #s4:1 ) = Rt32.h" , "1010 1 01 1 011 xxxxx 001ttttt -iiii0vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pv4 ) memh ( Rx32 ++ #s4:1 ) = Rt32" , "1010 1 01 1 010 xxxxx 001ttttt -iiii0vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pv4 ) memw ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerit_zomap)}, 
{ "if ( Pv4 ) memw ( Rs32 + #u6:2 ) = Rt32" , "0100 0000 100 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pv4 ) memw ( Rx32 ++ #s4:2 ) = Rt32" , "1010 1 01 1 100 xxxxx 001ttttt -iiii0vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if ( Pv4 .new ) memb ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerbtnew_zomap)}, 
{ "if ( Pv4 .new ) memd ( Rs32 ) = Rtt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerdtnew_zomap)}, 
{ "if ( Pv4 .new ) memh ( Rs32 ) = Rt32.h" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerftnew_zomap)}, 
{ "if ( Pv4 .new ) memh ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerhtnew_zomap)}, 
{ "if ( Pv4 .new ) memw ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstoreritnew_zomap)}, 
{ "if ( Rs32 != #0 ) jump:nt #r13:2" , "0110 0001 00isssss 00i0iiii iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_ARCHV3|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if ( Rs32 != #0 ) jump:t #r13:2" , "0110 0001 00isssss 00i1iiii iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_ARCHV3|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if ( Rs32 <= #0 ) jump:nt #r13:2" , "0110 0001 11isssss 00i0iiii iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_ARCHV3|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if ( Rs32 <= #0 ) jump:t #r13:2" , "0110 0001 11isssss 00i1iiii iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_ARCHV3|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if ( Rs32 == #0 ) jump:nt #r13:2" , "0110 0001 10isssss 00i0iiii iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_ARCHV3|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if ( Rs32 == #0 ) jump:t #r13:2" , "0110 0001 10isssss 00i1iiii iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_ARCHV3|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if ( Rs32 >= #0 ) jump:nt #r13:2" , "0110 0001 01isssss 00i0iiii iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_ARCHV3|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if ( Rs32 >= #0 ) jump:t #r13:2" , "0110 0001 01isssss 00i1iiii iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_ARCHV3|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if Pt4 .new Rd32 = memb ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrbtnew_zomap_alt)}, 
{ "if Pt4 .new Rd32 = memb ( Rs32 + #u6:0 )" , "0100 0011 000 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pt4 .new Rd32 = memh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrhtnew_zomap_alt)}, 
{ "if Pt4 .new Rd32 = memh ( Rs32 + #u6:1 )" , "0100 0011 010 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pt4 .new Rd32 = memub ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrubtnew_zomap_alt)}, 
{ "if Pt4 .new Rd32 = memub ( Rs32 + #u6:0 )" , "0100 0011 001 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pt4 .new Rd32 = memuh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadruhtnew_zomap_alt)}, 
{ "if Pt4 .new Rd32 = memuh ( Rs32 + #u6:1 )" , "0100 0011 011 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pt4 .new Rd32 = memw ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadritnew_zomap_alt)}, 
{ "if Pt4 .new Rd32 = memw ( Rs32 + #u6:2 )" , "0100 0011 100 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pt4 .new Rdd32 = memd ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrdtnew_zomap_alt)}, 
{ "if Pt4 .new Rdd32 = memd ( Rs32 + #u6:3 )" , "0100 0011 110 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_DOTNEW|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pt4 Rd32 = memb ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrbt_zomap_alt)}, 
{ "if Pt4 Rd32 = memb ( Rs32 + #u6:0 )" , "0100 0001 000 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pt4 Rd32 = memb ( Rx32 ++ #s4:0 )" , "1001 1 01 1 000 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pt4 Rd32 = memh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrht_zomap_alt)}, 
{ "if Pt4 Rd32 = memh ( Rs32 + #u6:1 )" , "0100 0001 010 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pt4 Rd32 = memh ( Rx32 ++ #s4:1 )" , "1001 1 01 1 010 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pt4 Rd32 = memub ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrubt_zomap_alt)}, 
{ "if Pt4 Rd32 = memub ( Rs32 + #u6:0 )" , "0100 0001 001 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pt4 Rd32 = memub ( Rx32 ++ #s4:0 )" , "1001 1 01 1 001 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pt4 Rd32 = memuh ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadruht_zomap_alt)}, 
{ "if Pt4 Rd32 = memuh ( Rs32 + #u6:1 )" , "0100 0001 011 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pt4 Rd32 = memuh ( Rx32 ++ #s4:1 )" , "1001 1 01 1 011 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pt4 Rd32 = memw ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrit_zomap_alt)}, 
{ "if Pt4 Rd32 = memw ( Rs32 + #u6:2 )" , "0100 0001 100 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pt4 Rd32 = memw ( Rx32 ++ #s4:2 )" , "1001 1 01 1 100 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pt4 Rdd32 = memd ( Rs32 )" , "0000ttdddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(L2_ploadrdt_zomap_alt)}, 
{ "if Pt4 Rdd32 = memd ( Rs32 + #u6:3 )" , "0100 0001 110 sssss 00-ttiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pt4 Rdd32 = memd ( Rx32 ++ #s4:3 )" , "1001 1 01 1 110 xxxxx 001-0tti iiiddddd" , HEXAGON_MACH, NULL, NULL, 0x3, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_LOAD|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pu4 .new Rd32 = #s12" , "0111 1110 0uu-iiii 001iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 .new Rd32 = Rs32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrtnew_alt)}, 
{ "if Pu4 .new Rd32 = add ( Rs32 , #s8 )" , "0111 0100 0uusssss 001iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 .new Rd32 = add ( Rs32 , Rt32 )" , "1111 1011 0-0sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 .new Rd32 = and ( Rs32 , Rt32 )" , "1111 1001 -00sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 .new Rd32 = or ( Rs32 , Rt32 )" , "1111 1001 -01sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 .new Rd32 = sub ( Rt32 , Rs32 )" , "1111 1011 0-1sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 .new Rd32 = xor ( Rs32 , Rt32 )" , "1111 1001 -11sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 .new Rdd32 = Rss32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrptnew_alt)}, 
{ "if Pu4 .new Rdd32 = combine ( Rs32 , Rt32 )" , "1111 1101 ---sssss 001ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_DOTNEW|A_CONDEXEC|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 .new jump:nt #r15:2" , "0101 1100 ii0iiiii 00i01-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_ARCHV2|A_IMPLICIT_READS_PC|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 .new jump:t #r15:2" , "0101 1100 ii0iiiii 00i11-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_ARCHV2|A_IMPLICIT_READS_PC|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 .new jumpr:nt Rs32" , "0101 0011 010sssss 00-01-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_ARCHV3|A_RESTRICT_NOLOOPSETUP|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 .new jumpr:t Rs32" , "0101 0011 010sssss 00-11-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_ARCHV3|A_RESTRICT_NOLOOPSETUP|A_DOTNEW|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC|CONDITION_DOTNEW, NULL},
{ "if Pu4 Rd32 = #s12" , "0111 1110 0uu-iiii 000iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pu4 Rd32 = Rs32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrt_alt)}, 
{ "if Pu4 Rd32 = add ( Rs32 , #s8 )" , "0111 0100 0uusssss 000iiiii iiiddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pu4 Rd32 = add ( Rs32 , Rt32 )" , "1111 1011 0-0sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pu4 Rd32 = and ( Rs32 , Rt32 )" , "1111 1001 -00sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pu4 Rd32 = or ( Rs32 , Rt32 )" , "1111 1001 -01sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pu4 Rd32 = sub ( Rt32 , Rs32 )" , "1111 1011 0-1sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pu4 Rd32 = xor ( Rs32 , Rt32 )" , "1111 1001 -11sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pu4 Rdd32 = Rss32" , "0000uudddddsssss0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(A2_tfrpt_alt)}, 
{ "if Pu4 Rdd32 = combine ( Rs32 , Rt32 )" , "1111 1101 ---sssss 000ttttt 0uuddddd" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_ARCHV2|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pu4 call #r15:2" , "0101 1101 ii0iiiii 00i00-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC|IMPLICIT_LR, 0|A_CALL|A_DIRECT|A_NOTE_PACKET_PC|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|A_IMPLICIT_WRITES_LR|A_IMPLICIT_READS_PC|CONDITIONAL_EXEC, NULL},
{ "if Pu4 callr Rs32" , "0101 0001 000sssss 00----uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC|IMPLICIT_LR, 0|A_CALL|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|A_IMPLICIT_WRITES_LR|CONDITIONAL_EXEC, NULL},
{ "if Pu4 jump #r15:2" , "0101 1100 ii0iiiii 00i00-uu iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_NOTE_CONDITIONAL|A_RELAX_COF_1ST|A_RELAX_COF_2ND|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if Pu4 jumpr Rs32" , "0101 0011 010sssss 00-00-uu --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_NOTE_CONDITIONAL|A_IMPLICIT_WRITES_PC|A_COF|CONDITIONAL_EXEC, NULL},
{ "if Pv4 .new memb ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerbtnew_zomap_alt)}, 
{ "if Pv4 .new memd ( Rs32 ) = Rtt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerdtnew_zomap_alt)}, 
{ "if Pv4 .new memh ( Rs32 ) = Rt32.h" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerftnew_zomap_alt)}, 
{ "if Pv4 .new memh ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstorerhtnew_zomap_alt)}, 
{ "if Pv4 .new memw ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S4_pstoreritnew_zomap_alt)}, 
{ "if Pv4 memb ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerbt_zomap_alt)}, 
{ "if Pv4 memb ( Rs32 + #u6:0 ) = Rt32" , "0100 0000 000 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pv4 memb ( Rx32 ++ #s4:0 ) = Rt32" , "1010 1 01 1 000 xxxxx 001ttttt -iiii0vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pv4 memd ( Rs32 ) = Rtt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerdt_zomap_alt)}, 
{ "if Pv4 memd ( Rs32 + #u6:3 ) = Rtt32" , "0100 0000 110 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pv4 memd ( Rx32 ++ #s4:3 ) = Rtt32" , "1010 1 01 1 110 xxxxx 001ttttt -iiii0vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pv4 memh ( Rs32 ) = Rt32.h" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerft_zomap_alt)}, 
{ "if Pv4 memh ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerht_zomap_alt)}, 
{ "if Pv4 memh ( Rs32 + #u6:1 ) = Rt32.h" , "0100 0000 011 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pv4 memh ( Rs32 + #u6:1 ) = Rt32" , "0100 0000 010 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pv4 memh ( Rx32 ++ #s4:1 ) = Rt32.h" , "1010 1 01 1 011 xxxxx 001ttttt -iiii0vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pv4 memh ( Rx32 ++ #s4:1 ) = Rt32" , "1010 1 01 1 010 xxxxx 001ttttt -iiii0vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pv4 memw ( Rs32 ) = Rt32" , "0000vvsssssttttt0000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_pstorerit_zomap_alt)}, 
{ "if Pv4 memw ( Rs32 + #u6:2 ) = Rt32" , "0100 0000 100 sssss 00ittttt iiiii-vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "if Pv4 memw ( Rx32 ++ #s4:2 ) = Rt32" , "1010 1 01 1 100 xxxxx 001ttttt -iiii0vv" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_ARCHV2|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CONDEXEC|CONDITIONAL_EXEC, NULL},
{ "isync" , "0101 0111 11000000 000---00 00000010" , HEXAGON_MACH, NULL, NULL, 0x4, 0, 0|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "jump #r22:2" , "0101 100i iiiiiiii 00iiiiii iiiiiii-" , HEXAGON_MACH, NULL, NULL, 0xc, 0|IMPLICIT_PC, 0|A_JUMP|A_DIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_BRANCHADDER|A_RELAX_COF_2ND|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_PC|A_COF, NULL},
{ "jumpr Rs32" , "0101 0010 100sssss 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_JUMP|A_INDIRECT|A_RESTRICT_LOOP_LA|A_RESTRICT_COF_MAX1|A_IMPLICIT_WRITES_PC|A_COF, NULL},
{ "k0lock" , "0110 1100 001----- 00------ 011-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_ARCHV3, NULL},
{ "k0unlock" , "0110 1100 001----- 00------ 100-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_ARCHV3, NULL},
{ "l2cleaninvidx ( Rs32 )" , "1010 100 00 11sssss 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1|A_ARCHV3|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "l2kill" , "1010 100 00 01----- 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_ARCHV2, NULL},
{ "loop0 ( #r7:2 , #U10 )" , "0110 1001 000IIIII 00-iiiii IIIii-II" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_LC0|IMPLICIT_SA0|IMPLICIT_SR_OVF, 0|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_NOTE_PACKET_PC|A_NOTE_LA_RESTRICT|A_RESTRICT_LOOP_LA|A_RESTRICT_BRANCHADDER_MAX1|A_NOTE_BRANCHADDER_MAX1|A_BRANCHADDER|A_IT_HWLOOP|A_IMPLICIT_WRITES_LC0|A_IMPLICIT_WRITES_SA0|A_IMPLICIT_WRITES_SRBIT|A_RESTRICT_NOSRMOVE|A_IMPLICIT_READS_PC, NULL},
{ "loop0 ( #r7:2 , Rs32 )" , "0110 0000 000sssss 00-iiiii ---ii---" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_LC0|IMPLICIT_SA0|IMPLICIT_SR_OVF, 0|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_NOTE_PACKET_PC|A_NOTE_LA_RESTRICT|A_RESTRICT_LOOP_LA|A_RESTRICT_BRANCHADDER_MAX1|A_NOTE_BRANCHADDER_MAX1|A_BRANCHADDER|A_IT_HWLOOP|A_IMPLICIT_WRITES_LC0|A_IMPLICIT_WRITES_SA0|A_IMPLICIT_WRITES_SRBIT|A_RESTRICT_NOSRMOVE|A_IMPLICIT_READS_PC, NULL},
{ "loop1 ( #r7:2 , #U10 )" , "0110 1001 001IIIII 00-iiiii IIIii-II" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_LC1|IMPLICIT_SA1, 0|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_NOTE_PACKET_PC|A_NOTE_LA_RESTRICT|A_RESTRICT_LOOP_LA|A_RESTRICT_BRANCHADDER_MAX1|A_NOTE_BRANCHADDER_MAX1|A_BRANCHADDER|A_IT_HWLOOP|A_IMPLICIT_WRITES_LC1|A_IMPLICIT_WRITES_SA1|A_IMPLICIT_READS_PC, NULL},
{ "loop1 ( #r7:2 , Rs32 )" , "0110 0000 001sssss 00-iiiii ---ii---" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_LC1|IMPLICIT_SA1, 0|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_NOTE_PACKET_PC|A_NOTE_LA_RESTRICT|A_RESTRICT_LOOP_LA|A_RESTRICT_BRANCHADDER_MAX1|A_NOTE_BRANCHADDER_MAX1|A_BRANCHADDER|A_IT_HWLOOP|A_IMPLICIT_WRITES_LC1|A_IMPLICIT_WRITES_SA1|A_IMPLICIT_READS_PC, NULL},
{ "memb ( gp + #u16:0 ) = Rt32" , "0100 1ii0 000 iiiii 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_1B|A_STORE|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_READS_GP, NULL},
{ "memb ( #g16:0 ) = Rt32" , "0100 1ii0 000 iiiii 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_1B|A_STORE|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_READS_GP, NULL},
{ "memb ( Rs32 ) = Rt32" , "0000sssssttttt000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_storerb_zomap)}, 
{ "memb ( Rs32 + #s11:0 ) = Rt32" , "1010 0 ii 1 000 sssss 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memb ( Rx32 ++ #s4:0 ) = Rt32" , "1010 1 01 1 000 xxxxx 000ttttt 0iiii---" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memb ( Rx32 ++ #s4:0 :circ ( Mu2 ) ) = Rt32" , "1010 1 00 1 000 xxxxx 00uttttt 0iiii-0-" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "memb ( Rx32 ++ Mu2 ) = Rt32" , "1010 1 10 1 000 xxxxx 00uttttt 0-------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memb ( Rx32 ++ Mu2 :brev ) = Rt32" , "1010 1 11 1 000 xxxxx 00uttttt 0-------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_1B|A_STORE|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memb ( Rx32 ++I :circ ( Mu2 ) ) = Rt32" , "1010 1 00 1 000 xxxxx 00uttttt 0-----1-" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_1B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_ARCHV2|A_CIRCADDR, NULL},
{ "memd ( gp + #u16:3 ) = Rtt32" , "0100 1ii0 110 iiiii 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_8B|A_STORE|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_READS_GP, NULL},
{ "memd ( #g16:3 ) = Rtt32" , "0100 1ii0 110 iiiii 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_8B|A_STORE|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_READS_GP, NULL},
{ "memd ( Rs32 ) = Rtt32" , "0000sssssttttt000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_storerd_zomap)}, 
{ "memd ( Rs32 + #s11:3 ) = Rtt32" , "1010 0 ii 1 110 sssss 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memd ( Rx32 ++ #s4:3 ) = Rtt32" , "1010 1 01 1 110 xxxxx 000ttttt 0iiii---" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memd ( Rx32 ++ #s4:3 :circ ( Mu2 ) ) = Rtt32" , "1010 1 00 1 110 xxxxx 00uttttt 0iiii-0-" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "memd ( Rx32 ++ Mu2 ) = Rtt32" , "1010 1 10 1 110 xxxxx 00uttttt 0-------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memd ( Rx32 ++ Mu2 :brev ) = Rtt32" , "1010 1 11 1 110 xxxxx 00uttttt 0-------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_8B|A_STORE|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memd ( Rx32 ++I :circ ( Mu2 ) ) = Rtt32" , "1010 1 00 1 110 xxxxx 00uttttt 0-----1-" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_8B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_ARCHV2|A_CIRCADDR, NULL},
{ "memh ( gp + #u16:1 ) = Rt32.h" , "0100 1ii0 011 iiiii 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_READS_GP, NULL},
{ "memh ( gp + #u16:1 ) = Rt32" , "0100 1ii0 010 iiiii 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_READS_GP, NULL},
{ "memh ( #g16:1 ) = Rt32.h" , "0100 1ii0 011 iiiii 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_READS_GP, NULL},
{ "memh ( #g16:1 ) = Rt32" , "0100 1ii0 010 iiiii 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_READS_GP, NULL},
{ "memh ( Rs32 ) = Rt32.h" , "0000sssssttttt000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_storerf_zomap)}, 
{ "memh ( Rs32 ) = Rt32" , "0000sssssttttt000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_storerh_zomap)}, 
{ "memh ( Rs32 + #s11:1 ) = Rt32.h" , "1010 0 ii 1 011 sssss 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memh ( Rs32 + #s11:1 ) = Rt32" , "1010 0 ii 1 010 sssss 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memh ( Rx32 ++ #s4:1 ) = Rt32.h" , "1010 1 01 1 011 xxxxx 000ttttt 0iiii---" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memh ( Rx32 ++ #s4:1 ) = Rt32" , "1010 1 01 1 010 xxxxx 000ttttt 0iiii---" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memh ( Rx32 ++ #s4:1 :circ ( Mu2 ) ) = Rt32.h" , "1010 1 00 1 011 xxxxx 00uttttt 0iiii-0-" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "memh ( Rx32 ++ #s4:1 :circ ( Mu2 ) ) = Rt32" , "1010 1 00 1 010 xxxxx 00uttttt 0iiii-0-" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "memh ( Rx32 ++ Mu2 ) = Rt32.h" , "1010 1 10 1 011 xxxxx 00uttttt 0-------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memh ( Rx32 ++ Mu2 ) = Rt32" , "1010 1 10 1 010 xxxxx 00uttttt 0-------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memh ( Rx32 ++ Mu2 :brev ) = Rt32.h" , "1010 1 11 1 011 xxxxx 00uttttt 0-------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memh ( Rx32 ++ Mu2 :brev ) = Rt32" , "1010 1 11 1 010 xxxxx 00uttttt 0-------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memh ( Rx32 ++I :circ ( Mu2 ) ) = Rt32.h" , "1010 1 00 1 011 xxxxx 00uttttt 0-----1-" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_ARCHV2|A_CIRCADDR, NULL},
{ "memh ( Rx32 ++I :circ ( Mu2 ) ) = Rt32" , "1010 1 00 1 010 xxxxx 00uttttt 0-----1-" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_2B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_ARCHV2|A_CIRCADDR, NULL},
{ "memw ( gp + #u16:2 ) = Rt32" , "0100 1ii0 100 iiiii 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_4B|A_STORE|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_READS_GP, NULL},
{ "memw ( #g16:2 ) = Rt32" , "0100 1ii0 100 iiiii 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_4B|A_STORE|A_ARCHV2|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_IMPLICIT_READS_GP, NULL},
{ "memw ( Rs32 ) = Rt32" , "0000sssssttttt000000000000000000" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0, HEXAGON_MAP_FNAME(S2_storeri_zomap)}, 
{ "memw ( Rs32 + #s11:2 ) = Rt32" , "1010 0 ii 1 100 sssss 00ittttt iiiiiiii" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memw ( Rx32 ++ #s4:2 ) = Rt32" , "1010 1 01 1 100 xxxxx 000ttttt 0iiii---" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memw ( Rx32 ++ #s4:2 :circ ( Mu2 ) ) = Rt32" , "1010 1 00 1 100 xxxxx 00uttttt 0iiii-0-" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_CIRCADDR, NULL},
{ "memw ( Rx32 ++ Mu2 ) = Rt32" , "1010 1 10 1 100 xxxxx 00uttttt 0-------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memw ( Rx32 ++ Mu2 :brev ) = Rt32" , "1010 1 11 1 100 xxxxx 00uttttt 0-------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_4B|A_STORE|A_BREVADDR|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "memw ( Rx32 ++I :circ ( Mu2 ) ) = Rt32" , "1010 1 00 1 100 xxxxx 00uttttt 0-----1-" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_4B|A_STORE|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST|A_ARCHV2|A_CIRCADDR, NULL},
{ "memw_locked ( Rs32 , Pd4 ) = Rt32" , "1010 000 01 01sssss 00-ttttt ------dd" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_MEMSIZE_4B|A_STORE|A_NOTE_NOSLOT1|A_RESTRICT_NOSLOT1|A_RESTRICT_PACKET_AXOK|A_RESTRICT_LATEPRED|A_NOTE_LATEPRED|A_MEMLIKE|A_RESTRICT_SINGLE_MEM_FIRST, NULL},
{ "nop" , "0111 1111 -------- 00------ --------" , HEXAGON_MACH, NULL, NULL, 0xf, 0, 0|A_IT_NOP, NULL},
{ "p3 = sp1loop0 ( #r7:2 , #U10 )" , "0110 1001 101IIIII 00-iiiii IIIii-II" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_LC0|IMPLICIT_SA0|IMPLICIT_SR_OVF|IMPLICIT_P3, 0|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_NOTE_PACKET_PC|A_NOTE_LA_RESTRICT|A_RESTRICT_LOOP_LA|A_RESTRICT_BRANCHADDER_MAX1|A_NOTE_BRANCHADDER_MAX1|A_BRANCHADDER|A_IT_HWLOOP|A_ARCHV2|A_RESTRICT_LATEPRED|A_NOTE_LATEPRED|A_IMPLICIT_WRITES_LC0|A_IMPLICIT_WRITES_SA0|A_IMPLICIT_WRITES_SRBIT|A_RESTRICT_NOSRMOVE|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_P3, NULL},
{ "p3 = sp1loop0 ( #r7:2 , Rs32 )" , "0110 0000 101sssss 00-iiiii ---ii---" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_LC0|IMPLICIT_SA0|IMPLICIT_SR_OVF|IMPLICIT_P3, 0|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_NOTE_PACKET_PC|A_NOTE_LA_RESTRICT|A_RESTRICT_LOOP_LA|A_RESTRICT_BRANCHADDER_MAX1|A_NOTE_BRANCHADDER_MAX1|A_BRANCHADDER|A_IT_HWLOOP|A_ARCHV2|A_RESTRICT_LATEPRED|A_NOTE_LATEPRED|A_IMPLICIT_WRITES_LC0|A_IMPLICIT_WRITES_SA0|A_IMPLICIT_WRITES_SRBIT|A_RESTRICT_NOSRMOVE|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_P3, NULL},
{ "p3 = sp2loop0 ( #r7:2 , #U10 )" , "0110 1001 110IIIII 00-iiiii IIIii-II" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_LC0|IMPLICIT_SA0|IMPLICIT_SR_OVF|IMPLICIT_P3, 0|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_NOTE_PACKET_PC|A_NOTE_LA_RESTRICT|A_RESTRICT_LOOP_LA|A_RESTRICT_BRANCHADDER_MAX1|A_NOTE_BRANCHADDER_MAX1|A_BRANCHADDER|A_IT_HWLOOP|A_ARCHV2|A_RESTRICT_LATEPRED|A_NOTE_LATEPRED|A_IMPLICIT_WRITES_LC0|A_IMPLICIT_WRITES_SA0|A_IMPLICIT_WRITES_SRBIT|A_RESTRICT_NOSRMOVE|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_P3, NULL},
{ "p3 = sp2loop0 ( #r7:2 , Rs32 )" , "0110 0000 110sssss 00-iiiii ---ii---" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_LC0|IMPLICIT_SA0|IMPLICIT_SR_OVF|IMPLICIT_P3, 0|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_NOTE_PACKET_PC|A_NOTE_LA_RESTRICT|A_RESTRICT_LOOP_LA|A_RESTRICT_BRANCHADDER_MAX1|A_NOTE_BRANCHADDER_MAX1|A_BRANCHADDER|A_IT_HWLOOP|A_ARCHV2|A_RESTRICT_LATEPRED|A_NOTE_LATEPRED|A_IMPLICIT_WRITES_LC0|A_IMPLICIT_WRITES_SA0|A_IMPLICIT_WRITES_SRBIT|A_RESTRICT_NOSRMOVE|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_P3, NULL},
{ "p3 = sp3loop0 ( #r7:2 , #U10 )" , "0110 1001 111IIIII 00-iiiii IIIii-II" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_LC0|IMPLICIT_SA0|IMPLICIT_SR_OVF|IMPLICIT_P3, 0|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_NOTE_PACKET_PC|A_NOTE_LA_RESTRICT|A_RESTRICT_LOOP_LA|A_RESTRICT_BRANCHADDER_MAX1|A_NOTE_BRANCHADDER_MAX1|A_BRANCHADDER|A_IT_HWLOOP|A_ARCHV2|A_RESTRICT_LATEPRED|A_NOTE_LATEPRED|A_IMPLICIT_WRITES_LC0|A_IMPLICIT_WRITES_SA0|A_IMPLICIT_WRITES_SRBIT|A_RESTRICT_NOSRMOVE|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_P3, NULL},
{ "p3 = sp3loop0 ( #r7:2 , Rs32 )" , "0110 0000 111sssss 00-iiiii ---ii---" , HEXAGON_MACH, NULL, NULL, 0x8, 0|IMPLICIT_LC0|IMPLICIT_SA0|IMPLICIT_SR_OVF|IMPLICIT_P3, 0|A_NOTE_PACKET_NPC|A_NOTE_RELATIVE_ADDRESS|A_NOTE_PACKET_PC|A_NOTE_LA_RESTRICT|A_RESTRICT_LOOP_LA|A_RESTRICT_BRANCHADDER_MAX1|A_NOTE_BRANCHADDER_MAX1|A_BRANCHADDER|A_IT_HWLOOP|A_ARCHV2|A_RESTRICT_LATEPRED|A_NOTE_LATEPRED|A_IMPLICIT_WRITES_LC0|A_IMPLICIT_WRITES_SA0|A_IMPLICIT_WRITES_SRBIT|A_RESTRICT_NOSRMOVE|A_IMPLICIT_READS_PC|A_IMPLICIT_WRITES_P3, NULL},
{ "pause ( #u8 )" , "0101 0100 01------ 00-iiiii ---iii--" , HEXAGON_MACH, NULL, NULL, 0x4, 0, 0|A_COF|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_ARCHV3, NULL},
{ "resume ( Rs32 )" , "0110 0100 010sssss 00------ 001-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "rteunlock" , "0101 0111 111----- 0001---- 000-----" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_PRIV|A_NOTE_PRIV|A_ARCHV3|A_IMPLICIT_WRITES_PC|A_COF, NULL},
{ "rte" , "0101 0111 111----- 0000---- 000-----" , HEXAGON_MACH, NULL, NULL, 0x4, 0|IMPLICIT_PC, 0|A_PRIV|A_NOTE_PRIV|A_IMPLICIT_WRITES_PC|A_COF, NULL},
{ "setimask ( Pt4 , Rs32 )" , "0110 0100 100sssss 00----tt --------" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_ARCHV3, NULL},
{ "start ( Rs32 )" , "0110 0100 011sssss 00------ 001-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "stop ( Rs32 )" , "0110 0100 011sssss 00------ 000-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "swi ( Rs32 )" , "0110 0100 000sssss 00------ 000-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_EXCEPTION_SWI, NULL},
{ "syncht" , "1010 100 00 10----- 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x1, 0, 0|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
{ "tlblock" , "0110 1100 001----- 00------ 001-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_ARCHV3, NULL},
{ "tlbp" , "0110 1100 100----- 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_RESTRICT_PACKET_AXOK, NULL},
{ "tlbr" , "0110 1100 010----- 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_RESTRICT_PACKET_AXOK, NULL},
{ "tlbunlock" , "0110 1100 001----- 00------ 010-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_ARCHV3, NULL},
{ "tlbw" , "0110 1100 000----- 00------ --------" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_RESTRICT_PACKET_AXOK, NULL},
{ "trap0 ( #u8 )" , "0101 0100 00------ 00-iiiii ---iii--" , HEXAGON_MACH, NULL, NULL, 0x4, 0, 0|A_COF|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_EXCEPTION_SWI, NULL},
{ "trap1 ( #u8 )" , "0101 0100 10------ 00-iiiii ---iii--" , HEXAGON_MACH, NULL, NULL, 0x4, 0, 0|A_COF|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET|A_EXCEPTION_SWI, NULL},
{ "wait ( Rs32 )" , "0110 0100 010sssss 00------ 000-----" , HEXAGON_MACH, NULL, NULL, 0x8, 0, 0|A_PRIV|A_NOTE_PRIV|A_NOTE_NOPACKET|A_RESTRICT_NOPACKET, NULL},
