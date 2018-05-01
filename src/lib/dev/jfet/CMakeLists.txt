add_library(jfet jfet.c jfetacld.c jfetask.c jfetdel.c jfetdest.c jfetdist.c
  jfetdset.c jfetic.c jfetload.c jfetmask.c jfetmdel.c jfetmpar.c jfetnoi.c
  jfetpar.c jfetpzld.c jfetset.c jfettemp.c jfettrun.c ../devsup.c)
target_link_libraries(jfet ckt ni)