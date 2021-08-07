# Daytime Server with Coroutines

This is a simple implementation of a daytime server using boost::asio and coroutines.

## Building:

```
git clone https://github.com/codeinred/daytime_server --recursive

chmod +x daytime_server/build.sh

daytime_server/build.sh
```

## Running:
Server:
```
# Daytime servers require sudo to access socket 13
sudo daytime_server/build/server
```
Client:
```
daytime_server/build/client $(hostname -i)
```
