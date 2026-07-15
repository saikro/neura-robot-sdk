# syntax=docker/dockerfile:1.7

FROM python:3.12-slim

WORKDIR /app
COPY proto/  ./proto/
COPY client/ ./client/

WORKDIR /app/client
RUN pip install --no-cache-dir -e '.[dev]' \
 && python generate_proto.py

# ENTRYPOINT (not CMD) so `docker compose run --rm client info` appends
# `info` to the entrypoint instead of replacing it.
ENTRYPOINT ["python", "-m", "robot_client"]
