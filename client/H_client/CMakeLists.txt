cmake_minimum_required(VERSION 3.16) # 指定项目所需的最低 CMake 版本为 3.16
project(client)
# 设置 C++ 标准为 C++17，并要求必须支持该标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(output_name client)  # 生成的可执行文件名称

# 项目基础目录配置
set(project_root ${CMAKE_CURRENT_SOURCE_DIR})  # 设置项目根目录
set(include_path ${project_root}/include)
set(proto_path ${project_root}/proto)  # proto目录
set(src_path ${project_root}/src)  # 设置源文件目录
set(bin_path ${project_root}/bin)  # 设置可执行文件目录
# 将这些目录添加到编译器的包含路径中，使得 #include 语句可以找到这些目录下的头文件
include_directories(${project_root})  # 确保项目根目录也在包含路径中
include_directories(${include_path})
include_directories(${proto_path})


# boost配置
find_package(Boost REQUIRED COMPONENTS system thread) # 查找并配置 Boost 库，要求必须找到 system 和 thread 组件
include_directories(${Boost_INCLUDE_DIRS}) # 将 Boost 的包含目录和库目录添加到项目中
link_directories(${Boost_LIBRARY_DIRS})


#jsoncpp配置
include_directories(/usr/include/jsoncpp/) # 手动指定 JSONCPP 的包含目录和库目录路径
link_directories(/usr/lib/x86_64-linux-gnu/)


# protobuf配置
include_directories(/usr/local/protobuf3_21_12/include/)
link_directories(/usr/local/protobuf3_21_12/lib/)


# 链接静态库
link_libraries(jsoncpp)


# 生成可执行文件
file(GLOB src_list ${src_path}/*.cpp)   # 使用通配符查找 src 目录下所有 .cpp 文件，并存储在 src_list 变量中
file(GLOB stc_list2 ${proto_path}/*.cc) # 查找 proto 目录下所有 .cc 文件（Protocol Buffers 生成的文件），存储在 stc_list2 变量中
set(src_list ${src_list} ${stc_list2})  # 将两个文件列表合并为一个列表
set(EXECUTABLE_OUTPUT_PATH ${bin_path}) # 设置可执行文件的输出路径
add_executable(${output_name} ${src_list})  # 创建可执行文件目标，使用之前定义的文件列表


# 链接库
target_link_libraries(${output_name} pthread)
target_link_libraries(${output_name} protobuf)