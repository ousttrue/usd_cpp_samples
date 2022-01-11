add_executable (simpleObject
        main.cpp
        SceneDelegate.cpp SceneDelegate.h tiny_obj_loader.h)
target_link_libraries(
        simpleObject
        usd
        )
