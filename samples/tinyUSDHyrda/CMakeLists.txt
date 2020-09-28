set(TARGET_NAME tinyUSDHyrda)
add_executable(${TARGET_NAME}
Source.cpp
)
target_link_libraries(${TARGET_NAME}
PRIVATE
    usd
)
