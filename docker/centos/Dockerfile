
FROM centos-kungfu-devel:latest
COPY app/build/app/*.rpm /tmp
RUN yum install -y /tmp/Kungfu*.rpm
RUN ln -s /opt/Kungfu/resources/kfc/kfc /usr/local/bin/kfc
SHELL ["/bin/bash", "-c"]





