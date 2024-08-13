# list

1. autoinit.h :
    - Automatically initialize all
    - Specifically, it calls exec、network、setlog... look the [dev.md](dev.md)
2. exec.h :
    - some practical features and encapsulation
        - singleton obj manage
            - ThreadPool

            ```cpp
                auto it = exec::getThreadObj().enqueue(func,args);
                auto res = it.get();
            ```

            - CSimpleIniA （config file）

            ```cpp
                auto debug = exec::getConfigObj().GetValue("Section","key","default");
            ```

        - utility func
            - look the [exec.h](https://github.com/moehoshio/exec.h)

3. network.h : networkBase :  
    - network encapsulation template class
        - prototype :

        ```cpp
        template <typename T, typename F, typename F2>
        class network : public networkBase {};
        ```

        - T : return type , default : std::string
        - F : write callback type , default :

        ```cpp
        size_t(char *, size_t, size_t, void *)
        ```

        - F2 : header callback type , default :

        ```cpp
        size_t(char *, size_t, size_t, void *)
        ```

        - func :
            - void Do : Applicable not return value

            ```cpp
            network net;
            decltype(net)::Args args{
                "https://api.example.com/file.zip",
                "/to/file/download.zip"
            };
            net.Do(networkBase::Opt::downloadFile,args);
            ```

            - T get : Applicable with return value

            ```cpp
            network net;
            decltype(net)::Args args{
                "https://api.example.com/file.txt",
            };
            auto res = net.get(networkBase::Opt::getContent,args);
            ```
