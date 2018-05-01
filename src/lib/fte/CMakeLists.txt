add_library(fte agraf.c arg.c aspice.c breakp.c circuits.c clip.c cmath1.c
  cmath2.c cmath3.c cmath4.c compose.c cpitf.c debugcom.c
  define.c device.c diff.c display.c doplot.c dotcards.c
  error.c evaluate.c fourier.c graf.c graphdb.c grid.c inp.c
  inpcom.c interp.c linear.c mfb.c misccoms.c miscvars.c
  nutctab.c nutinp.c nutmegif.c options.c outitf.c
  parse.c plot5.c plotcurv.c points.c postcoms.c postsc.c
  rawfile.c resource.c runcoms.c shyu.c signal.c spcmdtab.c
  spiceif.c subckt.c types.c vectors.c where.c x10.c x11.c
  gens.c newcoms.c dimens.c xgraph.c runcoms2.c breakp2.c)
target_link_libraries(fte inp m X11 Xt Xaw)