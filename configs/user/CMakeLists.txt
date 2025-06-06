cmake_minimum_required(VERSION 3.20 FATAL_ERROR) #minimum version of CMAKE required
project(UserAction)

# Load ActRoot custom script
set(ACTROOT $ENV{ACTROOT})
include(${ACTROOT}/Scripts.cmake)

#And call function
add_userlibrary(NAME UserAction SOURCES UserAction.h UserAction.cxx LINK ActAlgorithm)
