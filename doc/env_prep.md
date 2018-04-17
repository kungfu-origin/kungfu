Instruments for preparing independent kungfu environment 
==========

* yum install rpm-build

* yum install numactl

* Compiling Kungfu

    * install log4cplus [https://github.com/log4cplus/log4cplus]
        * take a look at issue: https://github.com/log4cplus/log4cplus/issues/169
        
    * install pid-2.1.1
        * install dependency nose-1.3.7 first (yum install python-nose)

    * install rfoo [https://github.com/aaiyer/rfoo]
        * install dependency Cython first

    * install supervisor
        * install dependency meld3 (yum install python-meld3)

    * install boost-1.62
        * make sure folder /opt/kungfu/toolchain/boost-1.62.0 exists
        * ./bootstrap.sh --prefix=/opt/kungfu/toolchain/boost-1.62.0
        * ./b2 install --prefix=/opt/kungfu/toolchain/boost-1.62.0

    * install cmake-[3.8.*] 

    * library search path:
        ```
        # # add LD_LIBRARY_PATH for all users
        # The cleanest way to add a library to the search path is by adding a file to /etc/ld.so.conf.d/
        
        echo "/usr/local/lib" >> /etc/ld.so.conf.d/log4cplus-2.0.so.conf  # suppose log4cplus is installed in /usr/local/lib
        
        # load the new configuration, run the following command as a root
        
        ldconfig -v
        ```
        
    * compiling kungfu and install kungfu
    
**Contributed by Aimin Huang.**