add_library(mes mes.c mesacl.c mesask.c mesdel.c mesdest.c mesdisto.c mesdset.c
  mesgetic.c mesload.c mesmask.c mesmdel.c mesmpar.c mesnoise.c mesparam.c
  mespzld.c messetup.c mestemp.c mestrunc.c)
target_link_libraries(mes disto)