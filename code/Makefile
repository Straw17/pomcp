.PHONY: all, clean
	
PROGNAME := main
	
CPPFLAGS := -DUSE_BOOST
CFLAGS := -O3
CXXFLAGS += -O3 -std=c++11
	
LDFLAGS += -g
LDFLAGS += -L/usr/include/boost/

BOOST_MODULES = \
	program_options	\
	filesystem	\
	graph	\
	iostreams	\
	system		\
	random		\
	regex		\
	serialization	\
	timer
	
BOOST_MODULES_TYPE := -mt
BOOST_MODULES_LIBS := $(addsuffix $(BOOT_MODULES_TYPE),$(BOOST_MODULES))
BOOST_LDFLAGS := $(addprefix -lboost_,$(BOOST_MODULES_LIBS))
CPPFLAGS += $(BOOST_CPPFLAGS)
LDFLAGS += $(BOOST_LDFLAGS)
	
SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)
OBJECTS = $(SOURCES:%.cpp=build/%.o)
	
all : $(PROGNAME)
	
$(PROGNAME) : $(OBJECTS) Makefile
	g++ -o $@ $(LDFLAGS) $(OBJECTS)
	
build/%.o : %.cpp $(HEADERS) Makefile
	g++ $(CXXFLAGS) $(CPPFLAGS) -c $(OUTPUT_OPTION) $<
	
clean :
	@echo "Clean."
	-rm -f build/*.o $(PROGNAME)
