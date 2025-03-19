CXX = g++
CFLAGS = -g -std=c++0x -O2 -Wall -Wno-unused -DLINUX -fPIC
CFLAGS += -DALIGNATTRIB= -DCDECL= -DHANDLE_WIN32_PRAGMA -DDLL_DECL= -D_REENTRANT= -DHUGEPTR= -D_LINUX_
CFLAGS += -Wno-format-security
CFLAGS += -I./ -I../Hsc3Api/include -I/usr/local/include/boost

LDFLAGS = -L../Hsc3Api/lib -L./
LDLIBS = -lpthread -lCommApi -lHsc3Api -lLogApi -lboost_system -lboost_thread -lboost_chrono

target = Hsc3Demo
src = $(wildcard *.cpp)
objs = $(patsubst %.cpp,%.o,$(src))

$(target): $(objs)
	$(CXX) $(objs) $(LDFLAGS) $(LDLIBS) -o $(target)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(target) $(objs) *.d