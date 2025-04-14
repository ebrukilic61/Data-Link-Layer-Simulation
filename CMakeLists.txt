cmake_minimum_required(VERSION 3.10)
project(viba_project)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Kaynak dosyalarını ekleyin
set(SOURCES
    main.cpp
    sender.cpp
    receiver.cpp
    protocol_utils.cpp
)

# Header dosyalarını ekleyin
set(HEADERS
    sender.h
    receiver.h
    protocol_utils.h
)

# Çalıştırılabilir dosya oluştur
add_executable(sender ${SOURCES} ${HEADERS})

# Arayüz kütüphaneleri
# find_package(Qt5 COMPONENTS Widgets REQUIRED)
# target_link_libraries(sender Qt5::Widgets)