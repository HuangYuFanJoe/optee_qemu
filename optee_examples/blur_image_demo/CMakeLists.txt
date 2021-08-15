project (blur_image_demo C)

set (MAIN host/main.c)
set (SRC1 host/encode.c)
set (SRC2 host/decode.c)

add_executable (${PROJECT_NAME} ${MAIN} ${SRC1} ${SRC2})

target_include_directories(${PROJECT_NAME}
			   PRIVATE ta/include
			   PRIVATE include
			   PRIVATE host)

target_link_libraries (${PROJECT_NAME} PRIVATE teec -lm)

install (TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
