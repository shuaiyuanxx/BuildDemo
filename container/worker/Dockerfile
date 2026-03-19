FROM alpine:latest
RUN apk add --no-cache g++
COPY worker.cpp /app/worker.cpp
RUN g++ -o /app/worker /app/worker.cpp
EXPOSE 9001
CMD ["/app/worker"]
