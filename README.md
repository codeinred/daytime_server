# Daytime Server with Coroutines

This is a simple implementation of a daytime server using boost::asio and coroutines.

## Building:

```
git clone https://github.com/functionalperez/daytime_server
cd daytime_server
mkdir build && cd build

cmake .. -DCMAKE_CXX_COMPILER=clang++ && cmake --build .
```

## Running:
Server: 
```
sudo ./server # Daytime servers require sudo to access socket 13
```
Client:
```
./client `hostname -I`
```
