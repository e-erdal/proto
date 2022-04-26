file(GLOB_RECURSE SOURCES ./*.cc)
add_library(ProtoBase STATIC ${SOURCES})
    target_precompile_headers(ProtoBase PUBLIC pch.hh)
    target_include_directories(ProtoBase PUBLIC .)
    target_link_libraries(ProtoBase PUBLIC 
        bx
        EASTL
        EAThread
        DirectXMath
        imgui
        lw
        fmt
        spdlog
        
        d3d11
        d3dcompiler
        dxgi
        dxguid
    )