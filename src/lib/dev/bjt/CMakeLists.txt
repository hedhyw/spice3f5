add_library(bjt bjt.c bjtacld.c bjtask.c bjtconv.c bjtdel.c bjtdest.c
  bjtdisto.c bjtdset.c bjtgetic.c bjtload.c bjtmask.c
  bjtmdel.c bjtmpar.c bjtnoise.c bjtparam.c bjtpzld.c
  bjtsacl.c bjtsetup.c bjtsload.c bjtsprt.c bjtsset.c
  bjtsupd.c bjttemp.c bjttrunc.c)
target_link_libraries(bjt disto)