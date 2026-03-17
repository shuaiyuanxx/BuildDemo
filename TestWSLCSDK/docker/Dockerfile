FROM alpine:latest
RUN apk add --no-cache g++
COPY server.cpp /app/server.cpp
RUN g++ -o /app/server /app/server.cpp
EXPOSE 9000
CMD ["/app/server"]
