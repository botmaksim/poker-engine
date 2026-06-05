cmake_minimum_required(VERSION 3.10)
project(PokerEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_library(poker_engine_lib
    src/Engine/GameState.cpp
    src/Engine/EvaluatorCore.cpp
    src/Games/TexasHoldem.cpp
    src/Games/Omaha.cpp
    src/Games/ShortDeck.cpp
    src/Games/SevenCardStud.cpp
    src/Games/Razz.cpp
    src/Games/FiveCardDraw.cpp
    src/Games/DeuceToSevenTripleDraw.cpp
    src/AI/BayesianTracker.cpp
    src/AI/CompositeBot.cpp
    src/AI/BaselineStrategies.cpp
    src/AI/BotFactory.cpp
    src/AI/Range.cpp
    src/AI/RangeParser.cpp
    src/AI/InferenceEngine.cpp
    src/Engine/Session.cpp
    src/Engine/Analyzer.cpp
    src/Engine/TournamentArchitecture.cpp
)

add_executable(poker_engine src/main.cpp)
target_link_libraries(poker_engine poker_engine_lib)

add_executable(test_simulation tools/test_simulation.cpp)
target_link_libraries(test_simulation poker_engine_lib)

add_executable(run_tournament tools/run_tournament.cpp)
target_link_libraries(run_tournament poker_engine_lib)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_IN 
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    )
    set(DOXYGEN_OUT 
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    )
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()
