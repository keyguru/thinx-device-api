FROM redis

ARG REDIS_PASSWORD=redis

EXPOSE 6379

COPY ./redis.conf /etc/redis.conf
COPY ./docker-entrypoint.sh /docker-entrypoint.sh

RUN echo "export REDIS_PASSWORD=${REDIS_PASSWORD}" > /redis-password

ENTRYPOINT [ "/docker-entrypoint.sh" ]