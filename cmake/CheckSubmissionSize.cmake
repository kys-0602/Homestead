foreach(required binary_path pak_path configuration maximum_save_size submission_limit)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing submission-size argument: ${required}")
    endif()
endforeach()

if(NOT configuration STREQUAL "Release")
    return()
endif()

if(NOT EXISTS "${binary_path}" OR NOT EXISTS "${pak_path}")
    message(FATAL_ERROR "Cannot measure missing submission output")
endif()

file(SIZE "${binary_path}" binary_size)
file(SIZE "${pak_path}" pak_size)
math(EXPR submission_size "${binary_size} + ${pak_size} + ${maximum_save_size}")
math(EXPR remaining_size "${submission_limit} - ${submission_size}")

if(submission_size GREATER submission_limit)
    message(FATAL_ERROR
        "Submission exceeds ${submission_limit} bytes: executable=${binary_size}, "
        "pak=${pak_size}, reserved_save=${maximum_save_size}, total=${submission_size}")
endif()

message(STATUS
    "Submission budget: executable=${binary_size}, pak=${pak_size}, "
    "reserved_save=${maximum_save_size}, total=${submission_size}, "
    "remaining=${remaining_size} bytes")
