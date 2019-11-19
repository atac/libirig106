
# Run tests in a GCC docker container #

FROM gcc:6.1

# Clean, build, and run tests
COPY . /libirig106
WORKDIR /libirig106
RUN make clean && make test
