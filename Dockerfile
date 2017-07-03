
# To use same image as bitbucket pipelines
FROM gcc:6.1

# To use ubuntu image
# FROM ubuntu:latest
# RUN apt-get update && apt-get install -y build-essential gcc make

# To use centos image
# FROM centos:latest
# RUN yum install -y build-essential gcc make

# Clean, build, and run tests
COPY . /libirig106
WORKDIR /libirig106
RUN make clean && make test
