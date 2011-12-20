add_library( blob_detection SHARED blob_detection.cpp )
add_library(mt_circdet SHARED mt_circdet.cpp )

target_link_libraries( blob_detection mt_circdet ${OpenCV_LIBS} -lcvblob -lrand_gen -lcircdet -lgsl -lgslcblas -ltbb )

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