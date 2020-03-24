#-------------------------------------------------------------------------------
# SUBLIBRARY configuration
#-------------------------------------------------------------------------------
target_sources(${PROJECT_NAME} PRIVATE
  cluto.c
  dimacs.c
  #metis.c
  #mm.c
  #snap.c
  #ugraph.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
