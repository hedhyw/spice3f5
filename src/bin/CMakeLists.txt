set(cconfc "${CMAKE_CURRENT_SOURCE_DIR}/cconf.c")
set(bconfc "${CMAKE_CURRENT_SOURCE_DIR}/bconf.c")
set(nconfc "${CMAKE_CURRENT_SOURCE_DIR}/nconf.c")
set(tunec "${CMAKE_CURRENT_SOURCE_DIR}/tunepc.c")

add_executable(cspice main.c ${tunec} ${cconfc})
target_link_libraries(cspice fte dev ckt cp hlp inp misc ni sparse)
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../lib")
target_compile_definitions(cspice PRIVATE SIMULATOR=1 BATCH=1 SPICE2=1)
install(TARGETS cspice RUNTIME DESTINATION "C:\\spice3f5")

add_executable(bspice main.c ${tunec} ${bconfc})
target_link_libraries(bspice fte dev ckt cp hlp inp misc ni sparse)
target_compile_definitions(bspice PRIVATE SIMULATOR=1 BATCH=1)
install(TARGETS bspice RUNTIME DESTINATION "C:\\spice3f5")

add_executable(nutmeg main.c ${tunec} ${nconfc})
target_link_libraries(nutmeg fte cp hlp misc)
install(TARGETS nutmeg RUNTIME DESTINATION "C:\\spice3f5")