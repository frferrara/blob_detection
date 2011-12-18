add_library( blob_detection SHARED blob_detection.cpp )

install( TARGETS blob_detection DESTINATION lib )
install( FILES blob_detection.hpp DESTINATION include )