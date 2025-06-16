异步服务器（应答式）
在B-server基础上的修改
1. 在Server中改用智能指针的方式管理Session类
2. 当接收到新连接时使用boost的uuid库生成一个session_id，与session绑定，在Server中使用map存储session_id和Session智能指针的键值对，方便后期做一些重连踢人等操作
3. 将Session的智能指针传递给lambda回调函数，lambda回调函数内部持有Session的智能指针，这样就保证了Session和该函数对象的生命周期一致
    - 不能直接使用this构造出来的智能指针，因为这样生成出的智能指针与map中的智能指针指向相同，但是引用计数独立，会出现多重释放问题
    - 需要将Session继承于std::enable_shared_from_this<CSession>模板类，这样可以通过shared_from_this()得到自己的智能指针，该智能指针与外部指向该对象的智能指针引用计数同步
4. 当Session的HandleRead()和HandleWrite()出现错误时，调用Server的ClearSession()函数将其在map中删除，可以去除其引用计数，使用智能指针的引用计数管理Session的生命周期来避免出现double free问题
    - 因此Session需要存储Server的指针方便调用ClearSession()函数
5. 将Session的创建时间延迟到了服务器接受到新连接后，因为后期session_id可能包含创建时间有关的信息，在Session初始化的时候就将Session_id绑定。不然要通过SetId等操作进行后期绑定，而开放这样的接口是不安全的
6. Session中的socket改用使用智能指针管理

> 下面代码输出的use_count为2和3；
```cpp
_socket->async_read_some(
    boost::asio::buffer(_recv_pack_data, _PACK_MAX_SIZE),
        // 使用shared_from_this()获取当前对象的shared_ptr
    [this, self_ptr = shared_from_this()](const boost::system::error_code& error, size_t recv_pack_size) {
        // 2 {map中的智能指针副本, lambda函数[]中的self_ptr}
        std::cout << "1 self_ptr use_count: " << _server->_sessions[_session_id].use_count() << std::endl;
        HandleRead(error, recv_pack_size, self_ptr);
        // 3 {map中的智能指针副本, lambda函数[]中的self_ptr, HandleRead()中构建的异步回调函数中的lambda函数[]中的self_ptr}
        std::cout << "2 self_ptr use_count: " << _server->_sessions[_session_id].use_count() << std::endl; 
    }
);
```