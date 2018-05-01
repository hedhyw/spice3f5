add_library(urc urc.c urcask.c urcdel.c urcdest.c urcmask.c urcmdel.c urcmpar.c
	urcparam.c urcsetup.c)
target_link_libraries(urc ckt)