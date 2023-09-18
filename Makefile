# src=${wildcard ./*.cpp}
# obj=${parsubst %.cpp, %.o, $(src)}
# target=app
# $(target):$(obj)
# 	$(CXX) $(obj) -o $(target)

# %.o:%.cpp
# 	$(CXX) -c $< -o $@

# .PHONY:clean
# clean:
# 	rm $(obj) -r

src := $(wildcard ./*.cpp)
objs := $(patsubst %.cpp, %.o, $(src))
CFLAG := -g

target = app

$(target): $(objs)
	$(CXX) $(objs) -o $(target)

%.o: %.cpp
	$(CXX) -c $< -o $@

	
# 删除.o文件
.PHONY:clean	# 将clean变为伪目标，此时执行clean不会生成clean文件
clean:
	rm $(objs) $(target)
