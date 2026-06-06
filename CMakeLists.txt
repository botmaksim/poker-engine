cmake_minimum_required(VERSION 3.10)
project(PokerEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_library(poker_engine_lib
    src/CoreEngine/GameState.cpp
    src/CoreEngine/Logger.cpp
    src/HandEvaluator/EvaluatorCore.cpp
    src/HandEvaluator/PerfectHashEvaluator.cpp
    src/HandEvaluator/PreflopHeuristics.cpp
    src/CoreEngine/Variants/TexasHoldem.cpp
    src/CoreEngine/Variants/Omaha.cpp
    src/CoreEngine/Variants/ShortDeck.cpp
    src/CoreEngine/Variants/SevenCardStud.cpp
    src/CoreEngine/Variants/Razz.cpp
    src/CoreEngine/Variants/FiveCardDraw.cpp
    src/CoreEngine/Variants/DeuceToSevenTripleDraw.cpp
    src/BayesianInference/BayesianTracker.cpp
    src/BotProfiles/CompositeBot.cpp
    src/BotProfiles/BaselineStrategies.cpp
    src/BotProfiles/BotFactory.cpp
    src/BayesianInference/Range.cpp
    src/BayesianInference/RangeParser.cpp
    src/BayesianInference/InferenceEngine.cpp
    src/CoreEngine/Session.cpp
    src/MonteCarloSimulator/Analyzer.cpp
    src/CoreEngine/TournamentArchitecture.cpp
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
