add_subdirectory(01-qtsql-database-beginner)
set_target_properties(01-qtsql-database-beginner PROPERTIES OUTPUT_NAME "05-01-qtsql-database")

add_subdirectory(02-qtsql-tablemodel-beginner)
set_target_properties(02-qtsql-tablemodel-beginner PROPERTIES OUTPUT_NAME "05-02-qtsql-tablemodel")

add_subdirectory(03-qtcharts-basic-beginner)
set_target_properties(03-qtcharts-basic-beginner PROPERTIES OUTPUT_NAME "05-03-qtcharts-basic")

add_subdirectory(04-qtmultimedia-player-beginner)
set_target_properties(04-qtmultimedia-player-beginner PROPERTIES OUTPUT_NAME "05-04-qtmultimedia-player")

add_subdirectory(05-qtmultimedia-camera-beginner)
set_target_properties(05-qtmultimedia-camera-beginner PROPERTIES OUTPUT_NAME "05-05-qtmultimedia-camera")

add_subdirectory(06-qtsvg-beginner)
set_target_properties(06-qtsvg-beginner PROPERTIES OUTPUT_NAME "05-06-qtsvg")

add_subdirectory(07-qtprintsupport-beginner)
set_target_properties(07-qtprintsupport-beginner PROPERTIES OUTPUT_NAME "05-07-qtprintsupport")

add_subdirectory(08-qtserialbus-modbus-beginner)
set_target_properties(08-qtserialbus-modbus-beginner PROPERTIES OUTPUT_NAME "05-08-qtserialbus-modbus")

add_subdirectory(09-qtmqtt-beginner)
set_target_properties(09-qtmqtt-beginner PROPERTIES OUTPUT_NAME "05-09-qtmqtt")

add_subdirectory(10-qtbluetooth-beginner)
set_target_properties(qtbluetooth_beginner PROPERTIES OUTPUT_NAME "05-10-qtbluetooth")

add_subdirectory(11-qtnfc-beginner)
set_target_properties(qtnfc_beginner PROPERTIES OUTPUT_NAME "05-11-qtnfc")

add_subdirectory(12-qtstatemachine-beginner)
set_target_properties(qtstatemachine_beginner PROPERTIES OUTPUT_NAME "05-12-qtstatemachine")

add_subdirectory(13-qtscxml-beginner)
set_target_properties(qtscxml_beginner PROPERTIES OUTPUT_NAME "05-13-qtscxml")

add_subdirectory(14-qt3d-basic-beginner)
set_target_properties(qt3d_basic_beginner PROPERTIES OUTPUT_NAME "05-14-qt3d-basic")

add_subdirectory(15-qtquick3d-beginner)
set_target_properties(qtquick3d_beginner PROPERTIES OUTPUT_NAME "05-15-qtquick3d")

add_subdirectory(16-qtquick3d-physics-beginner)
if(TARGET qtquick3d_physics_beginner)
    set_target_properties(qtquick3d_physics_beginner PROPERTIES OUTPUT_NAME "05-16-qtquick3d-physics")
endif()

add_subdirectory(17-qtpdf-beginner)
set_target_properties(qtpdf_beginner PROPERTIES OUTPUT_NAME "05-17-qtpdf")

add_subdirectory(18-qthttpserver-beginner)
set_target_properties(qthttpserver_beginner PROPERTIES OUTPUT_NAME "05-18-qthttpserver")

add_subdirectory(19-qtwebsockets-server-beginner)
set_target_properties(qtwebsockets_server_beginner PROPERTIES OUTPUT_NAME "05-19-qtwebsockets-server")

add_subdirectory(20-qtwebchannel-beginner)
set_target_properties(qtwebchannel_beginner PROPERTIES OUTPUT_NAME "05-20-qtwebchannel")

add_subdirectory(21-qtwebengine-beginner)
set_target_properties(qtwebengine_beginner PROPERTIES OUTPUT_NAME "05-21-qtwebengine")

add_subdirectory(22-qtremoteobjects-beginner)
set_target_properties(qtremoteobjects_beginner PROPERTIES OUTPUT_NAME "05-22-qtremoteobjects")

add_subdirectory(23-qtspatial-audio-beginner)
set_target_properties(qtspatial_audio_beginner PROPERTIES OUTPUT_NAME "05-23-qtspatial-audio")

add_subdirectory(24-qttexttospeech-beginner)
set_target_properties(qttexttospeech_beginner PROPERTIES OUTPUT_NAME "05-24-qttexttospeech")

add_subdirectory(25-qt5compat-migration-beginner)
set_target_properties(qt5compat_migration_beginner PROPERTIES OUTPUT_NAME "05-25-qt5compat-migration")
