







MAP_FUNCTION(A2_addsp)
{
if (GET_OP_VAL(1) & 1) {
snprintf(i, n,"R%d:%d=add(R%d:%d,R%d:%d):raw:hi",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1) |1,GET_OP_VAL(1) & -2,GET_OP_VAL(2)+1,GET_OP_VAL(2));
} else {
snprintf(i, n,"R%d:%d=add(R%d:%d,R%d:%d):raw:lo",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1) |1,GET_OP_VAL(1) & -2,GET_OP_VAL(2)+1,GET_OP_VAL(2));
}
}







MAP_FUNCTION(A2_neg)
{
snprintf(i, n,"R%d=sub(#0,R%d)",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(A2_not)
{
snprintf(i, n,"R%d=sub(#-1,R%d)",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(A2_tfrf)
{
snprintf(i, n,"if (!P%d) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrf_alt)
{
snprintf(i, n,"if (!P%d) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrfnew)
{
snprintf(i, n,"if (!P%d.new) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrfnew_alt)
{
snprintf(i, n,"if (!P%d.new) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrp)
{
snprintf(i, n,"R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1));
}







MAP_FUNCTION(A2_tfrpf)
{
snprintf(i, n,"if (!P%d) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrpf_alt)
{
snprintf(i, n,"if (!P%d) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrpfnew)
{
snprintf(i, n,"if (!P%d.new) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrpfnew_alt)
{
snprintf(i, n,"if (!P%d.new) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}









MAP_FUNCTION(A2_tfrpi)
{
if (GET_OP_VAL(1)<0) {
snprintf(i, n,"R%d:%d=combine(#-1,#%s)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_STR(1));
} else {
snprintf(i, n,"R%d:%d=combine(#0,#%s)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_STR(1));
}
}







MAP_FUNCTION(A2_tfrpt)
{
snprintf(i, n,"if (P%d) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrpt_alt)
{
snprintf(i, n,"if (P%d) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrptnew)
{
snprintf(i, n,"if (P%d.new) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrptnew_alt)
{
snprintf(i, n,"if (P%d.new) R%d:%d=combine(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrt)
{
snprintf(i, n,"if (P%d) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrt_alt)
{
snprintf(i, n,"if (P%d) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrtnew)
{
snprintf(i, n,"if (P%d.new) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(A2_tfrtnew_alt)
{
snprintf(i, n,"if (P%d.new) R%d=add(R%d,#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(A2_vaddb_map)
{
snprintf(i, n,"R%d:%d=vaddub(R%d:%d,R%d:%d)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(A2_vsubb_map)
{
snprintf(i, n,"R%d:%d=vsubub(R%d:%d,R%d:%d)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(A2_zxtb)
{
snprintf(i, n,"R%d=and(R%d,#255)",GET_OP_VAL(0),GET_OP_VAL(1));
}









MAP_FUNCTION(A4_boundscheck)
{
if (GET_OP_VAL(1) & 1) {
snprintf(i, n,"P%d=boundscheck(R%d:%d,R%d:%d):raw:hi",GET_OP_VAL(0),GET_OP_VAL(1) |1,GET_OP_VAL(1) & -2,GET_OP_VAL(2)+1,GET_OP_VAL(2));
} else {
snprintf(i, n,"P%d=boundscheck(R%d:%d,R%d:%d):raw:lo",GET_OP_VAL(0),GET_OP_VAL(1) |1,GET_OP_VAL(1) & -2,GET_OP_VAL(2)+1,GET_OP_VAL(2));
}
}







MAP_FUNCTION(C2_cmpgei)
{
snprintf(i, n,"P%d=cmp.gt(R%d,#%s-1)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}









MAP_FUNCTION(C2_cmpgeui)
{
if (GET_OP_VAL(2)==0) {
snprintf(i, n,"P%d=cmp.eq(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(1));
} else {
snprintf(i, n,"P%d=cmp.gtu(R%d,#%s-1)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}
}







MAP_FUNCTION(C2_cmplt)
{
snprintf(i, n,"P%d=cmp.gt(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(2),GET_OP_VAL(1));
}







MAP_FUNCTION(C2_cmpltu)
{
snprintf(i, n,"P%d=cmp.gtu(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(2),GET_OP_VAL(1));
}







MAP_FUNCTION(C2_pxfer_map)
{
snprintf(i, n,"P%d=or(P%d,P%d)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadalignb_zomap)
{
snprintf(i, n,"R%d:%d=memb_fifo(R%d+#0)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadalignh_zomap)
{
snprintf(i, n,"R%d:%d=memh_fifo(R%d+#0)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadbsw2_zomap)
{
snprintf(i, n,"R%d=membh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadbsw4_zomap)
{
snprintf(i, n,"R%d:%d=membh(R%d+#0)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadbzw2_zomap)
{
snprintf(i, n,"R%d=memubh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadbzw4_zomap)
{
snprintf(i, n,"R%d:%d=memubh(R%d+#0)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadrb_zomap)
{
snprintf(i, n,"R%d=memb(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadrd_zomap)
{
snprintf(i, n,"R%d:%d=memd(R%d+#0)",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadrh_zomap)
{
snprintf(i, n,"R%d=memh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadri_zomap)
{
snprintf(i, n,"R%d=memw(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadrub_zomap)
{
snprintf(i, n,"R%d=memub(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_loadruh_zomap)
{
snprintf(i, n,"R%d=memuh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L2_ploadrbf_zomap)
{
snprintf(i, n,"if (!P%d) R%d=memb(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrbf_zomap_alt)
{
snprintf(i, n,"if (!P%d) R%d=memb(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrbfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) R%d=memb(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrbfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) R%d=memb(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrbt_zomap)
{
snprintf(i, n,"if (P%d) R%d=memb(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrbt_zomap_alt)
{
snprintf(i, n,"if (P%d) R%d=memb(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrbtnew_zomap)
{
snprintf(i, n,"if (P%d.new) R%d=memb(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrbtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) R%d=memb(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrdf_zomap)
{
snprintf(i, n,"if (!P%d) R%d:%d=memd(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrdf_zomap_alt)
{
snprintf(i, n,"if (!P%d) R%d:%d=memd(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrdfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) R%d:%d=memd(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrdfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) R%d:%d=memd(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrdt_zomap)
{
snprintf(i, n,"if (P%d) R%d:%d=memd(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrdt_zomap_alt)
{
snprintf(i, n,"if (P%d) R%d:%d=memd(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrdtnew_zomap)
{
snprintf(i, n,"if (P%d.new) R%d:%d=memd(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrdtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) R%d:%d=memd(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrhf_zomap)
{
snprintf(i, n,"if (!P%d) R%d=memh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrhf_zomap_alt)
{
snprintf(i, n,"if (!P%d) R%d=memh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrhfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) R%d=memh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrhfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) R%d=memh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrht_zomap)
{
snprintf(i, n,"if (P%d) R%d=memh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrht_zomap_alt)
{
snprintf(i, n,"if (P%d) R%d=memh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrhtnew_zomap)
{
snprintf(i, n,"if (P%d.new) R%d=memh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrhtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) R%d=memh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrif_zomap)
{
snprintf(i, n,"if (!P%d) R%d=memw(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrif_zomap_alt)
{
snprintf(i, n,"if (!P%d) R%d=memw(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrifnew_zomap)
{
snprintf(i, n,"if (!P%d.new) R%d=memw(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrifnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) R%d=memw(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrit_zomap)
{
snprintf(i, n,"if (P%d) R%d=memw(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrit_zomap_alt)
{
snprintf(i, n,"if (P%d) R%d=memw(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadritnew_zomap)
{
snprintf(i, n,"if (P%d.new) R%d=memw(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadritnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) R%d=memw(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrubf_zomap)
{
snprintf(i, n,"if (!P%d) R%d=memub(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrubf_zomap_alt)
{
snprintf(i, n,"if (!P%d) R%d=memub(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrubfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) R%d=memub(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrubfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) R%d=memub(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrubt_zomap)
{
snprintf(i, n,"if (P%d) R%d=memub(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrubt_zomap_alt)
{
snprintf(i, n,"if (P%d) R%d=memub(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrubtnew_zomap)
{
snprintf(i, n,"if (P%d.new) R%d=memub(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadrubtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) R%d=memub(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadruhf_zomap)
{
snprintf(i, n,"if (!P%d) R%d=memuh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadruhf_zomap_alt)
{
snprintf(i, n,"if (!P%d) R%d=memuh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadruhfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) R%d=memuh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadruhfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) R%d=memuh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadruht_zomap)
{
snprintf(i, n,"if (P%d) R%d=memuh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadruht_zomap_alt)
{
snprintf(i, n,"if (P%d) R%d=memuh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadruhtnew_zomap)
{
snprintf(i, n,"if (P%d.new) R%d=memuh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L2_ploadruhtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) R%d=memuh(R%d+#0)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(L4_add_memopb_zomap)
{
snprintf(i, n,"memb(R%d+#0)+=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L4_add_memoph_zomap)
{
snprintf(i, n,"memh(R%d+#0)+=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L4_add_memopw_zomap)
{
snprintf(i, n,"memw(R%d+#0)+=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L4_and_memopb_zomap)
{
snprintf(i, n,"memb(R%d+#0)&=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L4_and_memoph_zomap)
{
snprintf(i, n,"memh(R%d+#0)&=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L4_and_memopw_zomap)
{
snprintf(i, n,"memw(R%d+#0)&=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L4_iadd_memopb_zomap)
{
snprintf(i, n,"memb(R%d+#0)+=#%s",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_iadd_memoph_zomap)
{
snprintf(i, n,"memh(R%d+#0)+=#%s",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_iadd_memopw_zomap)
{
snprintf(i, n,"memw(R%d+#0)+=#%s",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_iand_memopb_zomap)
{
snprintf(i, n,"memb(R%d+#0)=clrbit(#%s)",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_iand_memoph_zomap)
{
snprintf(i, n,"memh(R%d+#0)=clrbit(#%s)",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_iand_memopw_zomap)
{
snprintf(i, n,"memw(R%d+#0)=clrbit(#%s)",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_ior_memopb_zomap)
{
snprintf(i, n,"memb(R%d+#0)=setbit(#%s)",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_ior_memoph_zomap)
{
snprintf(i, n,"memh(R%d+#0)=setbit(#%s)",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_ior_memopw_zomap)
{
snprintf(i, n,"memw(R%d+#0)=setbit(#%s)",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_isub_memopb_zomap)
{
snprintf(i, n,"memb(R%d+#0)-=#%s",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_isub_memoph_zomap)
{
snprintf(i, n,"memh(R%d+#0)-=#%s",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_isub_memopw_zomap)
{
snprintf(i, n,"memw(R%d+#0)-=#%s",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(L4_or_memopb_zomap)
{
snprintf(i, n,"memb(R%d+#0)|=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L4_or_memoph_zomap)
{
snprintf(i, n,"memh(R%d+#0)|=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L4_or_memopw_zomap)
{
snprintf(i, n,"memw(R%d+#0)|=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L4_sub_memopb_zomap)
{
snprintf(i, n,"memb(R%d+#0)-=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L4_sub_memoph_zomap)
{
snprintf(i, n,"memh(R%d+#0)-=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(L4_sub_memopw_zomap)
{
snprintf(i, n,"memw(R%d+#0)-=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}









MAP_FUNCTION(M2_mpysmi)
{
if (((GET_OP_VAL(2)<0) && (GET_OP_VAL(2)>-256))) {
snprintf(i, n,"R%d=-mpyi(R%d,#%s*(-1))",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
} else {
snprintf(i, n,"R%d=+mpyi(R%d,#%s)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}
}







MAP_FUNCTION(M2_mpyui)
{
snprintf(i, n,"R%d=mpyi(R%d,R%d)",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}









MAP_FUNCTION(M2_vrcmpys_acc_s1)
{
if (GET_OP_VAL(2) & 1) {
snprintf(i, n,"R%d:%d+=vrcmpys(R%d:%d,R%d:%d):<<1:sat:raw:hi",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
} else {
snprintf(i, n,"R%d:%d+=vrcmpys(R%d:%d,R%d:%d):<<1:sat:raw:lo",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
}
}









MAP_FUNCTION(M2_vrcmpys_s1)
{
if (GET_OP_VAL(2) & 1) {
snprintf(i, n,"R%d:%d=vrcmpys(R%d:%d,R%d:%d):<<1:sat:raw:hi",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
} else {
snprintf(i, n,"R%d:%d=vrcmpys(R%d:%d,R%d:%d):<<1:sat:raw:lo",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
}
}









MAP_FUNCTION(M2_vrcmpys_s1rp)
{
if (GET_OP_VAL(2) & 1) {
snprintf(i, n,"R%d=vrcmpys(R%d:%d,R%d:%d):<<1:rnd:sat:raw:hi",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
} else {
snprintf(i, n,"R%d=vrcmpys(R%d:%d,R%d:%d):<<1:rnd:sat:raw:lo",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_VAL(2) |1,GET_OP_VAL(2) & -2);
}
}









MAP_FUNCTION(S2_asr_i_p_rnd_goodsyntax)
{
if (GET_OP_VAL(2)==0) {
snprintf(i, n,"R%d:%d=R%d:%d",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1));
} else {
snprintf(i, n,"R%d:%d=asr(R%d:%d,#u5-1):rnd",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1));
}
}









MAP_FUNCTION(S2_asr_i_r_rnd_goodsyntax)
{
if (GET_OP_VAL(2)==0) {
snprintf(i, n,"R%d=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
} else {
snprintf(i, n,"R%d=asr(R%d,#%s-1):rnd",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}
}







MAP_FUNCTION(S2_pstorerbf_zomap)
{
snprintf(i, n,"if (!P%d) memb(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerbf_zomap_alt)
{
snprintf(i, n,"if (!P%d) memb(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerbnewf_zomap)
{
snprintf(i, n,"if (!P%d) memb(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerbnewf_zomap_alt)
{
snprintf(i, n,"if (!P%d) memb(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerbnewt_zomap)
{
snprintf(i, n,"if (P%d) memb(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerbnewt_zomap_alt)
{
snprintf(i, n,"if (P%d) memb(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerbt_zomap)
{
snprintf(i, n,"if (P%d) memb(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerbt_zomap_alt)
{
snprintf(i, n,"if (P%d) memb(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerdf_zomap)
{
snprintf(i, n,"if (!P%d) memd(R%d+#0)=R%d:%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerdf_zomap_alt)
{
snprintf(i, n,"if (!P%d) memd(R%d+#0)=R%d:%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerdt_zomap)
{
snprintf(i, n,"if (P%d) memd(R%d+#0)=R%d:%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerdt_zomap_alt)
{
snprintf(i, n,"if (P%d) memd(R%d+#0)=R%d:%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerff_zomap)
{
snprintf(i, n,"if (!P%d) memh(R%d+#0)=R%d.h",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerff_zomap_alt)
{
snprintf(i, n,"if (!P%d) memh(R%d+#0)=R%d.h",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerft_zomap)
{
snprintf(i, n,"if (P%d) memh(R%d+#0)=R%d.h",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerft_zomap_alt)
{
snprintf(i, n,"if (P%d) memh(R%d+#0)=R%d.h",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerhf_zomap)
{
snprintf(i, n,"if (!P%d) memh(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerhf_zomap_alt)
{
snprintf(i, n,"if (!P%d) memh(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerhnewf_zomap)
{
snprintf(i, n,"if (!P%d) memh(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerhnewf_zomap_alt)
{
snprintf(i, n,"if (!P%d) memh(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerhnewt_zomap)
{
snprintf(i, n,"if (P%d) memh(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerhnewt_zomap_alt)
{
snprintf(i, n,"if (P%d) memh(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerht_zomap)
{
snprintf(i, n,"if (P%d) memh(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerht_zomap_alt)
{
snprintf(i, n,"if (P%d) memh(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerif_zomap)
{
snprintf(i, n,"if (!P%d) memw(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerif_zomap_alt)
{
snprintf(i, n,"if (!P%d) memw(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerinewf_zomap)
{
snprintf(i, n,"if (!P%d) memw(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerinewf_zomap_alt)
{
snprintf(i, n,"if (!P%d) memw(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerinewt_zomap)
{
snprintf(i, n,"if (P%d) memw(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerinewt_zomap_alt)
{
snprintf(i, n,"if (P%d) memw(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerit_zomap)
{
snprintf(i, n,"if (P%d) memw(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_pstorerit_zomap_alt)
{
snprintf(i, n,"if (P%d) memw(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S2_storerb_zomap)
{
snprintf(i, n,"memb(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(S2_storerbnew_zomap)
{
snprintf(i, n,"memb(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(S2_storerd_zomap)
{
snprintf(i, n,"memd(R%d+#0)=R%d:%d",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1));
}







MAP_FUNCTION(S2_storerf_zomap)
{
snprintf(i, n,"memh(R%d+#0)=R%d.h",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(S2_storerh_zomap)
{
snprintf(i, n,"memh(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(S2_storerhnew_zomap)
{
snprintf(i, n,"memh(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(S2_storeri_zomap)
{
snprintf(i, n,"memw(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(S2_storerinew_zomap)
{
snprintf(i, n,"memw(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1));
}







MAP_FUNCTION(S2_tableidxb_goodsyntax)
{
snprintf(i, n,"R%d=tableidxb(R%d,#%s,#%s):raw",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2),GET_OP_STR(3));
}







MAP_FUNCTION(S2_tableidxd_goodsyntax)
{
snprintf(i, n,"R%d=tableidxd(R%d,#%s,#%s-3):raw",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2),GET_OP_STR(3));
}







MAP_FUNCTION(S2_tableidxh_goodsyntax)
{
snprintf(i, n,"R%d=tableidxh(R%d,#%s,#%s-1):raw",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2),GET_OP_STR(3));
}







MAP_FUNCTION(S2_tableidxw_goodsyntax)
{
snprintf(i, n,"R%d=tableidxw(R%d,#%s,#%s-2):raw",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2),GET_OP_STR(3));
}







MAP_FUNCTION(S4_pstorerbfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) memb(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerbfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) memb(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerbnewfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) memb(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerbnewfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) memb(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerbnewtnew_zomap)
{
snprintf(i, n,"if (P%d.new) memb(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerbnewtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) memb(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerbtnew_zomap)
{
snprintf(i, n,"if (P%d.new) memb(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerbtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) memb(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerdfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) memd(R%d+#0)=R%d:%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerdfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) memd(R%d+#0)=R%d:%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerdtnew_zomap)
{
snprintf(i, n,"if (P%d.new) memd(R%d+#0)=R%d:%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerdtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) memd(R%d+#0)=R%d:%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2)+1,GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerffnew_zomap)
{
snprintf(i, n,"if (!P%d.new) memh(R%d+#0)=R%d.h",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerffnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) memh(R%d+#0)=R%d.h",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerftnew_zomap)
{
snprintf(i, n,"if (P%d.new) memh(R%d+#0)=R%d.h",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerftnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) memh(R%d+#0)=R%d.h",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerhfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) memh(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerhfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) memh(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerhnewfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) memh(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerhnewfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) memh(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerhnewtnew_zomap)
{
snprintf(i, n,"if (P%d.new) memh(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerhnewtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) memh(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerhtnew_zomap)
{
snprintf(i, n,"if (P%d.new) memh(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerhtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) memh(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerifnew_zomap)
{
snprintf(i, n,"if (!P%d.new) memw(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerifnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) memw(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerinewfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) memw(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerinewfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) memw(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerinewtnew_zomap)
{
snprintf(i, n,"if (P%d.new) memw(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstorerinewtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) memw(R%d+#0)=R%d.new",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstoreritnew_zomap)
{
snprintf(i, n,"if (P%d.new) memw(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_pstoreritnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) memw(R%d+#0)=R%d",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_VAL(2));
}







MAP_FUNCTION(S4_storeirb_zomap)
{
snprintf(i, n,"memb(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(S4_storeirbf_zomap)
{
snprintf(i, n,"if (!P%d) memb(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirbf_zomap_alt)
{
snprintf(i, n,"if (!P%d) memb(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirbfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) memb(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirbfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) memb(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirbt_zomap)
{
snprintf(i, n,"if (P%d) memb(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirbt_zomap_alt)
{
snprintf(i, n,"if (P%d) memb(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirbtnew_zomap)
{
snprintf(i, n,"if (P%d.new) memb(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirbtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) memb(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirh_zomap)
{
snprintf(i, n,"memh(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(S4_storeirhf_zomap)
{
snprintf(i, n,"if (!P%d) memh(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirhf_zomap_alt)
{
snprintf(i, n,"if (!P%d) memh(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirhfnew_zomap)
{
snprintf(i, n,"if (!P%d.new) memh(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirhfnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) memh(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirht_zomap)
{
snprintf(i, n,"if (P%d) memh(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirht_zomap_alt)
{
snprintf(i, n,"if (P%d) memh(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirhtnew_zomap)
{
snprintf(i, n,"if (P%d.new) memh(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirhtnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) memh(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeiri_zomap)
{
snprintf(i, n,"memw(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_STR(1));
}







MAP_FUNCTION(S4_storeirif_zomap)
{
snprintf(i, n,"if (!P%d) memw(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirif_zomap_alt)
{
snprintf(i, n,"if (!P%d) memw(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirifnew_zomap)
{
snprintf(i, n,"if (!P%d.new) memw(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirifnew_zomap_alt)
{
snprintf(i, n,"if (!P%d.new) memw(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirit_zomap)
{
snprintf(i, n,"if (P%d) memw(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeirit_zomap_alt)
{
snprintf(i, n,"if (P%d) memw(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeiritnew_zomap)
{
snprintf(i, n,"if (P%d.new) memw(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}







MAP_FUNCTION(S4_storeiritnew_zomap_alt)
{
snprintf(i, n,"if (P%d.new) memw(R%d+#0)=#%s",GET_OP_VAL(0),GET_OP_VAL(1),GET_OP_STR(2));
}









MAP_FUNCTION(S5_asrhub_rnd_sat_goodsyntax)
{
if (GET_OP_VAL(2)==0) {
snprintf(i, n,"R%d=vsathub(R%d:%d)",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1));
} else {
snprintf(i, n,"R%d=vasrhub(R%d:%d,#%s-1):raw",GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_STR(2));
}
}









MAP_FUNCTION(S5_vasrhrnd_goodsyntax)
{
if (GET_OP_VAL(2)==0) {
snprintf(i, n,"R%d:%d=R%d:%d",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1));
} else {
snprintf(i, n,"R%d:%d=vasrh(R%d:%d,#%s-1):raw",GET_OP_VAL(0)+1,GET_OP_VAL(0),GET_OP_VAL(1)+1,GET_OP_VAL(1),GET_OP_STR(2));
}
}







MAP_FUNCTION(Y2_crswap_old)
{
snprintf(i, n,"crswap(R%d,sgp0)",GET_OP_VAL(0));
}







MAP_FUNCTION(Y2_dcfetch)
{
snprintf(i, n,"dcfetch(R%d+#0)",GET_OP_VAL(0));
}

