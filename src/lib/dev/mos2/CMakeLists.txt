add_library(mos2 mos2.c mos2acld.c mos2ask.c mos2conv.c mos2del.c mos2dest.c
	mos2dist.c mos2dset.c mos2ic.c mos2load.c mos2mdel.c mos2mpar.c mos2noi.c
	mos2par.c mos2pzld.c mos2sacl.c mos2set.c mos2sld.c mos2sprt.c mos2sset.c
	mos2supd.c mos2temp.c mos2trun.c mos2mask.c)
target_link_libraries(mos2 disto)