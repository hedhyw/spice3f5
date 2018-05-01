set(confc "${CMAKE_CURRENT_SOURCE_DIR}/cconf.c")
set(tunec "${CMAKE_CURRENT_SOURCE_DIR}/tunepc.c")
set(devices "asrc bjt bsim1 bsim2 cap cccs csw dio disto ind isrc jfet ltra mes mos1 mos2 mos3 mos6 res sw tra urc vccs vcvs vsrc")
set(analyses "op dc tf ac tran pz disto noise sense")
set(version "3f5")

add_definitions(-DSIMULATOR)
add_executable(spice3 main.c ${tunec} ${confc})
target_link_libraries(spice3 fte dev ckt cp hlp inp mfb ni misc sparse)
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../lib")
install(TARGETS spice3 RUNTIME DESTINATION /usr/local/bin)

add_custom_command(
  OUTPUT ${confc}
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../util/mkvers.sh" "${devices}" "${analyses}" "${version}" > ${confc}
  COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/config.c >> ${confc}
)
add_executable(maxkeidx makeidx.c)
target_link_libraries (maxkeidx misc hlp)