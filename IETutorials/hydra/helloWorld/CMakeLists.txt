add_executable (helloWorld
main.cpp
SceneDelegate.cpp
)
target_link_libraries(helloWorld
PUBLIC
    usd
)
