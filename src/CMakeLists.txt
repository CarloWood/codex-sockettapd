add_executable(sockettapd
  sockettapd.cxx
)

target_link_libraries(sockettapd
  PRIVATE
    ${AICXX_OBJECTS_LIST}
    enchantum::enchantum
)
