add_subdirectory(01-qml-syntax-basics-beginner)
set_target_properties(QmlSyntaxDemo PROPERTIES OUTPUT_NAME "06-01-qml-syntax-basics")

add_subdirectory(02-property-binding-beginner)
set_target_properties(PropertyBindingDemo PROPERTIES OUTPUT_NAME "06-02-property-binding")

add_subdirectory(03-qtquick-controls-beginner)
set_target_properties(QuickControlsDemo PROPERTIES OUTPUT_NAME "06-03-qtquick-controls")

add_subdirectory(04-cpp-qml-interop-beginner)
set_target_properties(CppQmlInteropDemo PROPERTIES OUTPUT_NAME "06-04-cpp-qml-interop")

add_subdirectory(05-qml-animation-states-beginner)
set_target_properties(QmlAnimationDemo PROPERTIES OUTPUT_NAME "06-05-qml-animation-states")

add_subdirectory(06-qml-model-delegate-beginner)
set_target_properties(QmlModelDelegateDemo PROPERTIES OUTPUT_NAME "06-06-qml-model-delegate")

add_subdirectory(07-qml-canvas-particles-beginner)
set_target_properties(QmlCanvasParticlesDemo PROPERTIES OUTPUT_NAME "06-07-qml-canvas-particles")
