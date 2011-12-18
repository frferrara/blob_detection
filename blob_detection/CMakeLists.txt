add_library( blob_detection SHARED blob_detection.cpp )
add_library(mt_circdet SHARED mt_circdet.cpp )

install( TARGETS 
         blob_detection
         mt_circdet
         DESTINATION
         lib )
install( FILES 
         blob_detection.hpp
         mt_circdet.hpp
         DESTINATION
         include )