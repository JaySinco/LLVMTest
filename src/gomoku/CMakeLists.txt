get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(MAIN ${CURRENT_DIR_NAME})

add_executable(${MAIN}
    network.cpp
    train.cpp
    game.cpp
    mcts.cpp
    main.cpp
)

target_link_libraries(${MAIN} utils
    argparse::argparse
    Torch::torch
    $<IF:$<BOOL:${UNIX}>,,-INCLUDE:?warp_size@cuda@at@@YAHXZ>
    $<IF:$<BOOL:${UNIX}>,,-INCLUDE:?searchsorted_cuda@native@at@@YA?AVTensor@2@AEBV32@0_N1@Z>
)
